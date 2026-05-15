import 'dart:io';
import 'package:drift/drift.dart';
import 'package:drift_flutter/drift_flutter.dart';
import 'package:path/path.dart' as p;
import 'package:path_provider/path_provider.dart';

part 'app_database.g.dart';

// ─────────────────────────────────────────
// Tables
// ─────────────────────────────────────────

class Athletes extends Table {
  IntColumn get athleteId => integer()();
  TextColumn get name => text()();
  TextColumn get notes => text().nullable()();

  @override
  Set<Column> get primaryKey => {athleteId};
}

class Sessions extends Table {
  TextColumn get sessionId => text()();
  IntColumn get athleteId => integer()();
  DateTimeColumn get datetimeStart => dateTime()();
  IntColumn get distanceMeters => integer()();
  IntColumn get timeMs => integer()();
  RealColumn get avgSpeedMps => real()();
  RealColumn get avgSpeedKmh => real()();
  RealColumn get hrAvgRun => real()();
  RealColumn get hrMaxRun => real()();
  RealColumn get hrAvgRecovery => real()();
  RealColumn get hrMaxRecovery => real()();
  RealColumn get ecv => real()();
  TextColumn get notes => text().nullable()();
  TextColumn get deviceRaceId => text().nullable()();
  BoolColumn get isPR => boolean().withDefault(const Constant(false))();

  @override
  Set<Column> get primaryKey => {sessionId};
}

class HrSamples extends Table {
  IntColumn get id => integer().autoIncrement()();
  TextColumn get sessionId => text()();
  IntColumn get offsetMs => integer()();
  IntColumn get bpm => integer()();
  TextColumn get phase => text()(); // 'run' | 'recovery'
}

// ─────────────────────────────────────────
// Database
// ─────────────────────────────────────────

@DriftDatabase(tables: [Athletes, Sessions, HrSamples])
class AppDatabase extends _$AppDatabase {
  AppDatabase() : super(_openConnection());

  @override
  int get schemaVersion => 1;

  // ── Athletes ──────────────────────────
  Future<List<Athlete>> getAllAthletes() => select(athletes).get();

  Future<Athlete?> getAthleteById(int id) =>
      (select(athletes)..where((t) => t.athleteId.equals(id))).getSingleOrNull();

  Future<void> upsertAthlete(AthletesCompanion a) =>
      into(athletes).insertOnConflictUpdate(a);

  Future<void> deleteAthlete(int id) =>
      (delete(athletes)..where((t) => t.athleteId.equals(id))).go();

  // ── Sessions ─────────────────────────
  Future<List<Session>> querySessions({
    int? athleteId,
    int? distance,
    DateTime? from,
    DateTime? to,
  }) {
    final q = select(sessions)
      ..orderBy([(t) => OrderingTerm.desc(t.datetimeStart)]);
    q.where((t) {
      Expression<bool> cond = const Constant(true);
      if (athleteId != null) cond = cond & t.athleteId.equals(athleteId);
      if (distance != null) cond = cond & t.distanceMeters.equals(distance);
      if (from != null) cond = cond & t.datetimeStart.isBiggerOrEqualValue(from);
      if (to != null) cond = cond & t.datetimeStart.isSmallerOrEqualValue(to);
      return cond;
    });
    return q.get();
  }

  Future<Session?> getSessionById(String id) =>
      (select(sessions)..where((t) => t.sessionId.equals(id))).getSingleOrNull();

  Future<void> upsertSession(SessionsCompanion s) =>
      into(sessions).insertOnConflictUpdate(s);

  Future<void> deleteSession(String id) =>
      (delete(sessions)..where((t) => t.sessionId.equals(id))).go();

  Future<Session?> getPersonalRecord(int athleteId, int distance) async {
    final q = select(sessions)
      ..where((t) => t.athleteId.equals(athleteId) & t.distanceMeters.equals(distance))
      ..orderBy([(t) => OrderingTerm.asc(t.timeMs)])
      ..limit(1);
    return q.getSingleOrNull();
  }

  // ── HR Samples ────────────────────────
  Future<List<HrSample>> getHrSamples(String sessionId) =>
      (select(hrSamples)..where((t) => t.sessionId.equals(sessionId))).get();

  Future<void> insertHrSamples(List<HrSamplesCompanion> rows) =>
      batch((b) => b.insertAll(hrSamples, rows));
}

LazyDatabase _openConnection() {
  return LazyDatabase(() async {
    final dir = await getApplicationDocumentsDirectory();
    final file = File(p.join(dir.path, 'sprint_timer.db'));
    return driftDatabase(file: file);
  });
}
