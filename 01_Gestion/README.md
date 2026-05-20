01_Gestion — Gestión y control del proyecto Tréximo
Esta carpeta contiene la documentación de planeación, control, trazabilidad y auditoría del proyecto Tréximo: Sistema Inteligente de Cronometraje para Atletismo.
Su propósito es evidenciar cómo se tomaron las decisiones de diseño, cómo se validaron los subsistemas y cómo evolucionó el PMV durante los ciclos IPAC/CDIO.
Contenido recomendado
```text
01_Gestion/
├── README.md
├── Auditoria.md
├── BOM/
│   └── README.md
└── Planeación/
    ├── README.md
    ├── IPAC/
    │   └── README.md
    ├── Cronograma de actividades/
    ├── Auditoria y Planeación/
    ├── Expediente Técnico/
    ├── Definition_of_Done.md
    └── Protocolo_Pruebas/
        └── README.md
```
Documentos principales
Documento / carpeta	Propósito	Estado
`Planeación/README.md`	Explica la planeación general por fases CDIO/IPAC	Actualizar
`Planeación/IPAC/README.md`	Resume ciclos IPAC, entregables, decisiones y lecciones aprendidas	Actualizar
`BOM/README.md`	Documenta costos, proveedores, desviaciones y control financiero	Pendiente/Actualizar
`Planeación/Definition_of_Done.md`	Define criterios mínimos para dar por terminados firmware, hardware, software y documentación	Pendiente
`Planeación/Protocolo_Pruebas/README.md`	Resume pruebas unitarias, integración, campo y evidencia requerida	Pendiente
`Auditoria.md`	Registra hallazgos, riesgos, decisiones y trazabilidad técnica	Revisar
Estado del PMV
Área	Estado	Comentario
Cronómetro central	Funcional	Maneja estados, pantalla TFT, botones y subida a app
Peones salida/llegada	Funcional	Detección por ultrasonido y eventos ESP-NOW
UWB	Funcional	Calibración de distancia entre peones
App web	Funcional	Backend local + Flutter Web
Frecuencia cardiaca	Retirada del PMV	MAX30102 inestable en uso corporal; se usa valor fijo/simulado
Criterios de gestión
Toda entrega debe tener:
Responsable identificado.
Criterio de aceptación.
Evidencia asociada: foto, video, captura, log serial, commit o archivo.
Relación con un requisito o ciclo IPAC.
Estado: `Pendiente`, `En proceso`, `Validado`, `Parcial`, `Retirado del PMV`.
Convención de estados
Estado	Significado
Validado	Probado y documentado con evidencia
Parcial	Funciona con limitaciones o bajo condiciones específicas
Retirado PMV	Se excluye de la entrega mínima por causa técnica justificada
Pendiente	No implementado o sin evidencia suficiente
Requiere corrección	Falló en pruebas o necesita rediseño

