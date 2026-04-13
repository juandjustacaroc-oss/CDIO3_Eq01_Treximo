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

| Ítem | Propósito                                                  | Actividad                                                         | Criterio de aceptación                                                                       | Evidencia                                   |
| :--- | :--------------------------------------------------------- | :---------------------------------------------------------------- | :------------------------------------------------------------------------------------------- | :------------------------------------------ |
| 1    | Reorganizar el proyecto y establecer bases técnicas claras | Reestructurar el cronograma general del proyecto                  | El cronograma queda actualizado, coherente y alineado con las fases del proyecto             | Cronograma actualizado                      |
| 2    | Planificar la validación técnica del sistema               | Definir pruebas unitarias por subsistema                          | Cada subsistema cuenta con al menos una prueba definida y un criterio de validación          | Matriz preliminar de pruebas unitarias      |
| 3    | Asegurar trazabilidad documental                           | Organizar el repositorio y la documentación                       | La estructura documental del proyecto queda actualizada y ordenada                           | Documentación organizada en repositorio     |
| 4    | Fundamentar cambios sobre el diseño preliminar             | Realizar auditoría técnica del diseño inicial                     | Se identifican hallazgos, problemas y decisiones de rediseño justificadas                    | Informe de auditoría técnica                |
| 5    | Consolidar la defensa técnica del proyecto                 | Construir la Matriz de Defensa de Diseño (MDD)                    | Cada cambio importante del diseño queda justificado con su respectiva evidencia              | MDD finalizada                              |
| 6    | Consolidar materiales y compras del sistema                | Actualizar el BOM final con componentes, proveedores y estados    | Todos los componentes relevantes se encuentran registrados en el BOM                         | BOM final consolidado                       |
| 7    | Garantizar disponibilidad de hardware                      | Gestionar pago y envío de los componentes faltantes               | Se obtiene confirmación de compra y número de seguimiento                                    | Tracking number recibido                    |
| 8    | Adaptar el diseño electrónico a los nuevos requerimientos  | Corregir el PCB para la nueva batería                             | El diseño actualizado permite integrar correctamente la nueva alimentación                   | Archivo Gerber generado y revisado          |
| 9    | Ajustar el diseño físico al rediseño técnico               | Corregir el modelo 3D para los nuevos diseños                     | El modelo mecánico refleja los cambios estructurales del sistema                             | Modelo 3D actualizado                       |
| 10   | Integrar medición de distancia al sistema                  | Diseñar la incorporación del módulo UWB o alternativa equivalente | El diseño contempla montaje y conexión del subsistema de distancia                           | Diseño actualizado del sistema              |
| 11   | Validar el diseño con el cliente                           | Revisar requerimientos y cambios con el cliente                   | El cliente aprueba o retroalimenta formalmente el diseño actualizado                         | Registro o acta de validación con cliente   |
| 12   | Definir la arquitectura lógica del sistema                 | Diseñar el software y la interfaz                                 | Existe una primera propuesta funcional de firmware e interfaz                                | Primer diseño de interfaz                   |
| 13   | Validar comunicación I2C                                   | Probar protocolo I2C del sensor correspondiente                   | La trama de datos es visible y coherente en monitor serial                                   | Captura de monitor serial                   |
| 14   | Validar comunicación SPI                                   | Probar protocolo SPI de pantalla                                  | La pantalla responde correctamente a la inicialización y muestra datos                       | Captura de monitor serial o fotografía      |
| 15   | Validar el sensor de presencia                             | Comprobar funcionamiento del sensor de presencia                  | El sensor detecta correctamente el evento esperado                                           | Trama visible en monitor serial             |
| 16   | Validar el sensor de larga distancia                       | Comprobar funcionamiento del sistema de medición de distancia     | El sensor entrega valores consistentes y repetibles                                          | Trama visible en monitor serial             |
| 17   | Validar el sensor fisiológico                              | Comprobar funcionamiento del sensor de frecuencia cardíaca        | El sistema detecta y muestra valores fisiológicos coherentes                                 | Trama visible en monitor serial             |
| 18   | Validar trazabilidad temporal                              | Comprobar funcionamiento del RTC                                  | La fecha y hora son leídas correctamente                                                     | Trama visible en monitor serial             |
| 19   | Verificar estabilidad energética del sistema               | Probar el sistema de alimentación                                 | El sistema opera sin reinicios inesperados ni caídas de voltaje                              | Tabla de consumo energético                 |
| 20   | Evaluar comportamiento de comunicación inalámbrica         | Probar transmisión ESP-NOW entre nodos                            | La transmisión se realiza sin pérdidas significativas de eventos                             | Registro de transmisión inalámbrica         |
| 21   | Validar interacción de control                             | Probar botón de reinicio y señal de reinicio a peones             | El botón reinicia el sistema y los peones reciben la señal correctamente                     | Captura serial y evidencia visual           |
| 22   | Mejorar estabilidad del firmware                           | Depurar el código de los subsistemas                              | El sistema ejecuta sus funciones sin errores críticos                                        | Código corregido y pruebas repetidas        |
| 23   | Validar funcionamiento del cronómetro central              | Probar el cerebro de forma individual                             | La pantalla TFT muestra el estado correcto y responde a eventos                              | Comportamiento visible en la pantalla TFT   |
| 24   | Validar funcionamiento de los peones                       | Probar peón de salida y peón de llegada                           | Los peones detectan paso y transmiten correctamente                                          | Medición visible en monitor serial          |
| 25   | Validar integración parcial del sistema                    | Probar el sistema conjunto en condiciones controladas             | Los estados del sistema cambian correctamente de espera a carrera, recuperación y finalizado | Comportamiento visible en pantalla TFT      |
| 26   | Verificar exactitud del cronometraje                       | Comparar tiempos medidos por el sistema en varias pruebas         | Los tiempos son coherentes y repetibles entre pruebas                                        | Registro comparativo de tiempos             |
| 27   | Validar visualización remota                               | Probar transmisión de datos a la página web                       | La información mostrada en la web coincide con los datos del sistema                         | Correspondencia entre pantalla y página web |
| 28   | Optimizar recursos del sistema embebido                    | Ajustar la interfaz web para uso eficiente de memoria             | El sistema opera con estabilidad manteniendo la interfaz funcional                           | Funcionamiento estable del ESP32            |
| 29   | Integrar físicamente todos los subsistemas                 | Realizar el montaje total con los diseños mecánicos definitivos   | El sistema queda ensamblado y operativo físicamente                                          | Sistema completamente ensamblado            |
| 30   | Corregir desajustes detectados en integración              | Aplicar correcciones finales de diseño                            | Los errores detectados en pruebas previas quedan corregidos                                  | Registro de correcciones implementadas      |
| 31   | Validar el sistema en entorno real                         | Realizar pruebas de campo                                         | El sistema funciona fuera del entorno de laboratorio                                         | Registro de pruebas en campo                |
| 32   | Ejecutar validación final del proyecto                     | Realizar demostración completa en funcionamiento real             | El sistema cumple los requerimientos principales en una prueba integral                      | Demostración funcional documentada          |
| 33   | Consolidar el cierre técnico del proyecto                  | Organizar documentación final y evidencias                        | Se reúne toda la información final del proyecto en un dossier técnico                        | Dossier técnico consolidado                 |


