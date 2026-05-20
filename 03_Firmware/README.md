# 03_Firmware — Firmware ESP32 TRÉXIMO

Esta carpeta contiene el firmware de los nodos ESP32 que conforman el sistema **TRÉXIMO**.

El firmware implementa:
- detección de eventos,
- cronometraje,
- comunicación inalámbrica,
- cálculo de variables deportivas,
- integración con la aplicación web,
- y gestión de estados del sistema.

---

# Nodos del sistema

| Nodo | Función | Archivo esperado |
|---|---|---|
| Cerebro / Cronómetro | Recibe eventos, calcula tiempo y velocidad, y envía datos a la aplicación | `CerebroFinal_*.ino` |
| Peón salida | Detecta salida mediante HC-SR04 y envía evento `START` | `Peon_Salida_*.ino` |
| Peón llegada | Detecta llegada mediante HC-SR04 y envía evento `FINISH` | `Peon_Llegada_*.ino` |
| Simulador ESP32 | Genera carreras aleatorias para pruebas de backend/app | `ESP32_Treximo_Simulador_*.ino` |
| Banda HR | Evaluada durante desarrollo; retirada del PMV | SIN ARCHIVO |

---

# Arquitectura de firmware

```text
Peón salida ── MSG_START ┐
                         ├── Cerebro FSM ── WiFi POST ── App web
Peón llegada ─ MSG_FINISH┘

UWB TAG/ANCHOR ─ distancia
```

---

# Máquina de estados del cerebro

| Estado | Descripción | Evento de salida |
|---|---|---|
| `IDLE` | Espera carrera o calibración | START manual o `MSG_START` |
| `CALIBRATING` | Solicita y espera distancia UWB | `MSG_DISTANCE` o timeout |
| `RUN` | Cronómetro en carrera | STOP manual o `MSG_FINISH` |
| `RECOVERY` | Tiempo de recuperación entre carreras | Timeout de 60 s |
| `UPLOAD` | Envía datos al backend local | HTTP OK/Error |
| `ERROR` | Estado de fallo controlado | Reset manual |

---

# Librerías necesarias

## Librerías incluidas con Arduino / ESP32

```cpp
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <Preferences.h>
#include <SPI.h>
#include <Wire.h>
#include <time.h>
```

---

## Librerías externas

Instalar desde Library Manager o mediante ZIP, también se encuentran alojadas en el repositorio.

| Librería | Uso |
|---|---|
| `Adafruit_GFX` | Gráficos TFT |
| `Adafruit_ST7735` | Pantalla TFT ST7735 |
| `RTClib` | RTC DS3231 |
| `DW1000Ranging` | Comunicación UWB |
| `ArduinoJson` | Manejo de estructuras JSON |
| `MAX30105` | Histórico del sensor cardíaco |

---

# Configuración de placa

- Placa: `ESP32 Dev Module`
- Velocidad Serial: `115200`
- WiFi: `2.4 GHz`
- Canal ESP-NOW: igual para todos los nodos
- Flash: configuración estándar Arduino IDE

---

# Pines principales

## Cerebro

| Elemento | Pin |
|---|---|
| TFT SCK | GPIO18 |
| TFT MOSI | GPIO23 |
| TFT CS | GPIO5 |
| TFT DC | GPIO2 |
| TFT RST | GPIO4 |
| RTC SDA | GPIO21 |
| RTC SCL | GPIO22 |
| START | GPIO12 |
| STOP | GPIO13 |
| RESET | GPIO14 |

---

## Peones

| Elemento | Pin |
|---|---|
| HC-SR04 TRIG | GPIO15 |
| HC-SR04 ECHO | GPIO2 |
| LED | GPIO16 |
| UWB SCK | GPIO18 |
| UWB MISO | GPIO19 |
| UWB MOSI | GPIO23 |
| UWB CS | GPIO4 |
| UWB RST | GPIO27 |
| UWB IRQ | GPIO34 |

---

# Variables de configuración

```cpp
const char* WIFI_SSID = "TU_WIFI";
const char* WIFI_PASS = "TU_CLAVE";

const char* API_URL =
"http://IP_DEL_PC:8081/api/ingest-workout";

const char* DEVICE_TOKEN =
"treximo-secret-123";
```

---

# Reglas de desarrollo

- Evitar `delay()` en tareas críticas.
- Utilizar temporizadores o interrupciones cuando sea necesario.
- Mantener callbacks ESP-NOW ligeros.
- Guardar configuraciones persistentes mediante `Preferences`.
- Reportar errores por Serial y/o pantalla TFT.
- No subir credenciales reales a repositorios públicos.

---

# Flujo mínimo de prueba

1. Cargar firmware en peón de salida.
2. Cargar firmware en peón de llegada.
3. Cargar firmware en cerebro central.
4. Verificar recepción de `START` y `FINISH`.
5. Ejecutar calibración de distancia.
6. Ejecutar carrera de prueba.
7. Verificar envío HTTP al backend.
8. Confirmar visualización en la aplicación.

---

# Estado actual del firmware

| Subsistema | Estado | Observaciones |
|---|---|---|
| Comunicación ESP-NOW | Validado | Comunicación estable entre nodos |
| HC-SR04 | Parcial | Requiere calibración en exteriores |
| UWB | Parcial | Error variable en medición |
| RTC | Validado | Registro temporal estable |
| Interfaz TFT | Validado | Visualización funcional |
| Frecuencia cardíaca | Retirada del PMV | Lectura inestable y sensor dañado |

---

# Objetivo del firmware

El firmware de TRÉXIMO busca garantizar:
- sincronización entre nodos,
- medición precisa de eventos deportivos,
- portabilidad,
- integración con sistemas digitales,
- y trazabilidad del rendimiento deportivo en tiempo real.

---
