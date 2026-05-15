import 'dart:async';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:equatable/equatable.dart';
import 'package:uuid/uuid.dart';
import '../../domain/entities/entities.dart' as e;
import '../../domain/repositories/repositories.dart';
import '../../data/datasources/mock_datasource.dart';

const _uuid = Uuid();

// ─── Events ──────────────────────────────
abstract class LiveEvent extends Equatable {
  const LiveEvent();
  @override List<Object?> get props => [];
}

class LiveSelectAthlete extends LiveEvent {
  final e.Athlete athlete;
  const LiveSelectAthlete(this.athlete);
  @override List<Object?> get props => [athlete];
}

class LiveSelectDistance extends LiveEvent {
  final int meters;
  const LiveSelectDistance(this.meters);
  @override List<Object?> get props => [meters];
}

class LiveSimulateRace extends LiveEvent {} // mock: inicia sin hardware

class LiveReset extends LiveEvent {}

class _LiveTick extends LiveEvent {
  final int elapsedMs;
  const _LiveTick(this.elapsedMs);
  @override List<Object?> get props => [elapsedMs];
}

class _LivePacketReceived extends LiveEvent {
  final DevicePacket packet;
  const _LivePacketReceived(this.packet);
  @override List<Object?> get props => [packet];
}

// ─── States ──────────────────────────────
class LiveState extends Equatable {
  final e.Athlete? selectedAthlete;
  final int selectedDistance;
  final e.SessionState sessionState;
  final int elapsedMs;
  final int currentHr;
  final e.Session? finishedSession;

  const LiveState({
    this.selectedAthlete,
    this.selectedDistance = 100,
    this.sessionState = e.SessionState.idle,
    this.elapsedMs = 0,
    this.currentHr = 0,
    this.finishedSession,
  });

  LiveState copyWith({
    e.Athlete? selectedAthlete,
    int? selectedDistance,
    e.SessionState? sessionState,
    int? elapsedMs,
    int? currentHr,
    e.Session? finishedSession,
  }) => LiveState(
    selectedAthlete: selectedAthlete ?? this.selectedAthlete,
    selectedDistance: selectedDistance ?? this.selectedDistance,
    sessionState: sessionState ?? this.sessionState,
    elapsedMs: elapsedMs ?? this.elapsedMs,
    currentHr: currentHr ?? this.currentHr,
    finishedSession: finishedSession ?? this.finishedSession,
  );

  @override
  List<Object?> get props => [selectedAthlete, selectedDistance, sessionState, elapsedMs, currentHr, finishedSession];
}

// ─── BLoC ────────────────────────────────
class LiveSessionBloc extends Bloc<LiveEvent, LiveState> {
  final SessionRepository _sessionRepo;
  final SportDataSource _dataSource;

  Timer? _ticker;
  int _startEpoch = 0;
  StreamSubscription<DevicePacket>? _packetSub;

  // Accumulate HR samples
  final List<int> _hrRunSamples = [];
  final List<int> _hrRecoverySamples = [];

  LiveSessionBloc(this._sessionRepo, this._dataSource)
      : super(const LiveState()) {
    on<LiveSelectAthlete>((ev, emit) => emit(state.copyWith(selectedAthlete: ev.athlete)));
    on<LiveSelectDistance>((ev, emit) => emit(state.copyWith(selectedDistance: ev.meters)));
    on<LiveSimulateRace>(_onSimulate);
    on<LiveReset>(_onReset);
    on<_LiveTick>((ev, emit) => emit(state.copyWith(elapsedMs: ev.elapsedMs)));
    on<_LivePacketReceived>(_onPacket);

    _initDataSource();
  }

  void _initDataSource() async {
    await _dataSource.connect();
    _packetSub = _dataSource.packetStream.listen((p) => add(_LivePacketReceived(p)));
  }

  Future<void> _onSimulate(LiveSimulateRace ev, Emitter<LiveState> emit) async {
    if (state.selectedAthlete == null) return;
    final mock = _dataSource as MockSportDataSource;
    mock.simulateStart();
    mock.simulateFinish(delayMs: 10000 + (DateTime.now().millisecond % 3000));
  }

  Future<void> _onPacket(_LivePacketReceived ev, Emitter<LiveState> emit) async {
    final packet = ev.packet;

    if (packet.type == PacketType.hrUpdate) {
      final bpm = packet.payload['bpm'] as int;
      if (state.sessionState == e.SessionState.inProgress) _hrRunSamples.add(bpm);
      if (state.sessionState == e.SessionState.recovery) _hrRecoverySamples.add(bpm);
      emit(state.copyWith(currentHr: bpm));
      return;
    }

    if (packet.type == PacketType.start && state.sessionState != e.SessionState.inProgress) {
      _hrRunSamples.clear();
      _hrRecoverySamples.clear();
      _startEpoch = DateTime.now().millisecondsSinceEpoch;
      _ticker?.cancel();
      _ticker = Timer.periodic(const Duration(milliseconds: 100), (_) {
        add(_LiveTick(DateTime.now().millisecondsSinceEpoch - _startEpoch));
      });
      emit(state.copyWith(sessionState: e.SessionState.inProgress, elapsedMs: 0));
    }

    if (packet.type == PacketType.finish && state.sessionState == e.SessionState.inProgress) {
      _ticker?.cancel();
      final elapsed = packet.payload['timeMs'] as int? ?? state.elapsedMs;

      // Build session
      final dist = state.selectedDistance.toDouble();
      final speedMps = (dist * 1000) / elapsed;
      final hrRun = _hrRunSamples.isEmpty ? 165.0 : _hrRunSamples.reduce((a, b) => a + b) / _hrRunSamples.length;
      final hrMax = _hrRunSamples.isEmpty ? 185.0 : _hrRunSamples.reduce((a, b) => a > b ? a : b).toDouble();
      final hrRecovery = _hrRecoverySamples.isEmpty ? 130.0 : _hrRecoverySamples.reduce((a, b) => a + b) / _hrRecoverySamples.length;
      final ecv = speedMps / hrMax;

      final session = e.Session(
        sessionId: _uuid.v4(),
        athleteId: state.selectedAthlete!.athleteId,
        datetimeStart: DateTime.fromMillisecondsSinceEpoch(_startEpoch),
        distanceMeters: dist,
        timeMs: elapsed,
        avgSpeedMps: speedMps,
        avgSpeedKmh: speedMps * 3.6,
        bpmFinish: hrMax.round(),
        bpmRecovery: hrRecovery.round(),
        distanceCalibrated: true,
        ecv: ecv,
      );

      await _sessionRepo.save(session);

      emit(state.copyWith(
        sessionState: e.SessionState.recovery,
        elapsedMs: elapsed,
        finishedSession: session,
      ));
    }
  }

  Future<void> _onReset(LiveReset ev, Emitter<LiveState> emit) async {
    _ticker?.cancel();
    _hrRunSamples.clear();
    _hrRecoverySamples.clear();
    emit(const LiveState());
  }

  @override
  Future<void> close() {
    _ticker?.cancel();
    _packetSub?.cancel();
    return super.close();
  }
}
