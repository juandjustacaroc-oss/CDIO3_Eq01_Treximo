# CDIO3_Eq01_Treximo: Smart Athletic Timing & Physiological Monitoring System
Sistema distribuido basado en ESP32 para cronometraje automático y monitoreo fisiológico en pruebas de atletismo (100 m), diseñado para proporcionar mediciones precisas de tiempo, velocidad y frecuencia cardíaca en tiempo real. Este proyecto integra comunicación inalámbrica de baja latencia, procesamiento embebido y una interfaz web ligera para ofrecer retroalimentación inmediata al entrenador sin depender de infraestructura externa o servicios en la nube.

![Status](https://img.shields.io/badge/Status-Prototipo-yellow)

# Objetivo del Proyecto
Desarrollar un sistema portátil y autónomo capaz de:
- Detectar automáticamente el inicio y finalización de la carrera.
- Calcular el tiempo exacto del recorrido.
- Estimar la velocidad promedio del atleta.
- Monitorear la frecuencia cardíaca durante la carrera y la fase de recuperación.
- Visualizar métricas en pantalla OLED y en una interfaz web básica embebida.
- Reiniciar el sistema de forma segura para múltiples sesiones consecutivas.

# Arquitectura General
- Peon Salida/Peon Llegada.
- Cerebro Cronometro.
- Banda de variables fisiologicas.
La comunicación entre nodos se realiza mediante ESP-NOW, garantizando baja latencia y operación sin router.
El nodo central también opera como punto de acceso WiFi para permitir la visualización de datos desde un smartphone o tablet.

# Estado Actual
- ❌Hardware funcional
- ✅Comunicación estable entre nodos
- ❌Cronometría precisa basada en eventos
- ❌Monitoreo cardíaco integrado
- ✅Interfaz TFT operativa
- ❌Dashboard web ligero en pruebas

# Librerías necesarias

