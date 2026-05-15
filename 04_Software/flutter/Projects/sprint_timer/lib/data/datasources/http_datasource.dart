import 'dart:async';
import 'dart:convert';
import 'dart:io';
import '../../domain/entities/entities.dart';
import '../../domain/repositories/repositories.dart';

class HttpSportDataSource implements SportDataSource {
  final int port;
  HttpServer? _server;
  final StreamController<DevicePacket> _controller = StreamController<DevicePacket>.broadcast();
  bool _isConnected = false;

  HttpSportDataSource({this.port = 8080});

  @override
  Stream<DevicePacket> get packetStream => _controller.stream;

  @override
  bool get isConnected => _isConnected;

  @override
  Future<void> connect() async {
    if (_isConnected) return;
    try {
      _server = await HttpServer.bind(InternetAddress.anyIPv4, port);
      _isConnected = true;
      _controller.add(DevicePacket(
        type: PacketType.deviceStatus,
        payload: const {'status': 'connected', 'device': 'treximo-http'},
        receivedAt: DateTime.now(),
      ));

      _server!.listen((HttpRequest request) async {
        if (request.uri.path == '/api/session' && request.method == 'POST') {
          try {
            final content = await utf8.decoder.bind(request).join();
            final Map<String, dynamic> data = jsonDecode(content);

            // Ejemplo del JSON esperado:
            // { "device_id": "treximo-001", "race_id": 1, "athlete_code": 1, ... }
            
            // Enviamos el packet de START (simulado retroactivamente)
            // Esto es para que la UI sepa que hay una carrera (aunque ya terminó, 
            // la UI mostrará el resultado inmediatamente)
            // Lo ideal es que el ESP32 envíe el START cuando arranca, y el FINISH al final.
            // Si el ESP32 envía todo junto al final, emitimos un FINISH directo.
            
            _controller.add(DevicePacket(
              type: PacketType.finish,
              payload: data,
              receivedAt: DateTime.now(),
            ));

            request.response
              ..statusCode = HttpStatus.ok
              ..write('{"status": "ok"}')
              ..close();
          } catch (e) {
            request.response
              ..statusCode = HttpStatus.badRequest
              ..write('{"error": "Invalid JSON"}')
              ..close();
          }
        } else {
          request.response
            ..statusCode = HttpStatus.notFound
            ..write('{"error": "Not found"}')
            ..close();
        }
      });
    } catch (e) {
      _isConnected = false;
      throw Exception('Error starting HTTP server: $e');
    }
  }

  @override
  Future<void> disconnect() async {
    if (!_isConnected) return;
    await _server?.close(force: true);
    _isConnected = false;
    _controller.add(DevicePacket(
      type: PacketType.deviceStatus,
      payload: const {'status': 'disconnected'},
      receivedAt: DateTime.now(),
    ));
  }
}
