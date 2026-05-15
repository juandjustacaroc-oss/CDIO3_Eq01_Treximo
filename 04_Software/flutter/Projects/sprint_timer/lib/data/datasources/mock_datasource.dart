import 'dart:async';
import 'dart:math';
import 'package:uuid/uuid.dart';
import '../../domain/entities/entities.dart';
import '../../domain/repositories/repositories.dart';

const _uuid = Uuid();
final _rng = Random();

// ─────────────────────────────────────────
// Mock athletes (3 atletas de ejemplo)
// ─────────────────────────────────────────
final mockAthletes = [
  const Athlete(athleteId: 1, name: 'Carlos Álvarez', notes: 'Velocista 100m'),
  const Athlete(athleteId: 3, name: 'María López', notes: 'Velocista 50/100m'),
  const Athlete(athleteId: 7, name: 'Juan Pérez', notes: 'En desarrollo 50m'),
];

// ─────────────────────────────────────────
// Mock session generator
// ─────────────────────────────────────────
List<Session> generateMockSessions() {
  final sessions = <Session>[];

  // Carlos — 100m — 6 sesiones desde hace 30 días
  final carloBests100 = [11240, 11180, 11320, 11090, 10980, 10870]; // ms
  for (int i = 0; i < carloBests100.length; i++) {
    final t = carloBests100[i];
    final speedMps = 100000 / t;
    final hr = 162.0 + _rng.nextDouble() * 18;
    sessions.add(Session(
      sessionId: _uuid.v4(),
      athleteId: 1,
      datetimeStart: DateTime.now().subtract(Duration(days: 30 - i * 5, hours: 9)),
      distanceMeters: 100,
      timeMs: t,
      avgSpeedMps: speedMps,
      avgSpeedKmh: speedMps * 3.6,
      bpmFinish: (hr + 12 + _rng.nextInt(8)).round(),
      bpmRecovery: (hr - 15 - _rng.nextInt(10)).round(),
      distanceCalibrated: true,
      ecv: 6.07 / (hr + 12),
      isPR: i == carloBests100.length - 1,
      deviceRaceId: 'R${100 + i}',
    ));
  }

  // Carlos — 50m — 4 sesiones
  final carlos50 = [5820, 5740, 5690, 5650];
  for (int i = 0; i < carlos50.length; i++) {
    final t = carlos50[i];
    final speedMps = 50000 / t;
    final hr = 158.0 + _rng.nextDouble() * 15;
    sessions.add(Session(
      sessionId: _uuid.v4(),
      athleteId: 1,
      datetimeStart: DateTime.now().subtract(Duration(days: 28 - i * 7, hours: 10)),
      distanceMeters: 50,
      timeMs: t,
      avgSpeedMps: speedMps,
      avgSpeedKmh: speedMps * 3.6,
      bpmFinish: (hr + 10).round(),
      bpmRecovery: (hr - 20).round(),
      distanceCalibrated: true,
      ecv: 7.15 / (hr + 2),
      isPR: i == carlos50.length - 1,
    ));
  }

  // María — 50m — 5 sesiones
  final maria50 = [6820, 6750, 6710, 6680, 6640];
  for (int i = 0; i < maria50.length; i++) {
    final t = maria50[i];
    final speedMps = 50000 / t;
    final hr = 170.0 + _rng.nextDouble() * 14;
    sessions.add(Session(
      sessionId: _uuid.v4(),
      athleteId: 3,
      datetimeStart: DateTime.now().subtract(Duration(days: 25 - i * 5, hours: 16)),
      distanceMeters: 50,
      timeMs: t,
      avgSpeedMps: speedMps,
      avgSpeedKmh: speedMps * 3.6,
      bpmFinish: (hr + 15).round(),
      bpmRecovery: (hr - 12).round(),
      distanceCalibrated: true,
      ecv: 6.80 / (hr + 5),
      isPR: i == maria50.length - 1,
    ));
  }

  // María — 100m — 3 sesiones
  final maria100 = [12100, 11980, 11820];
  for (int i = 0; i < maria100.length; i++) {
    final t = maria100[i];
    final speedMps = 100000 / t;
    final hr = 172.0 + _rng.nextDouble() * 12;
    sessions.add(Session(
      sessionId: _uuid.v4(),
      athleteId: 3,
      datetimeStart: DateTime.now().subtract(Duration(days: 20 - i * 6, hours: 17)),
      distanceMeters: 100,
      timeMs: t,
      avgSpeedMps: speedMps,
      avgSpeedKmh: speedMps * 3.6,
      bpmFinish: (hr + 15).round(),
      bpmRecovery: (hr - 10).round(),
      distanceCalibrated: true,
      ecv: 5.80 / (hr + 5),
      isPR: i == maria100.length - 1,
    ));
  }

  // Juan — 50m — 4 sesiones
  final juan50 = [7200, 7050, 6990, 6920];
  for (int i = 0; i < juan50.length; i++) {
    final t = juan50[i];
    final speedMps = 50000 / t;
    final hr = 175.0 + _rng.nextDouble() * 15;
    sessions.add(Session(
      sessionId: _uuid.v4(),
      athleteId: 7,
      datetimeStart: DateTime.now().subtract(Duration(days: 21 - i * 5, hours: 8)),
      distanceMeters: 50,
      timeMs: t,
      avgSpeedMps: speedMps,
      avgSpeedKmh: speedMps * 3.6,
      bpmFinish: (hr + 16).round(),
      bpmRecovery: (hr - 10).round(),
      distanceCalibrated: true,
      ecv: 5.50 / (hr + 5),
      isPR: i == juan50.length - 1,
    ));
  }

  return sessions..sort((a, b) => b.datetimeStart.compareTo(a.datetimeStart));
}

