import '../../domain/entities/entities.dart';
import '../../domain/repositories/repositories.dart';
import '../datasources/mock_datasource.dart';

/// Repository en memoria — usa datos mock.
/// Reemplaza con DriftSessionRepository cuando tengas BD real.
class InMemorySessionRepository implements SessionRepository {
  final List<Session> _sessions;

  InMemorySessionRepository(this._sessions);

  @override
  Future<List<Session>> getAll({
    int? athleteId,
    int? distanceMeters,
    DateTime? from,
    DateTime? to,
  }) async {
    return _sessions.where((s) {
      if (athleteId != null && s.athleteId != athleteId) return false;
      if (distanceMeters != null && s.distanceMeters != distanceMeters) return false;
      if (from != null && s.datetimeStart.isBefore(from)) return false;
      if (to != null && s.datetimeStart.isAfter(to)) return false;
      return true;
    }).toList()
      ..sort((a, b) => b.datetimeStart.compareTo(a.datetimeStart));
  }

  @override
  Future<Session?> getById(String sessionId) async =>
      _sessions.firstWhere((s) => s.sessionId == sessionId);

  @override
  Future<void> save(Session session) async {
    _sessions.removeWhere((s) => s.sessionId == session.sessionId);
    _sessions.add(session);
  }

  @override
  Future<void> delete(String sessionId) async =>
      _sessions.removeWhere((s) => s.sessionId == sessionId);

  @override
  Future<Session?> getPersonalRecord(int athleteId, int distanceMeters) async {
    final filtered = _sessions
        .where((s) => s.athleteId == athleteId && s.distanceMeters == distanceMeters)
        .toList()
      ..sort((a, b) => a.timeMs.compareTo(b.timeMs));
    return filtered.isEmpty ? null : filtered.first;
  }

  @override
  Future<List<Session>> getRecentSessions(int athleteId, int distanceMeters,
      {int limit = 5}) async {
    final filtered = _sessions
        .where((s) => s.athleteId == athleteId && s.distanceMeters == distanceMeters)
        .toList()
      ..sort((a, b) => b.datetimeStart.compareTo(a.datetimeStart));
    return filtered.take(limit).toList();
  }
}

class InMemoryAthleteRepository implements AthleteRepository {
  final List<Athlete> _athletes;

  InMemoryAthleteRepository(this._athletes);

  @override
  Future<List<Athlete>> getAll() async => List.from(_athletes);

  @override
  Future<Athlete?> getById(int id) async =>
      _athletes.where((a) => a.athleteId == id).firstOrNull;

  @override
  Future<void> save(Athlete athlete) async {
    _athletes.removeWhere((a) => a.athleteId == athlete.athleteId);
    _athletes.add(athlete);
  }

  @override
  Future<void> delete(int athleteId) async =>
      _athletes.removeWhere((a) => a.athleteId == athleteId);
}

class InMemoryHrRepository implements HrSampleRepository {
  final Map<String, List<HrSample>> _samples = {};

  @override
  Future<List<HrSample>> getForSession(String sessionId) async =>
      _samples[sessionId] ?? generateHrSamples(
        // Crea muestras on-demand si no existen
        Session(
          sessionId: sessionId,
          athleteId: 0,
          datetimeStart: DateTime.now(),
          distanceMeters: 100,
          timeMs: 11000,
          avgSpeedMps: 9.0,
          avgSpeedKmh: 32.4,
          hrAvgRun: 168,
          hrMaxRun: 185,
          hrAvgRecovery: 120,
          hrMaxRecovery: 145,
          ecv: 0.054,
        ),
      );

  @override
  Future<void> saveAll(List<HrSample> samples) async {
    if (samples.isNotEmpty) {
      _samples[samples.first.sessionId] = samples;
    }
  }
}
