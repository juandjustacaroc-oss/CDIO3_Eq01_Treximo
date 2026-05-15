# Sprint Timer - Cronómetro Inteligente de Atletismo

Esta es una aplicación móvil desarrollada en Flutter diseñada para acompañar un sistema "Cronómetro Inteligente" con ESP32. Provee funcionalidades para registrar sesiones, ver el rendimiento de atletas, y evaluar la Eficiencia Cardiovascular (ECV).

## Requisitos
- Flutter SDK (>=3.2.0 <4.0.0)
- IDE recomendado: Android Studio o VS Code con plugins de Flutter y Dart.

## Instrucciones para Correr (Ejecución Local)

1. **Instalar dependencias**:
   ```bash
   flutter pub get
   ```

2. **(Opcional) Generar código de Drift (Base de Datos)**:
   Si modificas las entidades de base de datos en `lib/data/database/app_database.dart`, debes regenerar el código:
   ```bash
   dart run build_runner build --delete-conflicting-outputs
   ```

3. **Ejecutar en tu dispositivo o emulador**:
   ```bash
   flutter run
   ```

## Arquitectura y Estado Actual

La aplicación implementa una estructura Clean Architecture usando **BLoC/Riverpod** (actualmente BLoC) para manejo de estado, y **Drift** (SQLite) para almacenamiento local. 
Actualmente, la app se encuentra conectada a un `MockSportDataSource` que inyecta datos ficticios simulando atletas, carreras, y latidos (HR) en tiempo real, permitiendo probar la interfaz gráfica completa y el almacenamiento sin requerir hardware físico conectado.

## ¿Cómo reemplazar el MockDataSource por un RealDataSource?

Cuando el hardware (ESP32) esté listo para enviar datos (ej. mediante WebSocket, ESP-NOW vía gateway, o Bluetooth BLE), debes seguir estos pasos:

1. **Crea la nueva implementación**:
   Dirígete a `lib/data/datasources/` y crea una nueva clase (ej. `BleSportDataSource` o `WebSocketSportDataSource`) que implemente la interfaz `SportDataSource` (`lib/domain/repositories/repositories.dart`).

   ```dart
   class WebSocketSportDataSource implements SportDataSource {
     // Implementa packetStream, isConnected, connect() y disconnect()
     // traduciendo los bytes/JSON del ESP32 a DevicePacket.
   }
   ```

2. **Inyectar el nuevo DataSource en `main.dart`**:
   Cambia la instanciación en la función principal:
   ```dart
   // ANTES:
   // final dataSource = MockSportDataSource();
   
   // DESPUÉS:
   final dataSource = WebSocketSportDataSource('ws://ip-del-esp32');
   ```

3. **Listo**: 
   Como la UI (pantalla de Nueva Sesión) y los Blocs (`LiveSessionBloc`) dependen únicamente de la interfaz `SportDataSource`, **no tendrás que modificar ni una sola línea de la interfaz gráfica**. Toda la lógica de "Estado de Carrera" y almacenamiento de muestras procesará transparentemente los datos reales.

## Documentación UX/UI
Se adjuntan los documentos:
- [UX_UI_SPEC.md](UX_UI_SPEC.md): Mapa de navegación, pantallas y manejo de errores.
- [UI_KIT.md](UI_KIT.md): Paleta de colores Dark Mode, tipografía, espaciado y componentes.
