import '../entities/entities.dart';

// ─────────────────────────────────────────
// Athlete repository
// ─────────────────────────────────────────
abstract class AthleteRepository {
  Future<List<Athlete>> getAll();
  Future<Athlete?> getById(int id);
  Future<void> save(Athlete athlete);
  Future<void> delete(int athleteId);
}

// ─────────────────────────────────────────
// Session repository
// ─────────────────────────────────────────
abstract class SessionRepository {
  Future<List<Session>> getAll({
    int? athleteId,
    double? distanceMeters,
    DateTime? from,
    DateTime? to,
  });
  Future<Session?> getById(String sessionId);
  Future<void> save(Session session);
  Future<void> delete(String sessionId);

  /// Devuelve el mejor tiempo (PR) del atleta para una distancia.
  Future<Session?> getPersonalRecord(int athleteId, double distanceMeters);

  /// Devuelve los N deltas de ECV de las últimas sesiones anteriores.
  Future<List<Session>> getRecentSessions(int athleteId, double distanceMeters, {int limit = 5});
}

// ─────────────────────────────────────────
// HR samples repository
// ─────────────────────────────────────────
abstract class HrSampleRepository {
  Future<List<HrSample>> getForSession(String sessionId);
  Future<void> saveAll(List<HrSample> samples);
}

// ─────────────────────────────────────────
// Connection repository
// ─────────────────────────────────────────
abstract class ConnectionRepository {
  /// Stream de estado de cada dispositivo.
  Stream<List<ConnectionStatus>> watchAll();
  Future<List<ConnectionStatus>> getAll();
}

// ─────────────────────────────────────────
// DataSource interface (Mock | Real)
// Swap this to add BLE/WebSocket/REST support
// sin tocar el resto de la app.
// ─────────────────────────────────────────
abstract class SportDataSource {
  /// Stream de paquetes del dispositivo: START, FINISH, HR
  Stream<DevicePacket> get packetStream;
  Future<void> connect();
  Future<void> disconnect();
  bool get isConnected;
}

class DevicePacket {
  final PacketType type;
  final Map<String, dynamic> payload;
  final DateTime receivedAt;

  const DevicePacket({
    required this.type,
    required this.payload,
    required this.receivedAt,
  });
}

enum PacketType { start, finish, hrUpdate, deviceStatus }
