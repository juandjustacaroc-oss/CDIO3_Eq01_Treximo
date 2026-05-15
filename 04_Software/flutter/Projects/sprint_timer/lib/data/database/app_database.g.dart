// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'app_database.dart';

// ignore_for_file: type=lint
class $AthletesTable extends Athletes with TableInfo<$AthletesTable, Athlete> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $AthletesTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _athleteIdMeta =
      const VerificationMeta('athleteId');
  @override
  late final GeneratedColumn<int> athleteId = GeneratedColumn<int>(
      'athlete_id', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: false);
  static const VerificationMeta _nameMeta = const VerificationMeta('name');
  @override
  late final GeneratedColumn<String> name = GeneratedColumn<String>(
      'name', aliasedName, false,
      type: DriftSqlType.string, requiredDuringInsert: true);
  static const VerificationMeta _notesMeta = const VerificationMeta('notes');
  @override
  late final GeneratedColumn<String> notes = GeneratedColumn<String>(
      'notes', aliasedName, true,
      type: DriftSqlType.string, requiredDuringInsert: false);
  @override
  List<GeneratedColumn> get $columns => [athleteId, name, notes];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'athletes';
  @override
  VerificationContext validateIntegrity(Insertable<Athlete> instance,
      {bool isInserting = false}) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('athlete_id')) {
      context.handle(_athleteIdMeta,
          athleteId.isAcceptableOrUnknown(data['athlete_id']!, _athleteIdMeta));
    }
    if (data.containsKey('name')) {
      context.handle(
          _nameMeta, name.isAcceptableOrUnknown(data['name']!, _nameMeta));
    } else if (isInserting) {
      context.missing(_nameMeta);
    }
    if (data.containsKey('notes')) {
      context.handle(
          _notesMeta, notes.isAcceptableOrUnknown(data['notes']!, _notesMeta));
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {athleteId};
  @override
  Athlete map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return Athlete(
      athleteId: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}athlete_id'])!,
      name: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}name'])!,
      notes: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}notes']),
    );
  }

  @override
  $AthletesTable createAlias(String alias) {
    return $AthletesTable(attachedDatabase, alias);
  }
}

