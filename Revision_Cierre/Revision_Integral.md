# Diagnóstico y diseño del cierre local

## Alcance

El cierre reúne los tres escenarios existentes: Planeta 1 Halo/Titanfall, Espacio con Freeport y Planeta 2 con la distribución Kerwan/Orokin/Railjack del croquis compartido. La revisión mantiene sus módulos separados y documenta sus pruebas y límites.

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

## Última revisión visual y de controles

El HUD deja el título regional breve y los controles de navegación/modo. Quitamos el renglón de volumen, la descripción técnica de patrullas y el texto de recursos que no sirve durante la partida. M/N/+/- continúan procesándose por teclado; solo se ocultaron sus indicadores visuales. La interfaz conserva el pie con Planeta 1, Planeta 2, Espacio y Esc para salir.

Las capturas `Capturas/P1_Patrullas_Reacciones.jpg` y `Capturas/HUD_Limpio_3_Regiones.jpg` son salidas reales de `--pruebas`. Cada imagen de P2 se encuadró a ras del edificio para revisar el contacto con el terreno de día y noche. Los seis tienen al menos 0.12 unidades de penetración en su punto más bajo, sin hueco; el máximo registrado va de 0.343 a 1.923 unidades según la pendiente de la huella. La cuarta base (índice 4) presenta el mayor desnivel enterrado. Se ve apoyada en ambas iluminaciones, pero por la pendiente de casi 2 unidades conviene una última comprobación desde la cámara normal del jugador en Windows. Conservamos el valor de soporte de 0.12 y las colisiones actuales.

Los circuitos de los tres Grunt y el Hunter se desarrollan dentro de la elipse de Recinto (centro aproximado -65,160; semiejes 52,28). `SistemasPlaneta1::posicionPatrulla` desplaza el punto de origen del modelo y lo apoya sobre el terreno; `Planeta1::dibujar` aplica esa traslación al Grunt/Hunter completo y orienta el cuerpo según la tangente. `Aplicacion::probarC` muestrea avance, altura, separación y colisión cada 0.1 s. En sus 31 s registrados, las distancias recorridas fueron 30.92 u (Grunt1), 24.09 u (Grunt2), 24.71 u (Grunt3) y 20.46 u (Hunter), y los cuatro terminan en coordenadas diferentes a las iniciales. Al activar G/K/H el NPC mantiene su posición mientras corre la reacción y recupera el movimiento sin teletransporte. La comprobación visual de capturas evidencia los cuadros, pero el ensayo físico con teclado en una partida sigue pendiente.
