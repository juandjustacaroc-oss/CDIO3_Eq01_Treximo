# 03_Firmware

## Firmware del Sistema

Esta carpeta contiene todo el código fuente necesario para la operación del sistema distribuido.

El firmware está organizado por subsistemas:

- Cronómetro Central
- Peón de Salida
- Peón de Llegada
- Banda Fisiológica

---

## Estructura

### 📂 lib/
Librerías externas utilizadas por el proyecto:
- ESP-NOW
- MAX3010x
- Pantalla TFT / OLED
- RTC
- UWB (si aplica)

No modificar directamente estas librerías.
Las versiones deben documentarse en el informe técnico.

---

### 📂 src/
Código fuente del sistema:
- Archivos `.ino`
- Archivos `.c` / `.cpp`
- Scripts auxiliares `.py` (si aplica)

---

## Compilación

### Requisitos
- Arduino IDE 2.x o PlatformIO
- ESP32 Board Package actualizado
- Librerías listadas en `/lib`

### Pasos
1. Seleccionar placa ESP32 correspondiente.
2. Verificar puerto.
3. Compilar.
4. Cargar firmware.
5. Abrir Monitor Serie a 115200 baudios.

---

## Arquitectura del Software

El firmware del Cronómetro Central implementa:

- Máquina de estados finita (FSM):
  - ESPERANDO_SALIDA
  - EN_CARRERA
  - RECUPERACION
  - FINALIZADO

- Recepción de eventos vía ESP-NOW.
- Procesamiento de frecuencia cardíaca.
- Servidor web ligero para monitoreo.
- Control por botón físico de reinicio.

---

## Nota Técnica

El sistema prioriza:
- Baja latencia
- Uso eficiente de memoria
- Estabilidad en condiciones reales de campo
