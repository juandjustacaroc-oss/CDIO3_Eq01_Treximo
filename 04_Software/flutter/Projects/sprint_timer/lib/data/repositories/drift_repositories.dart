import 'package:drift/drift.dart';
import '../../domain/entities/entities.dart' as e;
import '../../domain/repositories/repositories.dart';
import '../database/app_database.dart';

class DriftAthleteRepository implements AthleteRepository {
  final AppDatabase _db;
  DriftAthleteRepository(this._db);

  @override
  Future<void> delete(int athleteId) => _db.deleteAthlete(athleteId);

  @override
  Future<List<e.Athlete>> getAll() async {
    final list = await _db.getAllAthletes();
    return list.map((a) => e.Athlete(athleteId: a.athleteId, name: a.name, notes: a.notes)).toList();
  }

  @override
  Future<e.Athlete?> getById(int id) async {
    final a = await _db.getAthleteById(id);
    if (a == null) return null;
    return e.Athlete(athleteId: a.athleteId, name: a.name, notes: a.notes);
  }

  @override
  Future<void> save(e.Athlete athlete) {
    return _db.upsertAthlete(AthletesCompanion(
      athleteId: Value(athlete.athleteId),
      name: Value(athlete.name),
      notes: Value(athlete.notes),
    ));
  }
}

class DriftSessionRepository implements SessionRepository {
  final AppDatabase _db;
  DriftSessionRepository(this._db);

  e.Session _mapSession(Session s) {
    return e.Session(
      sessionId: s.sessionId,
      athleteId: s.athleteId,
      datetimeStart: s.datetimeStart,
      distanceMeters: s.distanceMeters,
      timeMs: s.timeMs,
      avgSpeedMps: s.avgSpeedMps,
      avgSpeedKmh: s.avgSpeedKmh,
      bpmFinish: s.bpmFinish,
      bpmRecovery: s.bpmRecovery,
      distanceCalibrated: s.distanceCalibrated,
      ecv: s.ecv,
      notes: s.notes,
      deviceRaceId: s.deviceRaceId,
      isPR: s.isPR,
    );
  }

  @override
  Future<void> delete(String sessionId) => _db.deleteSession(sessionId);

  @override
  Future<List<e.Session>> getAll({
    int? athleteId,
    double? distanceMeters,
    DateTime? from,
    DateTime? to,
  }) async {
    final list = await _db.querySessions(
      athleteId: athleteId,
      distance: distanceMeters,
      from: from,
      to: to,
    );
    return list.map(_mapSession).toList();
  }

  @override
  Future<e.Session?> getById(String sessionId) async {
    final s = await _db.getSessionById(sessionId);
    if (s == null) return null;
    return _mapSession(s);
  }

  @override
  Future<e.Session?> getPersonalRecord(int athleteId, double distanceMeters) async {
    final s = await _db.getPersonalRecord(athleteId, distanceMeters);
    if (s == null) return null;
    return _mapSession(s);
  }

  @override
  Future<List<e.Session>> getRecentSessions(int athleteId, double distanceMeters, {int limit = 5}) async {
    final list = await _db.querySessions(athleteId: athleteId, distance: distanceMeters);
    return list.take(limit).map(_mapSession).toList();
  }

  @override
  Future<void> save(e.Session session) {
    return _db.upsertSession(SessionsCompanion(
      sessionId: Value(session.sessionId),
      athleteId: Value(session.athleteId),
      datetimeStart: Value(session.datetimeStart),
      distanceMeters: Value(session.distanceMeters),
      timeMs: Value(session.timeMs),
      avgSpeedMps: Value(session.avgSpeedMps),
      avgSpeedKmh: Value(session.avgSpeedKmh),
      bpmFinish: Value(session.bpmFinish),
      bpmRecovery: Value(session.bpmRecovery),
      distanceCalibrated: Value(session.distanceCalibrated),
      ecv: Value(session.ecv),
      notes: Value(session.notes),
      deviceRaceId: Value(session.deviceRaceId),
      isPR: Value(session.isPR),
    ));
  }
}
