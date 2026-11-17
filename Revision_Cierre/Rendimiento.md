# Rendimiento medido

Medición automatizada con `--medir-integral`, 120 muestras por región, VSync desactivado, 1280x800, Intel UHD Graphics 620 y OpenGL 3.3. La medición incluye dibujo, HUD, simulación y audio de la escena.

| Región | FPS medio | Mediana (ms) | P95 (ms) |
|---|---:|---:|---:|
| Planeta 1 | 59.7509 | 16.9070 | 17.5584 |
| Planeta 2 | 44.1205 | 22.5097 | 24.1051 |
| Espacio / Freeport | 85.7457 | 11.9641 | 15.1583 |

La reducción de Railjack dejó el modelo en 117153 triángulos frente a 390536 del recurso de trabajo, sin sustituirlo por geometría genérica. Estos valores son una referencia de esta máquina; no se presentan como garantía de FPS en otro equipo.