class Athlete extends DataClass implements Insertable<Athlete> {
  final int athleteId;
  final String name;
  final String? notes;
  const Athlete({required this.athleteId, required this.name, this.notes});
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['athlete_id'] = Variable<int>(athleteId);
    map['name'] = Variable<String>(name);
    if (!nullToAbsent || notes != null) {
      map['notes'] = Variable<String>(notes);
    }
    return map;
  }

  AthletesCompanion toCompanion(bool nullToAbsent) {
    return AthletesCompanion(
      athleteId: Value(athleteId),
      name: Value(name),
      notes:
          notes == null && nullToAbsent ? const Value.absent() : Value(notes),
    );
  }

  factory Athlete.fromJson(Map<String, dynamic> json,
      {ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return Athlete(
      athleteId: serializer.fromJson<int>(json['athleteId']),
      name: serializer.fromJson<String>(json['name']),
      notes: serializer.fromJson<String?>(json['notes']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'athleteId': serializer.toJson<int>(athleteId),
      'name': serializer.toJson<String>(name),
      'notes': serializer.toJson<String?>(notes),
    };
  }

  Athlete copyWith(
          {int? athleteId,
          String? name,
          Value<String?> notes = const Value.absent()}) =>
      Athlete(
        athleteId: athleteId ?? this.athleteId,
        name: name ?? this.name,
        notes: notes.present ? notes.value : this.notes,
      );
  Athlete copyWithCompanion(AthletesCompanion data) {
    return Athlete(
      athleteId: data.athleteId.present ? data.athleteId.value : this.athleteId,
      name: data.name.present ? data.name.value : this.name,
      notes: data.notes.present ? data.notes.value : this.notes,
    );
  }

  @override
  String toString() {
    return (StringBuffer('Athlete(')
          ..write('athleteId: $athleteId, ')
          ..write('name: $name, ')
          ..write('notes: $notes')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(athleteId, name, notes);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Athlete &&
          other.athleteId == this.athleteId &&
          other.name == this.name &&
          other.notes == this.notes);
}

class AthletesCompanion extends UpdateCompanion<Athlete> {
  final Value<int> athleteId;
  final Value<String> name;
  final Value<String?> notes;
  const AthletesCompanion({
    this.athleteId = const Value.absent(),
    this.name = const Value.absent(),
    this.notes = const Value.absent(),
  });
  AthletesCompanion.insert({
    this.athleteId = const Value.absent(),
    required String name,
    this.notes = const Value.absent(),
  }) : name = Value(name);
  static Insertable<Athlete> custom({
    Expression<int>? athleteId,
    Expression<String>? name,
    Expression<String>? notes,
  }) {
    return RawValuesInsertable({
      if (athleteId != null) 'athlete_id': athleteId,
      if (name != null) 'name': name,
      if (notes != null) 'notes': notes,
    });
  }

  AthletesCompanion copyWith(
      {Value<int>? athleteId, Value<String>? name, Value<String?>? notes}) {
    return AthletesCompanion(
      athleteId: athleteId ?? this.athleteId,
      name: name ?? this.name,
      notes: notes ?? this.notes,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (athleteId.present) {
      map['athlete_id'] = Variable<int>(athleteId.value);
    }
    if (name.present) {
      map['name'] = Variable<String>(name.value);
    }
    if (notes.present) {
      map['notes'] = Variable<String>(notes.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('AthletesCompanion(')
          ..write('athleteId: $athleteId, ')
          ..write('name: $name, ')
          ..write('notes: $notes')
          ..write(')'))
        .toString();
  }
}

class $SessionsTable extends Sessions with TableInfo<$SessionsTable, Session> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $SessionsTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _sessionIdMeta =
      const VerificationMeta('sessionId');
  @override
  late final GeneratedColumn<String> sessionId = GeneratedColumn<String>(
      'session_id', aliasedName, false,
      type: DriftSqlType.string, requiredDuringInsert: true);
  static const VerificationMeta _athleteIdMeta =
      const VerificationMeta('athleteId');
  @override
  late final GeneratedColumn<int> athleteId = GeneratedColumn<int>(
      'athlete_id', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _datetimeStartMeta =
      const VerificationMeta('datetimeStart');
  @override
  late final GeneratedColumn<DateTime> datetimeStart =
      GeneratedColumn<DateTime>('datetime_start', aliasedName, false,
          type: DriftSqlType.dateTime, requiredDuringInsert: true);
  static const VerificationMeta _distanceMetersMeta =
      const VerificationMeta('distanceMeters');
  @override
  late final GeneratedColumn<double> distanceMeters = GeneratedColumn<double>(
      'distance_meters', aliasedName, false,
      type: DriftSqlType.double, requiredDuringInsert: true);
  static const VerificationMeta _timeMsMeta = const VerificationMeta('timeMs');
  @override
  late final GeneratedColumn<int> timeMs = GeneratedColumn<int>(
      'time_ms', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _avgSpeedMpsMeta =
      const VerificationMeta('avgSpeedMps');
  @override
  late final GeneratedColumn<double> avgSpeedMps = GeneratedColumn<double>(
      'avg_speed_mps', aliasedName, false,
      type: DriftSqlType.double, requiredDuringInsert: true);
  static const VerificationMeta _avgSpeedKmhMeta =
      const VerificationMeta('avgSpeedKmh');
  @override
  late final GeneratedColumn<double> avgSpeedKmh = GeneratedColumn<double>(
      'avg_speed_kmh', aliasedName, false,
      type: DriftSqlType.double, requiredDuringInsert: true);
  static const VerificationMeta _bpmFinishMeta =
      const VerificationMeta('bpmFinish');
  @override
  late final GeneratedColumn<int> bpmFinish = GeneratedColumn<int>(
      'bpm_finish', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _bpmRecoveryMeta =
      const VerificationMeta('bpmRecovery');
  @override
  late final GeneratedColumn<int> bpmRecovery = GeneratedColumn<int>(
      'bpm_recovery', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _distanceCalibratedMeta =
      const VerificationMeta('distanceCalibrated');
  @override
  late final GeneratedColumn<bool> distanceCalibrated = GeneratedColumn<bool>(
      'distance_calibrated', aliasedName, false,
      type: DriftSqlType.bool,
      requiredDuringInsert: false,
      defaultConstraints: GeneratedColumn.constraintIsAlways(
          'CHECK ("distance_calibrated" IN (0, 1))'),
      defaultValue: const Constant(true));
  static const VerificationMeta _ecvMeta = const VerificationMeta('ecv');
  @override
  late final GeneratedColumn<double> ecv = GeneratedColumn<double>(
      'ecv', aliasedName, false,
      type: DriftSqlType.double, requiredDuringInsert: true);
  static const VerificationMeta _notesMeta = const VerificationMeta('notes');
  @override
  late final GeneratedColumn<String> notes = GeneratedColumn<String>(
      'notes', aliasedName, true,
      type: DriftSqlType.string, requiredDuringInsert: false);
  static const VerificationMeta _deviceRaceIdMeta =
      const VerificationMeta('deviceRaceId');
  @override
  late final GeneratedColumn<String> deviceRaceId = GeneratedColumn<String>(
      'device_race_id', aliasedName, true,
      type: DriftSqlType.string, requiredDuringInsert: false);
  static const VerificationMeta _isPRMeta = const VerificationMeta('isPR');
  @override
  late final GeneratedColumn<bool> isPR = GeneratedColumn<bool>(
      'is_p_r', aliasedName, false,
      type: DriftSqlType.bool,
      requiredDuringInsert: false,
      defaultConstraints:
          GeneratedColumn.constraintIsAlways('CHECK ("is_p_r" IN (0, 1))'),
      defaultValue: const Constant(false));
  @override
  List<GeneratedColumn> get $columns => [
        sessionId,
        athleteId,
        datetimeStart,
        distanceMeters,
        timeMs,
        avgSpeedMps,
        avgSpeedKmh,
        bpmFinish,
        bpmRecovery,
        distanceCalibrated,
        ecv,
        notes,
        deviceRaceId,
        isPR
      ];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'sessions';
  @override
  VerificationContext validateIntegrity(Insertable<Session> instance,
      {bool isInserting = false}) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('session_id')) {
      context.handle(_sessionIdMeta,
          sessionId.isAcceptableOrUnknown(data['session_id']!, _sessionIdMeta));
    } else if (isInserting) {
      context.missing(_sessionIdMeta);
    }
    if (data.containsKey('athlete_id')) {
      context.handle(_athleteIdMeta,
          athleteId.isAcceptableOrUnknown(data['athlete_id']!, _athleteIdMeta));
    } else if (isInserting) {
      context.missing(_athleteIdMeta);
    }
    if (data.containsKey('datetime_start')) {
      context.handle(
          _datetimeStartMeta,
          datetimeStart.isAcceptableOrUnknown(
              data['datetime_start']!, _datetimeStartMeta));
    } else if (isInserting) {
      context.missing(_datetimeStartMeta);
    }
    if (data.containsKey('distance_meters')) {
      context.handle(
          _distanceMetersMeta,
          distanceMeters.isAcceptableOrUnknown(
              data['distance_meters']!, _distanceMetersMeta));
    } else if (isInserting) {
      context.missing(_distanceMetersMeta);
    }
    if (data.containsKey('time_ms')) {
      context.handle(_timeMsMeta,
          timeMs.isAcceptableOrUnknown(data['time_ms']!, _timeMsMeta));
    } else if (isInserting) {
      context.missing(_timeMsMeta);
    }
    if (data.containsKey('avg_speed_mps')) {
      context.handle(
          _avgSpeedMpsMeta,
          avgSpeedMps.isAcceptableOrUnknown(
              data['avg_speed_mps']!, _avgSpeedMpsMeta));
    } else if (isInserting) {
      context.missing(_avgSpeedMpsMeta);
    }
    if (data.containsKey('avg_speed_kmh')) {
      context.handle(
          _avgSpeedKmhMeta,
          avgSpeedKmh.isAcceptableOrUnknown(
              data['avg_speed_kmh']!, _avgSpeedKmhMeta));
    } else if (isInserting) {
      context.missing(_avgSpeedKmhMeta);
    }
    if (data.containsKey('bpm_finish')) {
      context.handle(_bpmFinishMeta,
          bpmFinish.isAcceptableOrUnknown(data['bpm_finish']!, _bpmFinishMeta));
    } else if (isInserting) {
      context.missing(_bpmFinishMeta);
    }
    if (data.containsKey('bpm_recovery')) {
      context.handle(
          _bpmRecoveryMeta,
          bpmRecovery.isAcceptableOrUnknown(
              data['bpm_recovery']!, _bpmRecoveryMeta));
    } else if (isInserting) {
      context.missing(_bpmRecoveryMeta);
    }
    if (data.containsKey('distance_calibrated')) {
      context.handle(
          _distanceCalibratedMeta,
          distanceCalibrated.isAcceptableOrUnknown(
              data['distance_calibrated']!, _distanceCalibratedMeta));
    }
    if (data.containsKey('ecv')) {
      context.handle(
          _ecvMeta, ecv.isAcceptableOrUnknown(data['ecv']!, _ecvMeta));
    } else if (isInserting) {
      context.missing(_ecvMeta);
    }
    if (data.containsKey('notes')) {
      context.handle(
          _notesMeta, notes.isAcceptableOrUnknown(data['notes']!, _notesMeta));
    }
    if (data.containsKey('device_race_id')) {
      context.handle(
          _deviceRaceIdMeta,
          deviceRaceId.isAcceptableOrUnknown(
              data['device_race_id']!, _deviceRaceIdMeta));
    }
    if (data.containsKey('is_p_r')) {
      context.handle(
          _isPRMeta, isPR.isAcceptableOrUnknown(data['is_p_r']!, _isPRMeta));
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {sessionId};
  @override
  Session map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return Session(
      sessionId: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}session_id'])!,
      athleteId: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}athlete_id'])!,
      datetimeStart: attachedDatabase.typeMapping.read(
          DriftSqlType.dateTime, data['${effectivePrefix}datetime_start'])!,
      distanceMeters: attachedDatabase.typeMapping.read(
          DriftSqlType.double, data['${effectivePrefix}distance_meters'])!,
      timeMs: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}time_ms'])!,
      avgSpeedMps: attachedDatabase.typeMapping
          .read(DriftSqlType.double, data['${effectivePrefix}avg_speed_mps'])!,
      avgSpeedKmh: attachedDatabase.typeMapping
          .read(DriftSqlType.double, data['${effectivePrefix}avg_speed_kmh'])!,
      bpmFinish: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}bpm_finish'])!,
      bpmRecovery: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}bpm_recovery'])!,
      distanceCalibrated: attachedDatabase.typeMapping.read(
          DriftSqlType.bool, data['${effectivePrefix}distance_calibrated'])!,
      ecv: attachedDatabase.typeMapping
          .read(DriftSqlType.double, data['${effectivePrefix}ecv'])!,
      notes: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}notes']),
      deviceRaceId: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}device_race_id']),
      isPR: attachedDatabase.typeMapping
          .read(DriftSqlType.bool, data['${effectivePrefix}is_p_r'])!,
    );
  }

  @override
  $SessionsTable createAlias(String alias) {
    return $SessionsTable(attachedDatabase, alias);
  }
}