/// Genera muestras HR ficticias para una sesión
List<HrSample> generateHrSamples(Session session) {
  final samples = <HrSample>[];
  final runDuration = session.timeMs;
  const recoveryDuration = 120000; // 2 min recovery

  // RUN phase (cada 500ms)
  for (int ms = 0; ms < runDuration; ms += 500) {
    final progress = ms / runDuration;
    final bpm = (session.bpmFinish * 0.7 + session.bpmFinish * 0.3 * progress +
        (_rng.nextDouble() - 0.5) * 6).round();
    samples.add(HrSample(sessionId: session.sessionId, offsetMs: ms, bpm: bpm, phase: HrPhase.run));
  }

  // RECOVERY phase (cada 1000ms)
  for (int ms = 0; ms < recoveryDuration; ms += 1000) {
    final progress = ms / recoveryDuration;
    final bpm = (session.bpmFinish * (1 - progress * 0.5) +
        _rng.nextDouble() * 5).round();
    samples.add(HrSample(sessionId: session.sessionId, offsetMs: runDuration + ms, bpm: bpm, phase: HrPhase.recovery));
  }

  return samples;
}

// ─────────────────────────────────────────
// Mock DataSource (implementa la interfaz)
// ─────────────────────────────────────────
class MockSportDataSource implements SportDataSource {
  final _controller = StreamController<DevicePacket>.broadcast();
  bool _connected = false;
  Timer? _hrTimer;

  @override
  Stream<DevicePacket> get packetStream => _controller.stream;

  @override
  bool get isConnected => _connected;

  @override
  Future<void> connect() async {
    await Future.delayed(const Duration(milliseconds: 400));
    _connected = true;
    // Simula HR updates cada 500ms
    _hrTimer = Timer.periodic(const Duration(milliseconds: 500), (_) {
      if (_connected) {
        _controller.add(DevicePacket(
          type: PacketType.hrUpdate,
          payload: {'bpm': 65 + _rng.nextInt(30)},
          receivedAt: DateTime.now(),
        ));
      }
    });
  }

  @override
  Future<void> disconnect() async {
    _connected = false;
    _hrTimer?.cancel();
  }

  /// Simula un START manual (para pruebas en UI)
  void simulateStart() {
    _controller.add(DevicePacket(
      type: PacketType.start,
      payload: {},
      receivedAt: DateTime.now(),
    ));
  }

  /// Simula un FINISH tras [delayMs] ms
  void simulateFinish({int delayMs = 10500}) {
    Future.delayed(Duration(milliseconds: delayMs), () {
      _controller.add(DevicePacket(
        type: PacketType.finish,
        payload: {'timeMs': delayMs, 'hrPeak': 180 + _rng.nextInt(15)},
        receivedAt: DateTime.now(),
      ));
    });
  }

  void dispose() {
    _hrTimer?.cancel();
    _controller.close();
  }
}
