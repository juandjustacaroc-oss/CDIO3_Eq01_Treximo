# Planeación organizada para IPAC
------
## 1. Identificación
Proyecto: Sistema de cronometraje inteligente para atletismo
Metodología: CDIO
Propósito del IPAC: Organizar y dar trazabilidad a las actividades de diseño, auditoría, validación y demostración del sistema.

## 2. Planeación por fases
### Fase 1. Planificación y auditoría

Semana 1 
Actividades:
+ Reestructuración del cronograma general.
+ Planificación detallada de pruebas unitarias por subsistema.
+ Definición de criterios de validación técnica.

Resultados esperados
* Cronograma actualizado.
* Matriz preliminar de pruebas unitarias.
* Documentación actualizada.

### Fase 2. Auditoría y consolidación de diseño
Actividades
+ Consolidación del BOM final con proveedores y estados

Resultados esperados
+ BOM final consolidado.


### Fase 3. Diseño mecánico y electrónico
Actividades
+ Diseño preliminar de esquemáticos electrónicos.
+ Diseño preliminar de modelos 3D
  
Resultados esperados
+ Modelo 3D actualizado.
+ Archivos .STL .SCH .PCB

### Fase 4. Desarrollo de software y pruebas unitarias
Actividades de diseño

+ Definición de arquitectura del firmware.

Pruebas unitarias
+ Validar protocolo I2C del sensor.
+ Validar protocolo SPI de pantalla.
+ Validar funcionamiento del sensor de presencia.
+ Validar funcionamiento del sensor de larga distancia.
+ Validar funcionamiento del sensor de frecuencia.
+ Validar funcionamiento del RTC.

Resultados esperados
+ Tramas de datos visibles en monitor serial.
+ Código funcional validado por módulo.
+ Registro fotográfico y capturas de pruebas.

### Fase 5. Validación eléctrica y comunicación
Actividades
+ Diseño preliminar de software e interfaz.
+ Pruebas del sistema de alimentación.
+ Medición de consumo bajo carga.
+ Pruebas de transmisión ESP-NOW.
+ Pruebas iniciales de medición de distancia.
+ Validación del botón de reinicio y señal a peones.

Resultados esperados
+ Primer diseño de interfaz.
+ Tabla de consumo energético.
+ Registro de transmisión inalámbrica.
+ Tabla preliminar de error en medición de distancia.

### Fase 6. Integración parcial
Semana 6

Pruebas
+ Validar funcionamiento del cronómetro (cerebro).
+ Validar funcionamiento de peones.
+ Validar transición correcta entre estados del sistema.
+ Validar cronometraje consistente en múltiples pruebas.
+ Validar funcionamiento del sistema conjunto.

Resultados esperados
+ Comportamiento visible en pantalla TFT.
+ Medición de velocidad en monitor serial.
+ Registro comparativo de tiempos.
+ Evidencia audiovisual interna.

### Fase 7. Interfaz web y transmisión de datos
Actividades
+ Validar transmisión de datos a la página web.
+ Verificar correspondencia entre pantalla TFT y navegador.
+ Optimizar el uso de memoria.
+ Realizar pruebas de actualización en tiempo real.

Resultados esperados
+ Correspondencia de datos en pantalla y página web.
+ Capturas del navegador.
+ Validación de sincronización de datos.

### Fase 8. Montaje total y correcciones
Actividades
+ Montaje total con diseños mecánicos definitivos.
+ Ajuste de alineación de sensores.
+ Pruebas reales en campo.
+ Correcciones finales del diseño.

Resultados esperados
+ Sistema completamente ensamblado.
+ Registro de pruebas en entorno real.
+ Correcciones implementadas.

### Fase 9. Demostración final
Actividades
+ Prueba completa de carrera.
+ Validación final de medición de distancia.
+ Validación de tiempo, velocidad y frecuencia cardíaca.
+ Consolidación de documentación final.

Resultados esperados
+ Demostración funcional en entorno real.
+ Dossier técnico consolidado.
+ Evidencias finales organizadas.