class Session extends DataClass implements Insertable<Session> {
  final String sessionId;
  final int athleteId;
  final DateTime datetimeStart;
  final double distanceMeters;
  final int timeMs;
  final double avgSpeedMps;
  final double avgSpeedKmh;
  final int bpmFinish;
  final int bpmRecovery;
  final bool distanceCalibrated;
  final double ecv;
  final String? notes;
  final String? deviceRaceId;
  final bool isPR;
  const Session(
      {required this.sessionId,
      required this.athleteId,
      required this.datetimeStart,
      required this.distanceMeters,
      required this.timeMs,
      required this.avgSpeedMps,
      required this.avgSpeedKmh,
      required this.bpmFinish,
      required this.bpmRecovery,
      required this.distanceCalibrated,
      required this.ecv,
      this.notes,
      this.deviceRaceId,
      required this.isPR});
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['session_id'] = Variable<String>(sessionId);
    map['athlete_id'] = Variable<int>(athleteId);
    map['datetime_start'] = Variable<DateTime>(datetimeStart);
    map['distance_meters'] = Variable<double>(distanceMeters);
    map['time_ms'] = Variable<int>(timeMs);
    map['avg_speed_mps'] = Variable<double>(avgSpeedMps);
    map['avg_speed_kmh'] = Variable<double>(avgSpeedKmh);
    map['bpm_finish'] = Variable<int>(bpmFinish);
    map['bpm_recovery'] = Variable<int>(bpmRecovery);
    map['distance_calibrated'] = Variable<bool>(distanceCalibrated);
    map['ecv'] = Variable<double>(ecv);
    if (!nullToAbsent || notes != null) {
      map['notes'] = Variable<String>(notes);
    }
    if (!nullToAbsent || deviceRaceId != null) {
      map['device_race_id'] = Variable<String>(deviceRaceId);
    }
    map['is_p_r'] = Variable<bool>(isPR);
    return map;
  }

  SessionsCompanion toCompanion(bool nullToAbsent) {
    return SessionsCompanion(
      sessionId: Value(sessionId),
      athleteId: Value(athleteId),
      datetimeStart: Value(datetimeStart),
      distanceMeters: Value(distanceMeters),
      timeMs: Value(timeMs),
      avgSpeedMps: Value(avgSpeedMps),
      avgSpeedKmh: Value(avgSpeedKmh),
      bpmFinish: Value(bpmFinish),
      bpmRecovery: Value(bpmRecovery),
      distanceCalibrated: Value(distanceCalibrated),
      ecv: Value(ecv),
      notes:
          notes == null && nullToAbsent ? const Value.absent() : Value(notes),
      deviceRaceId: deviceRaceId == null && nullToAbsent
          ? const Value.absent()
          : Value(deviceRaceId),
      isPR: Value(isPR),
    );
  }

  factory Session.fromJson(Map<String, dynamic> json,
      {ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return Session(
      sessionId: serializer.fromJson<String>(json['sessionId']),
      athleteId: serializer.fromJson<int>(json['athleteId']),
      datetimeStart: serializer.fromJson<DateTime>(json['datetimeStart']),
      distanceMeters: serializer.fromJson<double>(json['distanceMeters']),
      timeMs: serializer.fromJson<int>(json['timeMs']),
      avgSpeedMps: serializer.fromJson<double>(json['avgSpeedMps']),
      avgSpeedKmh: serializer.fromJson<double>(json['avgSpeedKmh']),
      bpmFinish: serializer.fromJson<int>(json['bpmFinish']),
      bpmRecovery: serializer.fromJson<int>(json['bpmRecovery']),
      distanceCalibrated: serializer.fromJson<bool>(json['distanceCalibrated']),
      ecv: serializer.fromJson<double>(json['ecv']),
      notes: serializer.fromJson<String?>(json['notes']),
      deviceRaceId: serializer.fromJson<String?>(json['deviceRaceId']),
      isPR: serializer.fromJson<bool>(json['isPR']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'sessionId': serializer.toJson<String>(sessionId),
      'athleteId': serializer.toJson<int>(athleteId),
      'datetimeStart': serializer.toJson<DateTime>(datetimeStart),
      'distanceMeters': serializer.toJson<double>(distanceMeters),
      'timeMs': serializer.toJson<int>(timeMs),
      'avgSpeedMps': serializer.toJson<double>(avgSpeedMps),
      'avgSpeedKmh': serializer.toJson<double>(avgSpeedKmh),
      'bpmFinish': serializer.toJson<int>(bpmFinish),
      'bpmRecovery': serializer.toJson<int>(bpmRecovery),
      'distanceCalibrated': serializer.toJson<bool>(distanceCalibrated),
      'ecv': serializer.toJson<double>(ecv),
      'notes': serializer.toJson<String?>(notes),
      'deviceRaceId': serializer.toJson<String?>(deviceRaceId),
      'isPR': serializer.toJson<bool>(isPR),
    };
  }

  Session copyWith(
          {String? sessionId,
          int? athleteId,
          DateTime? datetimeStart,
          double? distanceMeters,
          int? timeMs,
          double? avgSpeedMps,
          double? avgSpeedKmh,
          int? bpmFinish,
          int? bpmRecovery,
          bool? distanceCalibrated,
          double? ecv,
          Value<String?> notes = const Value.absent(),
          Value<String?> deviceRaceId = const Value.absent(),
          bool? isPR}) =>
      Session(
        sessionId: sessionId ?? this.sessionId,
        athleteId: athleteId ?? this.athleteId,
        datetimeStart: datetimeStart ?? this.datetimeStart,
        distanceMeters: distanceMeters ?? this.distanceMeters,
        timeMs: timeMs ?? this.timeMs,
        avgSpeedMps: avgSpeedMps ?? this.avgSpeedMps,
        avgSpeedKmh: avgSpeedKmh ?? this.avgSpeedKmh,
        bpmFinish: bpmFinish ?? this.bpmFinish,
        bpmRecovery: bpmRecovery ?? this.bpmRecovery,
        distanceCalibrated: distanceCalibrated ?? this.distanceCalibrated,
        ecv: ecv ?? this.ecv,
        notes: notes.present ? notes.value : this.notes,
        deviceRaceId:
            deviceRaceId.present ? deviceRaceId.value : this.deviceRaceId,
        isPR: isPR ?? this.isPR,
      );
  Session copyWithCompanion(SessionsCompanion data) {
    return Session(
      sessionId: data.sessionId.present ? data.sessionId.value : this.sessionId,
      athleteId: data.athleteId.present ? data.athleteId.value : this.athleteId,
      datetimeStart: data.datetimeStart.present
          ? data.datetimeStart.value
          : this.datetimeStart,
      distanceMeters: data.distanceMeters.present
          ? data.distanceMeters.value
          : this.distanceMeters,
      timeMs: data.timeMs.present ? data.timeMs.value : this.timeMs,
      avgSpeedMps:
          data.avgSpeedMps.present ? data.avgSpeedMps.value : this.avgSpeedMps,
      avgSpeedKmh:
          data.avgSpeedKmh.present ? data.avgSpeedKmh.value : this.avgSpeedKmh,
      bpmFinish: data.bpmFinish.present ? data.bpmFinish.value : this.bpmFinish,
      bpmRecovery:
          data.bpmRecovery.present ? data.bpmRecovery.value : this.bpmRecovery,
      distanceCalibrated: data.distanceCalibrated.present
          ? data.distanceCalibrated.value
          : this.distanceCalibrated,
      ecv: data.ecv.present ? data.ecv.value : this.ecv,
      notes: data.notes.present ? data.notes.value : this.notes,
      deviceRaceId: data.deviceRaceId.present
          ? data.deviceRaceId.value
          : this.deviceRaceId,
      isPR: data.isPR.present ? data.isPR.value : this.isPR,
    );
  }

  @override
  String toString() {
    return (StringBuffer('Session(')
          ..write('sessionId: $sessionId, ')
          ..write('athleteId: $athleteId, ')
          ..write('datetimeStart: $datetimeStart, ')
          ..write('distanceMeters: $distanceMeters, ')
          ..write('timeMs: $timeMs, ')
          ..write('avgSpeedMps: $avgSpeedMps, ')
          ..write('avgSpeedKmh: $avgSpeedKmh, ')
          ..write('bpmFinish: $bpmFinish, ')
          ..write('bpmRecovery: $bpmRecovery, ')
          ..write('distanceCalibrated: $distanceCalibrated, ')
          ..write('ecv: $ecv, ')
          ..write('notes: $notes, ')
          ..write('deviceRaceId: $deviceRaceId, ')
          ..write('isPR: $isPR')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(
      sessionId,
      athleteId,
      datetimeStart,
      distanceMeters,
      timeMs,
      avgSpeedMps,
      avgSpeedKmh,
      bpmFinish,
      bpmRecovery,
      distanceCalibrated,
      ecv,
      notes,
      deviceRaceId,
      isPR);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is Session &&
          other.sessionId == this.sessionId &&
          other.athleteId == this.athleteId &&
          other.datetimeStart == this.datetimeStart &&
          other.distanceMeters == this.distanceMeters &&
          other.timeMs == this.timeMs &&
          other.avgSpeedMps == this.avgSpeedMps &&
          other.avgSpeedKmh == this.avgSpeedKmh &&
          other.bpmFinish == this.bpmFinish &&
          other.bpmRecovery == this.bpmRecovery &&
          other.distanceCalibrated == this.distanceCalibrated &&
          other.ecv == this.ecv &&
          other.notes == this.notes &&
          other.deviceRaceId == this.deviceRaceId &&
          other.isPR == this.isPR);
}

class SessionsCompanion extends UpdateCompanion<Session> {
  final Value<String> sessionId;
  final Value<int> athleteId;
  final Value<DateTime> datetimeStart;
  final Value<double> distanceMeters;
  final Value<int> timeMs;
  final Value<double> avgSpeedMps;
  final Value<double> avgSpeedKmh;
  final Value<int> bpmFinish;
  final Value<int> bpmRecovery;
  final Value<bool> distanceCalibrated;
  final Value<double> ecv;
  final Value<String?> notes;
  final Value<String?> deviceRaceId;
  final Value<bool> isPR;
  final Value<int> rowid;
  const SessionsCompanion({
    this.sessionId = const Value.absent(),
    this.athleteId = const Value.absent(),
    this.datetimeStart = const Value.absent(),
    this.distanceMeters = const Value.absent(),
    this.timeMs = const Value.absent(),
    this.avgSpeedMps = const Value.absent(),
    this.avgSpeedKmh = const Value.absent(),
    this.bpmFinish = const Value.absent(),
    this.bpmRecovery = const Value.absent(),
    this.distanceCalibrated = const Value.absent(),
    this.ecv = const Value.absent(),
    this.notes = const Value.absent(),
    this.deviceRaceId = const Value.absent(),
    this.isPR = const Value.absent(),
    this.rowid = const Value.absent(),
  });
  SessionsCompanion.insert({
    required String sessionId,
    required int athleteId,
    required DateTime datetimeStart,
    required double distanceMeters,
    required int timeMs,
    required double avgSpeedMps,
    required double avgSpeedKmh,
    required int bpmFinish,
    required int bpmRecovery,
    this.distanceCalibrated = const Value.absent(),
    required double ecv,
    this.notes = const Value.absent(),
    this.deviceRaceId = const Value.absent(),
    this.isPR = const Value.absent(),
    this.rowid = const Value.absent(),
  })  : sessionId = Value(sessionId),
        athleteId = Value(athleteId),
        datetimeStart = Value(datetimeStart),
        distanceMeters = Value(distanceMeters),
        timeMs = Value(timeMs),
        avgSpeedMps = Value(avgSpeedMps),
        avgSpeedKmh = Value(avgSpeedKmh),
        bpmFinish = Value(bpmFinish),
        bpmRecovery = Value(bpmRecovery),
        ecv = Value(ecv);
  static Insertable<Session> custom({
    Expression<String>? sessionId,
    Expression<int>? athleteId,
    Expression<DateTime>? datetimeStart,
    Expression<double>? distanceMeters,
    Expression<int>? timeMs,
    Expression<double>? avgSpeedMps,
    Expression<double>? avgSpeedKmh,
    Expression<int>? bpmFinish,
    Expression<int>? bpmRecovery,
    Expression<bool>? distanceCalibrated,
    Expression<double>? ecv,
    Expression<String>? notes,
    Expression<String>? deviceRaceId,
    Expression<bool>? isPR,
    Expression<int>? rowid,
  }) {
    return RawValuesInsertable({
      if (sessionId != null) 'session_id': sessionId,
      if (athleteId != null) 'athlete_id': athleteId,
      if (datetimeStart != null) 'datetime_start': datetimeStart,
      if (distanceMeters != null) 'distance_meters': distanceMeters,
      if (timeMs != null) 'time_ms': timeMs,
      if (avgSpeedMps != null) 'avg_speed_mps': avgSpeedMps,
      if (avgSpeedKmh != null) 'avg_speed_kmh': avgSpeedKmh,
      if (bpmFinish != null) 'bpm_finish': bpmFinish,
      if (bpmRecovery != null) 'bpm_recovery': bpmRecovery,
      if (distanceCalibrated != null) 'distance_calibrated': distanceCalibrated,
      if (ecv != null) 'ecv': ecv,
      if (notes != null) 'notes': notes,
      if (deviceRaceId != null) 'device_race_id': deviceRaceId,
      if (isPR != null) 'is_p_r': isPR,
      if (rowid != null) 'rowid': rowid,
    });
  }

  SessionsCompanion copyWith(
      {Value<String>? sessionId,
      Value<int>? athleteId,
      Value<DateTime>? datetimeStart,
      Value<double>? distanceMeters,
      Value<int>? timeMs,
      Value<double>? avgSpeedMps,
      Value<double>? avgSpeedKmh,
      Value<int>? bpmFinish,
      Value<int>? bpmRecovery,
      Value<bool>? distanceCalibrated,
      Value<double>? ecv,
      Value<String?>? notes,
      Value<String?>? deviceRaceId,
      Value<bool>? isPR,
      Value<int>? rowid}) {
    return SessionsCompanion(
      sessionId: sessionId ?? this.sessionId,
      athleteId: athleteId ?? this.athleteId,
      datetimeStart: datetimeStart ?? this.datetimeStart,
      distanceMeters: distanceMeters ?? this.distanceMeters,
      timeMs: timeMs ?? this.timeMs,
      avgSpeedMps: avgSpeedMps ?? this.avgSpeedMps,
      avgSpeedKmh: avgSpeedKmh ?? this.avgSpeedKmh,
      bpmFinish: bpmFinish ?? this.bpmFinish,
      bpmRecovery: bpmRecovery ?? this.bpmRecovery,
      distanceCalibrated: distanceCalibrated ?? this.distanceCalibrated,
      ecv: ecv ?? this.ecv,
      notes: notes ?? this.notes,
      deviceRaceId: deviceRaceId ?? this.deviceRaceId,
      isPR: isPR ?? this.isPR,
      rowid: rowid ?? this.rowid,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (sessionId.present) {
      map['session_id'] = Variable<String>(sessionId.value);
    }
    if (athleteId.present) {
      map['athlete_id'] = Variable<int>(athleteId.value);
    }
    if (datetimeStart.present) {
      map['datetime_start'] = Variable<DateTime>(datetimeStart.value);
    }
    if (distanceMeters.present) {
      map['distance_meters'] = Variable<double>(distanceMeters.value);
    }
    if (timeMs.present) {
      map['time_ms'] = Variable<int>(timeMs.value);
    }
    if (avgSpeedMps.present) {
      map['avg_speed_mps'] = Variable<double>(avgSpeedMps.value);
    }
    if (avgSpeedKmh.present) {
      map['avg_speed_kmh'] = Variable<double>(avgSpeedKmh.value);
    }
    if (bpmFinish.present) {
      map['bpm_finish'] = Variable<int>(bpmFinish.value);
    }
    if (bpmRecovery.present) {
      map['bpm_recovery'] = Variable<int>(bpmRecovery.value);
    }
    if (distanceCalibrated.present) {
      map['distance_calibrated'] = Variable<bool>(distanceCalibrated.value);
    }
    if (ecv.present) {
      map['ecv'] = Variable<double>(ecv.value);
    }
    if (notes.present) {
      map['notes'] = Variable<String>(notes.value);
    }
    if (deviceRaceId.present) {
      map['device_race_id'] = Variable<String>(deviceRaceId.value);
    }
    if (isPR.present) {
      map['is_p_r'] = Variable<bool>(isPR.value);
    }
    if (rowid.present) {
      map['rowid'] = Variable<int>(rowid.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('SessionsCompanion(')
          ..write('sessionId: $sessionId, ')
          ..write('athleteId: $athleteId, ')
          ..write('datetimeStart: $datetimeStart, ')
          ..write('distanceMeters: $distanceMeters, ')
          ..write('timeMs: $timeMs, ')
          ..write('avgSpeedMps: $avgSpeedMps, ')
          ..write('avgSpeedKmh: $avgSpeedKmh, ')
          ..write('bpmFinish: $bpmFinish, ')
          ..write('bpmRecovery: $bpmRecovery, ')
          ..write('distanceCalibrated: $distanceCalibrated, ')
          ..write('ecv: $ecv, ')
          ..write('notes: $notes, ')
          ..write('deviceRaceId: $deviceRaceId, ')
          ..write('isPR: $isPR, ')
          ..write('rowid: $rowid')
          ..write(')'))
        .toString();
  }
}

class $HrSamplesTable extends HrSamples
    with TableInfo<$HrSamplesTable, HrSample> {
  @override
  final GeneratedDatabase attachedDatabase;
  final String? _alias;
  $HrSamplesTable(this.attachedDatabase, [this._alias]);
  static const VerificationMeta _idMeta = const VerificationMeta('id');
  @override
  late final GeneratedColumn<int> id = GeneratedColumn<int>(
      'id', aliasedName, false,
      hasAutoIncrement: true,
      type: DriftSqlType.int,
      requiredDuringInsert: false,
      defaultConstraints:
          GeneratedColumn.constraintIsAlways('PRIMARY KEY AUTOINCREMENT'));
  static const VerificationMeta _sessionIdMeta =
      const VerificationMeta('sessionId');
  @override
  late final GeneratedColumn<String> sessionId = GeneratedColumn<String>(
      'session_id', aliasedName, false,
      type: DriftSqlType.string, requiredDuringInsert: true);
  static const VerificationMeta _offsetMsMeta =
      const VerificationMeta('offsetMs');
  @override
  late final GeneratedColumn<int> offsetMs = GeneratedColumn<int>(
      'offset_ms', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _bpmMeta = const VerificationMeta('bpm');
  @override
  late final GeneratedColumn<int> bpm = GeneratedColumn<int>(
      'bpm', aliasedName, false,
      type: DriftSqlType.int, requiredDuringInsert: true);
  static const VerificationMeta _phaseMeta = const VerificationMeta('phase');
  @override
  late final GeneratedColumn<String> phase = GeneratedColumn<String>(
      'phase', aliasedName, false,
      type: DriftSqlType.string, requiredDuringInsert: true);
  @override
  List<GeneratedColumn> get $columns => [id, sessionId, offsetMs, bpm, phase];
  @override
  String get aliasedName => _alias ?? actualTableName;
  @override
  String get actualTableName => $name;
  static const String $name = 'hr_samples';
  @override
  VerificationContext validateIntegrity(Insertable<HrSample> instance,
      {bool isInserting = false}) {
    final context = VerificationContext();
    final data = instance.toColumns(true);
    if (data.containsKey('id')) {
      context.handle(_idMeta, id.isAcceptableOrUnknown(data['id']!, _idMeta));
    }
    if (data.containsKey('session_id')) {
      context.handle(_sessionIdMeta,
          sessionId.isAcceptableOrUnknown(data['session_id']!, _sessionIdMeta));
    } else if (isInserting) {
      context.missing(_sessionIdMeta);
    }
    if (data.containsKey('offset_ms')) {
      context.handle(_offsetMsMeta,
          offsetMs.isAcceptableOrUnknown(data['offset_ms']!, _offsetMsMeta));
    } else if (isInserting) {
      context.missing(_offsetMsMeta);
    }
    if (data.containsKey('bpm')) {
      context.handle(
          _bpmMeta, bpm.isAcceptableOrUnknown(data['bpm']!, _bpmMeta));
    } else if (isInserting) {
      context.missing(_bpmMeta);
    }
    if (data.containsKey('phase')) {
      context.handle(
          _phaseMeta, phase.isAcceptableOrUnknown(data['phase']!, _phaseMeta));
    } else if (isInserting) {
      context.missing(_phaseMeta);
    }
    return context;
  }

  @override
  Set<GeneratedColumn> get $primaryKey => {id};
  @override
  HrSample map(Map<String, dynamic> data, {String? tablePrefix}) {
    final effectivePrefix = tablePrefix != null ? '$tablePrefix.' : '';
    return HrSample(
      id: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}id'])!,
      sessionId: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}session_id'])!,
      offsetMs: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}offset_ms'])!,
      bpm: attachedDatabase.typeMapping
          .read(DriftSqlType.int, data['${effectivePrefix}bpm'])!,
      phase: attachedDatabase.typeMapping
          .read(DriftSqlType.string, data['${effectivePrefix}phase'])!,
    );
  }

  @override
  $HrSamplesTable createAlias(String alias) {
    return $HrSamplesTable(attachedDatabase, alias);
  }
}

class HrSample extends DataClass implements Insertable<HrSample> {
  final int id;
  final String sessionId;
  final int offsetMs;
  final int bpm;
  final String phase;
  const HrSample(
      {required this.id,
      required this.sessionId,
      required this.offsetMs,
      required this.bpm,
      required this.phase});
  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    map['id'] = Variable<int>(id);
    map['session_id'] = Variable<String>(sessionId);
    map['offset_ms'] = Variable<int>(offsetMs);
    map['bpm'] = Variable<int>(bpm);
    map['phase'] = Variable<String>(phase);
    return map;
  }

  HrSamplesCompanion toCompanion(bool nullToAbsent) {
    return HrSamplesCompanion(
      id: Value(id),
      sessionId: Value(sessionId),
      offsetMs: Value(offsetMs),
      bpm: Value(bpm),
      phase: Value(phase),
    );
  }

  factory HrSample.fromJson(Map<String, dynamic> json,
      {ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return HrSample(
      id: serializer.fromJson<int>(json['id']),
      sessionId: serializer.fromJson<String>(json['sessionId']),
      offsetMs: serializer.fromJson<int>(json['offsetMs']),
      bpm: serializer.fromJson<int>(json['bpm']),
      phase: serializer.fromJson<String>(json['phase']),
    );
  }
  @override
  Map<String, dynamic> toJson({ValueSerializer? serializer}) {
    serializer ??= driftRuntimeOptions.defaultSerializer;
    return <String, dynamic>{
      'id': serializer.toJson<int>(id),
      'sessionId': serializer.toJson<String>(sessionId),
      'offsetMs': serializer.toJson<int>(offsetMs),
      'bpm': serializer.toJson<int>(bpm),
      'phase': serializer.toJson<String>(phase),
    };
  }

  HrSample copyWith(
          {int? id,
          String? sessionId,
          int? offsetMs,
          int? bpm,
          String? phase}) =>
      HrSample(
        id: id ?? this.id,
        sessionId: sessionId ?? this.sessionId,
        offsetMs: offsetMs ?? this.offsetMs,
        bpm: bpm ?? this.bpm,
        phase: phase ?? this.phase,
      );
  HrSample copyWithCompanion(HrSamplesCompanion data) {
    return HrSample(
      id: data.id.present ? data.id.value : this.id,
      sessionId: data.sessionId.present ? data.sessionId.value : this.sessionId,
      offsetMs: data.offsetMs.present ? data.offsetMs.value : this.offsetMs,
      bpm: data.bpm.present ? data.bpm.value : this.bpm,
      phase: data.phase.present ? data.phase.value : this.phase,
    );
  }

  @override
  String toString() {
    return (StringBuffer('HrSample(')
          ..write('id: $id, ')
          ..write('sessionId: $sessionId, ')
          ..write('offsetMs: $offsetMs, ')
          ..write('bpm: $bpm, ')
          ..write('phase: $phase')
          ..write(')'))
        .toString();
  }

  @override
  int get hashCode => Object.hash(id, sessionId, offsetMs, bpm, phase);
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      (other is HrSample &&
          other.id == this.id &&
          other.sessionId == this.sessionId &&
          other.offsetMs == this.offsetMs &&
          other.bpm == this.bpm &&
          other.phase == this.phase);
}

class HrSamplesCompanion extends UpdateCompanion<HrSample> {
  final Value<int> id;
  final Value<String> sessionId;
  final Value<int> offsetMs;
  final Value<int> bpm;
  final Value<String> phase;
  const HrSamplesCompanion({
    this.id = const Value.absent(),
    this.sessionId = const Value.absent(),
    this.offsetMs = const Value.absent(),
    this.bpm = const Value.absent(),
    this.phase = const Value.absent(),
  });
  HrSamplesCompanion.insert({
    this.id = const Value.absent(),
    required String sessionId,
    required int offsetMs,
    required int bpm,
    required String phase,
  })  : sessionId = Value(sessionId),
        offsetMs = Value(offsetMs),
        bpm = Value(bpm),
        phase = Value(phase);
  static Insertable<HrSample> custom({
    Expression<int>? id,
    Expression<String>? sessionId,
    Expression<int>? offsetMs,
    Expression<int>? bpm,
    Expression<String>? phase,
  }) {
    return RawValuesInsertable({
      if (id != null) 'id': id,
      if (sessionId != null) 'session_id': sessionId,
      if (offsetMs != null) 'offset_ms': offsetMs,
      if (bpm != null) 'bpm': bpm,
      if (phase != null) 'phase': phase,
    });
  }

  HrSamplesCompanion copyWith(
      {Value<int>? id,
      Value<String>? sessionId,
      Value<int>? offsetMs,
      Value<int>? bpm,
      Value<String>? phase}) {
    return HrSamplesCompanion(
      id: id ?? this.id,
      sessionId: sessionId ?? this.sessionId,
      offsetMs: offsetMs ?? this.offsetMs,
      bpm: bpm ?? this.bpm,
      phase: phase ?? this.phase,
    );
  }

  @override
  Map<String, Expression> toColumns(bool nullToAbsent) {
    final map = <String, Expression>{};
    if (id.present) {
      map['id'] = Variable<int>(id.value);
    }
    if (sessionId.present) {
      map['session_id'] = Variable<String>(sessionId.value);
    }
    if (offsetMs.present) {
      map['offset_ms'] = Variable<int>(offsetMs.value);
    }
    if (bpm.present) {
      map['bpm'] = Variable<int>(bpm.value);
    }
    if (phase.present) {
      map['phase'] = Variable<String>(phase.value);
    }
    return map;
  }

  @override
  String toString() {
    return (StringBuffer('HrSamplesCompanion(')
          ..write('id: $id, ')
          ..write('sessionId: $sessionId, ')
          ..write('offsetMs: $offsetMs, ')
          ..write('bpm: $bpm, ')
          ..write('phase: $phase')
          ..write(')'))
        .toString();
  }
}

abstract class _$AppDatabase extends GeneratedDatabase {
  _$AppDatabase(QueryExecutor e) : super(e);
  $AppDatabaseManager get managers => $AppDatabaseManager(this);
  late final $AthletesTable athletes = $AthletesTable(this);
  late final $SessionsTable sessions = $SessionsTable(this);
  late final $HrSamplesTable hrSamples = $HrSamplesTable(this);
  @override
  Iterable<TableInfo<Table, Object?>> get allTables =>
      allSchemaEntities.whereType<TableInfo<Table, Object?>>();
  @override
  List<DatabaseSchemaEntity> get allSchemaEntities =>
      [athletes, sessions, hrSamples];
}

typedef $$AthletesTableCreateCompanionBuilder = AthletesCompanion Function({
  Value<int> athleteId,
  required String name,
  Value<String?> notes,
});
typedef $$AthletesTableUpdateCompanionBuilder = AthletesCompanion Function({
  Value<int> athleteId,
  Value<String> name,
  Value<String?> notes,
});

class $$AthletesTableTableManager extends RootTableManager<
    _$AppDatabase,
    $AthletesTable,
    Athlete,
    $$AthletesTableFilterComposer,
    $$AthletesTableOrderingComposer,
    $$AthletesTableCreateCompanionBuilder,
    $$AthletesTableUpdateCompanionBuilder> {
  $$AthletesTableTableManager(_$AppDatabase db, $AthletesTable table)
      : super(TableManagerState(
          db: db,
          table: table,
          filteringComposer:
              $$AthletesTableFilterComposer(ComposerState(db, table)),
          orderingComposer:
              $$AthletesTableOrderingComposer(ComposerState(db, table)),
          updateCompanionCallback: ({
            Value<int> athleteId = const Value.absent(),
            Value<String> name = const Value.absent(),
            Value<String?> notes = const Value.absent(),
          }) =>
              AthletesCompanion(
            athleteId: athleteId,
            name: name,
            notes: notes,
          ),
          createCompanionCallback: ({
            Value<int> athleteId = const Value.absent(),
            required String name,
            Value<String?> notes = const Value.absent(),
          }) =>
              AthletesCompanion.insert(
            athleteId: athleteId,
            name: name,
            notes: notes,
          ),
        ));
}

class $$AthletesTableFilterComposer
    extends FilterComposer<_$AppDatabase, $AthletesTable> {
  $$AthletesTableFilterComposer(super.$state);
  ColumnFilters<int> get athleteId => $state.composableBuilder(
      column: $state.table.athleteId,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get name => $state.composableBuilder(
      column: $state.table.name,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get notes => $state.composableBuilder(
      column: $state.table.notes,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));
}

class $$AthletesTableOrderingComposer
    extends OrderingComposer<_$AppDatabase, $AthletesTable> {
  $$AthletesTableOrderingComposer(super.$state);
  ColumnOrderings<int> get athleteId => $state.composableBuilder(
      column: $state.table.athleteId,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get name => $state.composableBuilder(
      column: $state.table.name,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get notes => $state.composableBuilder(
      column: $state.table.notes,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));
}

typedef $$SessionsTableCreateCompanionBuilder = SessionsCompanion Function({
  required String sessionId,
  required int athleteId,
  required DateTime datetimeStart,
  required double distanceMeters,
  required int timeMs,
  required double avgSpeedMps,
  required double avgSpeedKmh,
  required int bpmFinish,
  required int bpmRecovery,
  Value<bool> distanceCalibrated,
  required double ecv,
  Value<String?> notes,
  Value<String?> deviceRaceId,
  Value<bool> isPR,
  Value<int> rowid,
});
typedef $$SessionsTableUpdateCompanionBuilder = SessionsCompanion Function({
  Value<String> sessionId,
  Value<int> athleteId,
  Value<DateTime> datetimeStart,
  Value<double> distanceMeters,
  Value<int> timeMs,
  Value<double> avgSpeedMps,
  Value<double> avgSpeedKmh,
  Value<int> bpmFinish,
  Value<int> bpmRecovery,
  Value<bool> distanceCalibrated,
  Value<double> ecv,
  Value<String?> notes,
  Value<String?> deviceRaceId,
  Value<bool> isPR,
  Value<int> rowid,
});

class $$SessionsTableTableManager extends RootTableManager<
    _$AppDatabase,
    $SessionsTable,
    Session,
    $$SessionsTableFilterComposer,
    $$SessionsTableOrderingComposer,
    $$SessionsTableCreateCompanionBuilder,
    $$SessionsTableUpdateCompanionBuilder> {
  $$SessionsTableTableManager(_$AppDatabase db, $SessionsTable table)
      : super(TableManagerState(
          db: db,
          table: table,
          filteringComposer:
              $$SessionsTableFilterComposer(ComposerState(db, table)),
          orderingComposer:
              $$SessionsTableOrderingComposer(ComposerState(db, table)),
          updateCompanionCallback: ({
            Value<String> sessionId = const Value.absent(),
            Value<int> athleteId = const Value.absent(),
            Value<DateTime> datetimeStart = const Value.absent(),
            Value<double> distanceMeters = const Value.absent(),
            Value<int> timeMs = const Value.absent(),
            Value<double> avgSpeedMps = const Value.absent(),
            Value<double> avgSpeedKmh = const Value.absent(),
            Value<int> bpmFinish = const Value.absent(),
            Value<int> bpmRecovery = const Value.absent(),
            Value<bool> distanceCalibrated = const Value.absent(),
            Value<double> ecv = const Value.absent(),
            Value<String?> notes = const Value.absent(),
            Value<String?> deviceRaceId = const Value.absent(),
            Value<bool> isPR = const Value.absent(),
            Value<int> rowid = const Value.absent(),
          }) =>
              SessionsCompanion(
            sessionId: sessionId,
            athleteId: athleteId,
            datetimeStart: datetimeStart,
            distanceMeters: distanceMeters,
            timeMs: timeMs,
            avgSpeedMps: avgSpeedMps,
            avgSpeedKmh: avgSpeedKmh,
            bpmFinish: bpmFinish,
            bpmRecovery: bpmRecovery,
            distanceCalibrated: distanceCalibrated,
            ecv: ecv,
            notes: notes,
            deviceRaceId: deviceRaceId,
            isPR: isPR,
            rowid: rowid,
          ),
          createCompanionCallback: ({
            required String sessionId,
            required int athleteId,
            required DateTime datetimeStart,
            required double distanceMeters,
            required int timeMs,
            required double avgSpeedMps,
            required double avgSpeedKmh,
            required int bpmFinish,
            required int bpmRecovery,
            Value<bool> distanceCalibrated = const Value.absent(),
            required double ecv,
            Value<String?> notes = const Value.absent(),
            Value<String?> deviceRaceId = const Value.absent(),
            Value<bool> isPR = const Value.absent(),
            Value<int> rowid = const Value.absent(),
          }) =>
              SessionsCompanion.insert(
            sessionId: sessionId,
            athleteId: athleteId,
            datetimeStart: datetimeStart,
            distanceMeters: distanceMeters,
            timeMs: timeMs,
            avgSpeedMps: avgSpeedMps,
            avgSpeedKmh: avgSpeedKmh,
            bpmFinish: bpmFinish,
            bpmRecovery: bpmRecovery,
            distanceCalibrated: distanceCalibrated,
            ecv: ecv,
            notes: notes,
            deviceRaceId: deviceRaceId,
            isPR: isPR,
            rowid: rowid,
          ),
        ));
}

class $$SessionsTableFilterComposer
    extends FilterComposer<_$AppDatabase, $SessionsTable> {
  $$SessionsTableFilterComposer(super.$state);
  ColumnFilters<String> get sessionId => $state.composableBuilder(
      column: $state.table.sessionId,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get athleteId => $state.composableBuilder(
      column: $state.table.athleteId,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<DateTime> get datetimeStart => $state.composableBuilder(
      column: $state.table.datetimeStart,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<double> get distanceMeters => $state.composableBuilder(
      column: $state.table.distanceMeters,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get timeMs => $state.composableBuilder(
      column: $state.table.timeMs,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<double> get avgSpeedMps => $state.composableBuilder(
      column: $state.table.avgSpeedMps,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<double> get avgSpeedKmh => $state.composableBuilder(
      column: $state.table.avgSpeedKmh,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get bpmFinish => $state.composableBuilder(
      column: $state.table.bpmFinish,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get bpmRecovery => $state.composableBuilder(
      column: $state.table.bpmRecovery,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<bool> get distanceCalibrated => $state.composableBuilder(
      column: $state.table.distanceCalibrated,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<double> get ecv => $state.composableBuilder(
      column: $state.table.ecv,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get notes => $state.composableBuilder(
      column: $state.table.notes,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get deviceRaceId => $state.composableBuilder(
      column: $state.table.deviceRaceId,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<bool> get isPR => $state.composableBuilder(
      column: $state.table.isPR,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));
}

class $$SessionsTableOrderingComposer
    extends OrderingComposer<_$AppDatabase, $SessionsTable> {
  $$SessionsTableOrderingComposer(super.$state);
  ColumnOrderings<String> get sessionId => $state.composableBuilder(
      column: $state.table.sessionId,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get athleteId => $state.composableBuilder(
      column: $state.table.athleteId,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<DateTime> get datetimeStart => $state.composableBuilder(
      column: $state.table.datetimeStart,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<double> get distanceMeters => $state.composableBuilder(
      column: $state.table.distanceMeters,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get timeMs => $state.composableBuilder(
      column: $state.table.timeMs,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<double> get avgSpeedMps => $state.composableBuilder(
      column: $state.table.avgSpeedMps,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<double> get avgSpeedKmh => $state.composableBuilder(
      column: $state.table.avgSpeedKmh,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get bpmFinish => $state.composableBuilder(
      column: $state.table.bpmFinish,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get bpmRecovery => $state.composableBuilder(
      column: $state.table.bpmRecovery,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<bool> get distanceCalibrated => $state.composableBuilder(
      column: $state.table.distanceCalibrated,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<double> get ecv => $state.composableBuilder(
      column: $state.table.ecv,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get notes => $state.composableBuilder(
      column: $state.table.notes,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get deviceRaceId => $state.composableBuilder(
      column: $state.table.deviceRaceId,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<bool> get isPR => $state.composableBuilder(
      column: $state.table.isPR,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));
}

typedef $$HrSamplesTableCreateCompanionBuilder = HrSamplesCompanion Function({
  Value<int> id,
  required String sessionId,
  required int offsetMs,
  required int bpm,
  required String phase,
});
typedef $$HrSamplesTableUpdateCompanionBuilder = HrSamplesCompanion Function({
  Value<int> id,
  Value<String> sessionId,
  Value<int> offsetMs,
  Value<int> bpm,
  Value<String> phase,
});

class $$HrSamplesTableTableManager extends RootTableManager<
    _$AppDatabase,
    $HrSamplesTable,
    HrSample,
    $$HrSamplesTableFilterComposer,
    $$HrSamplesTableOrderingComposer,
    $$HrSamplesTableCreateCompanionBuilder,
    $$HrSamplesTableUpdateCompanionBuilder> {
  $$HrSamplesTableTableManager(_$AppDatabase db, $HrSamplesTable table)
      : super(TableManagerState(
          db: db,
          table: table,
          filteringComposer:
              $$HrSamplesTableFilterComposer(ComposerState(db, table)),
          orderingComposer:
              $$HrSamplesTableOrderingComposer(ComposerState(db, table)),
          updateCompanionCallback: ({
            Value<int> id = const Value.absent(),
            Value<String> sessionId = const Value.absent(),
            Value<int> offsetMs = const Value.absent(),
            Value<int> bpm = const Value.absent(),
            Value<String> phase = const Value.absent(),
          }) =>
              HrSamplesCompanion(
            id: id,
            sessionId: sessionId,
            offsetMs: offsetMs,
            bpm: bpm,
            phase: phase,
          ),
          createCompanionCallback: ({
            Value<int> id = const Value.absent(),
            required String sessionId,
            required int offsetMs,
            required int bpm,
            required String phase,
          }) =>
              HrSamplesCompanion.insert(
            id: id,
            sessionId: sessionId,
            offsetMs: offsetMs,
            bpm: bpm,
            phase: phase,
          ),
        ));
}

class $$HrSamplesTableFilterComposer
    extends FilterComposer<_$AppDatabase, $HrSamplesTable> {
  $$HrSamplesTableFilterComposer(super.$state);
  ColumnFilters<int> get id => $state.composableBuilder(
      column: $state.table.id,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get sessionId => $state.composableBuilder(
      column: $state.table.sessionId,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get offsetMs => $state.composableBuilder(
      column: $state.table.offsetMs,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<int> get bpm => $state.composableBuilder(
      column: $state.table.bpm,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));

  ColumnFilters<String> get phase => $state.composableBuilder(
      column: $state.table.phase,
      builder: (column, joinBuilders) =>
          ColumnFilters(column, joinBuilders: joinBuilders));
}

class $$HrSamplesTableOrderingComposer
    extends OrderingComposer<_$AppDatabase, $HrSamplesTable> {
  $$HrSamplesTableOrderingComposer(super.$state);
  ColumnOrderings<int> get id => $state.composableBuilder(
      column: $state.table.id,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get sessionId => $state.composableBuilder(
      column: $state.table.sessionId,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get offsetMs => $state.composableBuilder(
      column: $state.table.offsetMs,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<int> get bpm => $state.composableBuilder(
      column: $state.table.bpm,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));

  ColumnOrderings<String> get phase => $state.composableBuilder(
      column: $state.table.phase,
      builder: (column, joinBuilders) =>
          ColumnOrderings(column, joinBuilders: joinBuilders));
}

class $AppDatabaseManager {
  final _$AppDatabase _db;
  $AppDatabaseManager(this._db);
  $$AthletesTableTableManager get athletes =>
      $$AthletesTableTableManager(_db, _db.athletes);
  $$SessionsTableTableManager get sessions =>
      $$SessionsTableTableManager(_db, _db.sessions);
  $$HrSamplesTableTableManager get hrSamples =>
      $$HrSamplesTableTableManager(_db, _db.hrSamples);
}
