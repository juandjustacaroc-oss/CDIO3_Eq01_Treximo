# Planeación organizada para IPAC
------
## 1. Identificación
Proyecto: Sistema de cronometraje inteligente para atletismo
Metodología: CDIO
Propósito del IPAC: Organizar y dar trazabilidad a las actividades de diseño, auditoría, validación y demostración del sistema.

## 2. Planeación por fases
### Fase 1. Planificación y auditoría

Semana 1 
Objetivo: Reorganizar el proyecto y establecer bases técnicas claras.
Actividades:
+ Reestructuración del cronograma general.
+ Planificación detallada de pruebas unitarias por subsistema.
+ Definición de criterios de validación técnica.
+ Organización del repositorio y documentación.
+ Inicio de auditoría técnica del diseño preliminar.
+ Actualización de documentos anteriores.

Resultados esperados

* Cronograma actualizado.
* Matriz preliminar de pruebas unitarias.
* Documentación actualizada.

### Fase 2. Auditoría y consolidación de diseño

Semana 2
Objetivo: Validar el diseño original y fundamentar los cambios requeridos.

Actividades

Finalización de auditoría técnica.
Construcción de la Matriz de Defensa de Diseño (MDD).
Consolidación del BOM final con proveedores y estados.
Verificación y recepción de componentes.
Gestión de pago y envío de ítems faltantes.
Definición formal del sistema de medición de distancia.
Diseño preliminar de esquemáticos electrónicos.

Resultados esperados

MDD completa.
BOM final consolidado.
Arquitectura del sistema actualizada.
Tracking number recibido.

### Fase 3. Diseño mecánico y electrónico

Semana 3
Objetivo: Adaptar el diseño físico a las decisiones técnicas finales.

Actividades

Corrección del PCB para nueva batería.
Generación y revisión de archivos Gerber.
Corrección del modelo 3D para nuevos diseños.
Diseño estructural para integración del módulo UWB.
Diseño de soportes para sensores de presencia.
Diseño de carcasa del cronómetro.
Validación de requerimientos con el cliente.
Inicio del desarrollo del sistema de medición de distancia.

Resultados esperados

Archivo Gerber generado y revisado.
Modelo 3D actualizado.
Validación documentada con el cliente.

### Fase 4. Desarrollo de software y pruebas unitarias

Semana 4
Objetivo: Garantizar el funcionamiento individual de cada subsistema antes de integrar.

Actividades de diseño

Definición de arquitectura del firmware.
Diseño preliminar de software e interfaz.

Pruebas unitarias

Validar protocolo I2C del sensor.
Validar protocolo SPI de pantalla.
Validar funcionamiento del sensor de presencia.
Validar funcionamiento del sensor de larga distancia.
Validar funcionamiento del sensor de frecuencia.
Validar funcionamiento del RTC.

Resultados esperados

Primer diseño de interfaz.
Tramas de datos visibles en monitor serial.
Código funcional validado por módulo.
Registro fotográfico y capturas de pruebas.

### Fase 5. Validación eléctrica y comunicación

Semana 5
Objetivo: Verificar estabilidad energética y comunicación inalámbrica.

Actividades

Pruebas del sistema de alimentación.
Medición de consumo bajo carga.
Pruebas de transmisión ESP-NOW.
Pruebas iniciales de medición de distancia.
Validación del botón de reinicio y señal a peones.
Depuración de firmware.

Resultados esperados

Tabla de consumo energético.
Registro de transmisión inalámbrica.
Tabla preliminar de error en medición de distancia.

### Fase 6. Integración parcial

Semana 6
Objetivo: Integrar subsistemas y validar funcionamiento conjunto.

Pruebas

Validar funcionamiento del cronómetro (cerebro).
Validar funcionamiento de peones.
Validar transición correcta entre estados del sistema.
Validar cronometraje consistente en múltiples pruebas.
Validar funcionamiento del sistema conjunto.

Resultados esperados

Comportamiento visible en pantalla TFT.
Medición de velocidad en monitor serial.
Registro comparativo de tiempos.
Evidencia audiovisual interna.

### Fase 7. Interfaz web y transmisión de datos

Semana 7
Objetivo: Confirmar coherencia entre visualización local y remota.

Actividades

Validar transmisión de datos a la página web.
Verificar correspondencia entre pantalla TFT y navegador.
Optimizar el uso de memoria.
Realizar pruebas de actualización en tiempo real.

Resultados esperados

Correspondencia de datos en pantalla y página web.
Capturas del navegador.
Validación de sincronización de datos.

### Fase 8. Montaje total y correcciones

Semana 8
Objetivo: Ensamblar completamente el sistema y ejecutar ajustes finales.

Actividades

Montaje total con diseños mecánicos definitivos.
Ajuste de alineación de sensores.
Pruebas reales en campo.
Correcciones finales del diseño.

Resultados esperados

Sistema completamente ensamblado.
Registro de pruebas en entorno real.
Correcciones implementadas.

### Fase 9. Demostración final

Semana 9
Objetivo: Ejecutar la validación integral del sistema en condiciones reales.

Actividades

Prueba completa de carrera.
Validación final de medición de distancia.
Validación de tiempo, velocidad y frecuencia cardíaca.
Consolidación de documentación final.

Resultados esperados

Demostración funcional en entorno real.
Dossier técnico consolidado.
Evidencias finales organizadas.


| Tipo            | Actividad                                                   | Evidencia esperada                          | Semana |
| --------------- | ----------------------------------------------------------- | ------------------------------------------- | ------ |
| Revisión        | Actualizar información de documentos anteriores             | Documentación actualizada                   | 1      |
| Compras         | Gestionar pago y envío de ítems pendientes                  | Tracking number recibido                    | 2      |
| Diseño PCB      | Corregir PCB para nueva batería                             | Archivo Gerber generado y revisado          | 3      |
| Diseño mecánico | Corregir modelo 3D para nuevos diseños                      | Modelo 3D actualizado                       | 3      |
| Diseño software | Diseñar software e interfaz                                 | Primer diseño de interfaz                   | 4      |
| Prueba          | Validar protocolo I2C del sensor                            | Trama visible en monitor serial             | 4      |
| Prueba          | Validar protocolo SPI de pantalla                           | Trama visible en monitor serial             | 4      |
| Prueba          | Validar funcionamiento del sensor de presencia              | Trama visible en monitor serial             | 4      |
| Prueba          | Validar funcionamiento del sensor de larga distancia        | Trama visible en monitor serial             | 4      |
| Prueba          | Validar funcionamiento del sensor de frecuencia             | Trama visible en monitor serial             | 4      |
| Prueba          | Validar funcionamiento del RTC                              | Trama visible en monitor serial             | 4      |
| Prueba          | Validar funcionamiento del cronómetro (cerebro)             | Comportamiento visible en pantalla TFT      | 6      |
| Prueba          | Validar funcionamiento de peones                            | Medición de velocidad en monitor serial     | 6      |
| Prueba          | Validar funcionamiento del sistema conjunto                 | Comportamiento visible en pantalla TFT      | 6      |
| Prueba          | Validar funcionamiento de transmisión de datos a página web | Correspondencia entre pantalla y página web | 7      |
| Prueba          | Montaje total con diseños mecánicos                         | Sistema ensamblado                          | 8      |
| Prueba          | Correcciones del diseño                                     | Sistema ajustado                            | 8      |
| Final           | Demostración en funcionamiento real                         | Validación integral del sistema             | 9      |

