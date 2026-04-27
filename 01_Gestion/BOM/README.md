# Bill Of Materials (BOM)
Este documento detalla todos los componentes electrónicos, electromecánicos y materiales de manufactura necesarios para un sistema básico para construir un sistema funcional.

# Cronometro
| Ítem | Componente             | Cant. | Referencia / Especificación                 | Función                                                                     | Est. Costo (COP) |
| :--- | :--------------------- | :---: | :------------------------------------------ | :-------------------------------------------------------------------------- | :--------------: |
| 1    | **Microcontrolador**   |   1   | **ESP32 DevKit V1 / ESP32-WROOM-32 DevKit** | Procesamiento principal del cronómetro, control de estados y comunicaciones |     $ 31.178     |
| 2    | **Pantalla TFT**       |   1   | **TFT SPI ST7735 1.77” / 160x128**          | Visualización local de tiempo, estado y frecuencia cardíaca                 |     $ 34.600     |
| 3    | **Batería recargable** |   1   | **LiPo / Li-ion 3.7 V 2200 mAh**            | Alimentación portátil del cronómetro                                        |     $ 20.000     |
| 4    | **Módulo de carga**    |   1   | **TP4056 con protección**                   | Carga segura de la batería del cronómetro                                   |      $ 5.500     |
| 5    | **Reloj de Tiempo Real**|   1   | **Caja impresa en PLA**                     | Protección mecánica e integración física del cronómetro         |               $ 20.000     |
| 6    | **Indicador visual**   |   1   | **LED + resistencia**                       | Confirmación visual de estados y eventos                                    |      $ 1.000     |
| 7    | **Carcasa**            |   1   | **Caja impresa en PLA**                     | Protección mecánica e integración física del cronómetro                     |     $ 20.000     |
# Peon
| Ítem | Componente              | Cant. | Referencia / Especificación                 | Función                                   | Est. Costo (COP) |
| :--- | :---------------------- | :---: | :------------------------------------------ | :---------------------------------------- | :--------------: |
| 12   | **Microcontrolador**    |   1   | **ESP32 DevKit V1 / ESP32-WROOM-32 DevKit** | Control del peón y transmisión de eventos |     $ 31.178     |
| 13   | **Sensor de presencia** |   1   | **HC-SR04**                                 | Detección del paso del atleta             |      $ 6.900     |
| 14   | **Módulo UWB**          |   1   | **BU01**                                    | Medición de distancia entre peones        |     $ 84.410     |
| 15   | **Batería recargable**  |   1   | **LiPo / Li-ion 3.7 V 2200 mAh**            | Alimentación portátil del peón            |     $ 20.000     |
| 16   | **Módulo de carga**     |   1   | **TP4056 con protección**                   | Carga segura de la batería del peón       |      $ 5.500     |
| 17   | **Carcasa**             |   1   | **Caja impresa en PLA**                     | Protección e integración física del peón  |     $ 20.000     |
| 18   | **Indicador visual**    |   1   | **LED + resistencia**                       | Confirmación visual de detección y estado |      $ 1.000     |


# Banda
| Ítem | Componente                        | Cant. | Referencia / Especificación                 | Función                                         | Est. Costo (COP) |
| :--- | :-------------------------------- | :---: | :------------------------------------------ | :---------------------------------------------- | :--------------: |
| 7    | **Microcontrolador**              |   1   | **ESP32 DevKit V1 / ESP32-WROOM-32 DevKit** | Adquisición y transmisión de datos fisiológicos |     $ 31.178     |
| 8    | **Sensor de frecuencia cardíaca** |   1   | **PulseSensor genérico**                    | Medición de la frecuencia cardíaca del atleta   |      $ 9.758     |
| 9    | **Batería recargable**            |   1   | **LiPo / Li-ion 3.7 V 2200 mAh**            | Alimentación autónoma de la banda               |     $ 20.000     |
| 10   | **Módulo de carga**               |   1   | **TP4056 con protección**                   | Carga segura de la batería de la banda          |      $ 5.500     |
| 11   | **Carcasa / soporte**             |   1   | **Caja impresa en PLA**                     | Integración mecánica del módulo fisiológico     |     $ 20.000     |

# Sistema completo
| Categoría                                                  | Subtotal Estimado |
| :--------------------------------------------------------- | :---------------- |
| Cronómetro                                                 | $ 112.278         |
| Peón                                                       | $ 168.988         |
| Peón                                                       | $ 168.988         |
| Banda                                                      | $ 86.436          |
| **TOTAL SISTEMA COMPLETO (Cronómetro + Banda + 2 Peones)** | **$ 536.690 COP** |


*(Precios estimados en mercado local colombiano - MercadoLibre / Electrónicas locales a Feb 2026)*

---
