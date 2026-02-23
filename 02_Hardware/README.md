# 02_Hardware

## Diseño y Arquitectura Física del Sistema

Esta carpeta contiene todos los recursos asociados al desarrollo físico y electrónico del sistema distribuido basado en ESP32.

El hardware está dividido en tres subsistemas principales:

- 🧠 Cronómetro Central
- 📡 Peones (Salida y Llegada)
- ❤️ Banda de Variables Fisiológicas

---

## Estructura de Carpetas

### 📂 Datasheets/
Contiene únicamente hojas técnicas oficiales (PDF) de los componentes utilizados:
- ESP32
- UWB (DW1000 / RYUW122)
- MAX3010x
- RTC
- Pantalla TFT
- Módulos de carga
- Sensores ultrasónicos

Estos documentos fundamentan las decisiones técnicas adoptadas en el diseño.

---

### 📂 Mechanical/
Archivos mecánicos del proyecto:
- STL (impresión 3D)
- DXF (corte láser)
- CAD (modelos completos)

Incluye:
- Carcasa del cronómetro
- Soportes de peones
- Integración de banda fisiológica

---

### 📂 Source/
Archivos editables de diseño electrónico:
- KiCad
- Altium
- Eagle

Contiene:
- Esquemáticos
- PCB
- Layout
- Gerbers (si aplica)

---

## Consideraciones Técnicas

- Alimentación autónoma con sistema de carga y protección.
- Comunicación inalámbrica UWB para medición de distancia.
- ESP-NOW para eventos críticos de cronometraje.
- Diseño modular para mantenimiento y escalabilidad.

---

## Estado

El hardware ha sido auditado y actualizado tras la revisión técnica del diseño inicial.
Las modificaciones están justificadas en la Matriz de Defensa de Diseño (ver carpeta 01_Gestion).
