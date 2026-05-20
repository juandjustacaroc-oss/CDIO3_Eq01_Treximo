# Gestión del Proyecto — TRÉXIMO

Esta carpeta contiene la documentación de planeación, control, trazabilidad y auditoría del proyecto **TRÉXIMO: Sistema Inteligente de Cronometraje para Atletismo**.

Su propósito es evidenciar:
- La evolución del Producto Mínimo Viable (PMV).
- Las decisiones de diseño tomadas durante el desarrollo.
- La validación de subsistemas.
- La gestión técnica y organizacional del proyecto bajo metodología CDIO/IPAC.

---

# Estructura de la carpeta

```text
01_Gestion/
├── README.md
├── Auditoria.md
├── BOM/
│   └── README.md
└── Planeacion/
    ├── README.md
    ├── IPAC/
    │   └── README.md
    ├── Cronograma de actividades/
    ├── Auditoria y Planeacion/
    ├── Expediente Tecnico/
    ├── Definition_of_Done.md
    └── Protocolo_Pruebas/
        └── README.md
```

---

# Documentos principales

| Documento / Carpeta | Propósito | Estado |
|---|---|---|
| `Planeacion/README.md` | Explica la planeación general por fases CDIO/IPAC | En actualización |
| `Planeacion/IPAC/README.md` | Resume ciclos IPAC, entregables, decisiones y lecciones aprendidas | En actualización |
| `BOM/README.md` | Documenta costos, proveedores, desviaciones y control financiero | Pendiente |
| `Planeacion/Definition_of_Done.md` | Define criterios mínimos de finalización para hardware, firmware, software y documentación | Pendiente |
| `Planeacion/Protocolo_Pruebas/README.md` | Resume pruebas unitarias, integración, pruebas de campo y evidencia requerida | Pendiente |
| `Auditoria.md` | Registra hallazgos, riesgos, decisiones y trazabilidad técnica | En revisión |

---

# Estado actual del PMV

| Área | Estado | Comentario |
|---|---|---|
| Cronómetro central | Funcional | Manejo de estados, pantalla TFT, botones y conexión con aplicación |
| Peones de salida/llegada | Funcional | Detección mediante ultrasonido y comunicación ESP-NOW |
| Sistema UWB | Parcial | Requiere calibración y validación de distancia real |
| Aplicación web | Funcional | Backend local y visualización mediante Flutter Web |
| Frecuencia cardiaca | Retirada del PMV | MAX30102 presentó inestabilidad en condiciones reales de uso |

---

# Criterios de gestión

Toda entrega o componente desarrollado debe incluir:

- Responsable identificado.
- Criterio de aceptación definido.
- Evidencia asociada:
  - fotografía,
  - video,
  - captura,
  - log serial,
  - commit,
  - archivo técnico.
- Relación con un requisito funcional o ciclo IPAC.
- Estado documentado.

---

# Convención de estados

| Estado | Significado |
|---|---|
| `Validado` | Probado y documentado con evidencia |
| `Parcial` | Funciona con limitaciones o bajo condiciones específicas |
| `Retirado del PMV` | Excluido de la entrega mínima por causa técnica justificada |
| `Pendiente` | No implementado o sin evidencia suficiente |
| `Requiere corrección` | Falló pruebas o necesita rediseño |

---

# Objetivo de gestión

La documentación de esta carpeta busca garantizar:
- trazabilidad técnica,
- control de cambios,
- seguimiento de requisitos,
- validación progresiva,
- y soporte para auditorías académicas y técnicas del proyecto.

---
