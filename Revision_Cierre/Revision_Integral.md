# Diagnóstico y diseño del cierre local

## Alcance

Trabajamos sobre la rama `mejoras-teoria-final`, derivada de `main` sin modificar ni publicar `main`. El cierre integra los tres escenarios existentes: Planeta 1 Halo/Titanfall, Espacio con Freeport y Planeta 2 con la distribución Kerwan/Orokin/Railjack del croquis compartido.

## Decisiones de diseño

- Se conservaron 1–3 para cambio de región, F1–F4 para las cámaras existentes y C para recuperar la nave.
- Planeta 1 recibió el holocrón de seis estados temporales, el reflector Covenant, dos luces de paso y la vista de colonia en F3.
- Espacio conserva Freeport como estación principal. El tráfico usa curvas Bézier con continuidad de velocidad; no se reintrodujo Gateway ni colisiones antiguas.
- Planeta 2 usa caminos y plazas procedurales de baja geometría, tres patrullas NPC, dos spotlights y la plataforma Railjack. El Railjack se redujo fuera del ejecutable mediante un GLTF reproducible.
- El audio se reproduce con miniaudio y WAV PCM16 convertidos desde originales conservados. Las pistas distribuidas tienen procedencia CC0 documentada.

## Evidencia visual

- `Capturas/planeta1_revision.png`: holocrón, reflector, carrera y observación.
- `Capturas/espacio_revision.png`: tráfico alrededor de Freeport en cuatro tiempos.
- `Capturas/planeta2_revision.png`: edificios, torre, recorridos, Railjack y comparación día/noche.
- `Referencias/Planeta2_Croquis.png`: imagen de diseño proporcionada por el usuario, usada como referencia de distribución.

## Límites honestos

Las pruebas automatizadas renderizan cuadros, simulan deltaTime, inspeccionan píxeles, colisiones, audio y cargadores GLTF. No sustituyen una partida completa con teclado/mouse ni la escucha subjetiva prolongada en el equipo del estudiante. Tampoco se declara validado el cuarto recorrido académico completo de estación y cuerpos celestes: aquí se deja preparado el tramo de Planeta 1/Freeport.
