# Sprint Timer - UI Kit

El diseño de "Sprint Timer" se centra en una experiencia **Dark Mode** (Modo Oscuro) por defecto, dado que es una app deportiva que puede ser utilizada en exteriores o en pistas de atletismo. El modo oscuro reduce el deslumbramiento, mejora el contraste para métricas clave, y aporta una sensación "Premium" e "Inteligente".

## 1. Paleta de Colores

Se utilizan colores de fondo profundos con acentos vibrantes para crear una jerarquía clara.

| Token de Color | Valor (Hex) | Uso Principal |
| :--- | :--- | :--- |
| **Background (Scaffold)** | `#0B0C10` | Fondo principal de las pantallas. Muy oscuro. |
| **Surface (Card)** | `#1F2833` | Fondo de tarjetas, modales y elementos elevados. |
| **Primary (Accent)** | `#66FCF1` | Botones principales, enlaces, progreso (Cian/Neón). |
| **Secondary (Accent)** | `#45A29E` | Variaciones secundarias del primary, bordes sutiles. |
| **Text Primary** | `#FFFFFF` | Textos de alto contraste (Títulos, métricas grandes). |
| **Text Secondary** | `#C5C6C7` | Textos secundarios (Subtítulos, etiquetas, leyendas). |
| **Success** | `#00E676` | Indicadores positivos (Mejora de PR, Sistema Conectado). |
| **Warning/Error** | `#FF1744` | Indicadores negativos (Desconexión, Error, Peor marca). |

## 2. Tipografía

La app emplea dos familias tipográficas (previamente configuradas en `pubspec.yaml`):

### DM Sans (UI Principal)
- Propósito: Textos descriptivos, botones, títulos.
- Estilos:
  - `HeadlineLarge`: DM Sans Bold, 32sp (Títulos de página).
  - `TitleMedium`: DM Sans SemiBold, 18sp (Títulos de tarjeta).
  - `BodyMedium`: DM Sans Regular, 14sp (Párrafos, descripciones).
  - `LabelLarge`: DM Sans Medium, 14sp (Botones).

### JetBrains Mono (Métricas y Datos)
- Propósito: Mostrar datos tabulares, cronómetros, IDs, velocidades. Al ser monoespaciada, evita saltos visuales cuando los números cambian rápidamente (ej. cronómetro en vivo).
- Estilos:
  - `DisplayLarge`: JetBrains Mono Bold, 48sp (Cronómetro en vivo).
  - `HeadlineMedium`: JetBrains Mono Bold, 24sp (Métricas en tarjetas).
  - `BodySmall`: JetBrains Mono Regular, 12sp (Fechas, IDs en listas).

## 3. Espaciado (Grid & Margins)

- Sistema basado en múltiplos de 8.
- Margen de pantalla (Screen Padding): `16px`.
- Espaciado entre componentes (Gap): `16px` o `24px` para separación de secciones.
- Borde de tarjetas (Border Radius): `16px` (Bordes redondeados, amigables pero tecnológicos).

## 4. Componentes Clave

1. **MetricCard**: 
   - Una tarjeta (Surface) que contiene un ícono o título en pequeño (Text Secondary) y un valor numérico en grande (JetBrains Mono, Text Primary).
   - Opcional: Una pequeña flecha de tendencia (Success/Error) y un % de cambio.

2. **StatusChip**:
   - Etiqueta pequeña y redondeada.
   - Variantes:
     - `Active/Connected`: Fondo verde muy translúcido (`#00E676` al 15%), texto verde.
     - `Disconnected`: Fondo rojo translúcido, texto rojo.
     - `Neutral`: Fondo gris translúcido, texto blanco/gris.

3. **SessionTile**:
   - Fila de la lista de historial.
   - Izquierda: Icono de deporte o distancia.
   - Centro: Fecha y Distancia.
   - Derecha: Tiempo principal (JetBrains Mono) y pequeño badge de ECV.

4. **FilterBar**:
   - Lista horizontal (`SingleChildScrollView` o `ListView` horizontal) con `ActionChips`.
   - Permite seleccionar rápidamente "50m", "100m", "Esta semana".

5. **ChartSection (fl_chart)**:
   - Gráfico de líneas (LineChart).
   - Fondo: Transparente.
   - Línea principal: Primary (`#66FCF1`).
   - Sombreado bajo la línea: Gradiente del Primary hacia transparente (`#66FCF1` con `alpha 0.3` a `0.0`).
   - Grid lines: Ocultas o muy tenues (`alpha 0.1`) para no saturar.

## 5. Micro-Interacciones (Animaciones)
- Las transiciones de pantalla utilizan *Fade* o *Slide* muy rápido (200ms) para dar sensación de velocidad, acorde a la temática de "sprints".
- Al presionar un botón o tarjeta, debe haber un efecto sutil de escalado (`ScaleTransition` al 0.98).
