import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:equatable/equatable.dart';
import '../../domain/entities/entities.dart';
import '../../domain/repositories/repositories.dart';

// ─── Events ──────────────────────────────
abstract class SessionEvent extends Equatable {
  const SessionEvent();
  @override List<Object?> get props => [];
}

class LoadSessions extends SessionEvent {
  final int? athleteId;
  final double? distanceMeters;
  final DateTime? from;
  final DateTime? to;
  const LoadSessions({this.athleteId, this.distanceMeters, this.from, this.to});
  @override List<Object?> get props => [athleteId, distanceMeters, from, to];
}

class SaveSession extends SessionEvent {
  final Session session;
  const SaveSession(this.session);
  @override List<Object?> get props => [session];
}

class DeleteSession extends SessionEvent {
  final String sessionId;
  const DeleteSession(this.sessionId);
  @override List<Object?> get props => [sessionId];
}

// ─── States ──────────────────────────────
// Named SessionBlocState to avoid clash with entities.dart SessionState enum
abstract class SessionBlocState extends Equatable {
  const SessionBlocState();
  @override List<Object?> get props => [];
}

class SessionsInitial extends SessionBlocState {}

class SessionsLoading extends SessionBlocState {}

class SessionsLoaded extends SessionBlocState {
  final List<Session> sessions;
  final int? filterAthleteId;
  final double? filterDistance;
  final DateTime? filterFrom;
  final DateTime? filterTo;

  const SessionsLoaded(
    this.sessions, {
    this.filterAthleteId,
    this.filterDistance,
    this.filterFrom,
    this.filterTo,
  });

  @override
  List<Object?> get props => [sessions, filterAthleteId, filterDistance, filterFrom, filterTo];
}

class SessionsError extends SessionBlocState {
  final String message;
  const SessionsError(this.message);
  @override List<Object?> get props => [message];
}

// ─── BLoC ────────────────────────────────
class SessionBloc extends Bloc<SessionEvent, SessionBlocState> {
  final SessionRepository _repo;

  SessionBloc(this._repo) : super(SessionsInitial()) {
    on<LoadSessions>(_onLoad);
    on<SaveSession>(_onSave);
    on<DeleteSession>(_onDelete);
  }

  Future<void> _onLoad(LoadSessions e, Emitter<SessionBlocState> emit) async {
    emit(SessionsLoading());
    try {
      final sessions = await _repo.getAll(
        athleteId: e.athleteId,
        distanceMeters: e.distanceMeters,
        from: e.from,
        to: e.to,
      );
      emit(SessionsLoaded(
        sessions,
        filterAthleteId: e.athleteId,
        filterDistance: e.distanceMeters,
        filterFrom: e.from,
        filterTo: e.to,
      ));
    } catch (err) {
      emit(SessionsError(err.toString()));
    }
  }

  Future<void> _onSave(SaveSession e, Emitter<SessionBlocState> emit) async {
    await _repo.save(e.session);
    add(const LoadSessions()); // reload
  }

  Future<void> _onDelete(DeleteSession e, Emitter<SessionBlocState> emit) async {
    await _repo.delete(e.sessionId);
    add(const LoadSessions());
  }
}
