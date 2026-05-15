import 'package:equatable/equatable.dart';

// ─────────────────────────────────────────
// Athlete entity
// ─────────────────────────────────────────
class Athlete extends Equatable {
  final int athleteId;
  final String name;
  final String? notes;

  const Athlete({
    required this.athleteId,
    required this.name,
    this.notes,
  });

  @override
  List<Object?> get props => [athleteId, name, notes];
}

// ─────────────────────────────────────────
// Session entity
// ─────────────────────────────────────────
class Session extends Equatable {
  final String sessionId;
  final int athleteId;
  final DateTime datetimeStart;
  final int distanceMeters; // 50 o 100

  final int timeMs;
  final double avgSpeedMps;
  final double avgSpeedKmh;

  final double hrAvgRun;
  final double hrMaxRun;
  final double hrAvgRecovery;
  final double hrMaxRecovery;

  /// ECV = avgSpeedMps / hrAvgRun  (configurable en Settings)
  final double ecv;

  final String? notes;

  /// ID de carrera en el dispositivo (para vincular con RTC)
  final String? deviceRaceId;

  /// Indica si es récord personal para el atleta en esa distancia
  final bool isPR;

  const Session({
    required this.sessionId,
    required this.athleteId,
    required this.datetimeStart,
    required this.distanceMeters,
    required this.timeMs,
    required this.avgSpeedMps,
    required this.avgSpeedKmh,
    required this.hrAvgRun,
    required this.hrMaxRun,
    required this.hrAvgRecovery,
    required this.hrMaxRecovery,
    required this.ecv,
    this.notes,
    this.deviceRaceId,
    this.isPR = false,
  });

  /// Muestra el tiempo como MM:SS.d
  String get timeFormatted {
    final min = timeMs ~/ 60000;
    final sec = (timeMs % 60000) ~/ 1000;
    final tenth = (timeMs % 1000) ~/ 100;
    if (min > 0) return '${min.toString().padLeft(2, '0')}:${sec.toString().padLeft(2, '0')}.$tenth';
    return '${sec.toString().padLeft(2, '0')}.$tenth';
  }

  @override
  List<Object?> get props => [sessionId];
}

// ─────────────────────────────────────────
// HR Sample (para gráficas de fase)
// ─────────────────────────────────────────
class HrSample extends Equatable {
  final String sessionId;
  final int offsetMs;   // ms desde inicio de carrera
  final int bpm;
  final HrPhase phase;

  const HrSample({
    required this.sessionId,
    required this.offsetMs,
    required this.bpm,
    required this.phase,
  });

  @override
  List<Object?> get props => [sessionId, offsetMs];
}

enum HrPhase { run, recovery }

// ─────────────────────────────────────────
// Connection status
// ─────────────────────────────────────────
class ConnectionStatus extends Equatable {
  final String deviceName;
  final bool isConnected;
  final DateTime? lastSeen;
  final int? rssi;
  final String? firmwareVersion;

  const ConnectionStatus({
    required this.deviceName,
    required this.isConnected,
    this.lastSeen,
    this.rssi,
    this.firmwareVersion,
  });

  @override
  List<Object?> get props => [deviceName, isConnected, lastSeen, rssi];
}

// ─────────────────────────────────────────
// Session state (para pantalla Nueva sesión)
// ─────────────────────────────────────────
enum SessionState { idle, ready, inProgress, finished, recovery }
