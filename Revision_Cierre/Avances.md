# Registro de avances del cierre

## Base

La rama histórica `mejoras-teoria-final` partió de `main` en `9eb34c8` y sus cambios quedaron integrados en `main`; se eliminó localmente en el cierre anterior. En esta revisión trabajamos únicamente en `main`, sin modificar ramas de los compañeros.

## Planeta 1 — 13 de noviembre de 2026

Holocron con seis estados temporales, elevacion y escala3.5, retorno12s, Y cercana al pedestal Covenant. Modelo reutilizado sin inventar partes articuladas. Reflector adicional I, dos luminarias de paso y ajuste moderado de ambiente nocturno. HUD de carrera muestra velocidad y posicion reales; F3 encuadra colonia. Exploration Theme convertida desde OGG y original conservado.

Release x64 compilado, regresion completa y pruebas integrales posteriores al encuadre de F3: salida0. Capturas OpenGL revisadas de holocron, luces, carrera y camara. Las pruebas heredadas del HUD exigian ocultar velocidad: se actualizaron para comprobar los datos solicitados. No se declara partida manual ni escucha subjetiva prolongada.

Las pistas proporcionadas tienen procedencia CC0 documentada en Recursos/Audio/Musica_licencias.md; no se atribuyen al equipo. No se inventa participacion de David, Miguel o Pedro en estos cambios.

## Espacio — 13 de noviembre de 2026

Se conservan Freeport, colisiones por componentes y orientacion propia Alien +Z/Banshee -Z. Se corrigio una parada innecesaria al cambiar de tramo: las curvas comparten velocidad de 8 u/s en sus enlaces y solo frenan para acoplar. Airy sustituye la pista sintetizada. Release y pruebas integrales: salida0, continuidad de velocidad, giros, colisiones normal/boost y audio; capturas reales de aproximacion/acoplamiento/salida/retorno. No se agregan esferas ni otra geometria de galaxia superpuesta.

## Planeta 2 — 16 de noviembre de 2026

Se alineo la distribución con el croquis compartido: acceso frontal, plaza Kerwan, corredor de transición, torre Orokin/Warframe y plataforma Railjack. Se reutilizaron las edificaciones existentes y se agregaron caminos, plazas y cuatro luminarias de recorrido mediante geometría ligera. La cámara F3 encuadra la plaza y F4 el sector Warframe; P e I controlan los dos spotlights independientes. La iluminación nocturna mantiene visibilidad y activa las luces de camino.

Los tres NPC tienen centros y radios independientes: Clank (plaza), Blarg (transición) y Warframe (torre). Sus rutas son continuas y el shader añade locomoción visible al Blarg sin modificar su malla. Railjack conserva la animación de piezas exportadas; se generó `Railjack_Optimizado.gltf` con 117153 triángulos frente a 390536 del GLB de trabajo (reducción 69.999%). El apoyo se calcula sobre el terreno y la plataforma, evitando dejarlo flotando.

La pista musical de Planeta 2 es `Our_Expanse_Chill.wav`; el original OGG y su procedencia CC0 se conservan. La batería integral obtuvo salida0: 5400 muestras de NPC/suelo/separación, foco con 138492 píxeles modificados, cielo día/noche y cierre de miniaudio. La captura `Capturas/planeta2_revision.png` documenta edificios, torre, caminos, Railjack y el contraste día/noche.

## Verificación final local

Release y Debug x64 fueron reconstruidos con Visual Studio 2022. `full_final.log` termina en `CIERRE CORRECTO`; incluye cargadores GLTF, contratos de regiones, carrera, Freeport, Planeta 2 y miniaudio. `rendimiento_final.log` midió 120 muestras por región con VSync desactivado en Intel UHD Graphics 620: Planeta 1 59.7509 FPS, Planeta 2 44.1205 FPS y Espacio 85.7457 FPS. Son mediciones automatizadas de esta máquina, no una garantía para otro hardware.

## Correcciones finales — 16 de noviembre de 2026

El HUD ya no muestra el volumen/canal, el encabezado temático de cada planeta ni la línea técnica de patrullas/Railjack. Se conservan los controles de audio M/N/+/- en el teclado, la cabecera breve por región, F1/F3/F4, la energía corporal, observación del Saturn V y la navegación fija 1–3/ESC.

En Planeta 1 agregamos cuatro circuitos elípticos independientes: Grunt 1 (22 s), Grunt 2 (25 s), Grunt 3 (28 s) y Hunter (30 s). Todos parten de las posiciones acordadas, se mantienen dentro de los límites del Recinto y calculan altura mediante `TerrenoPlaneta1::altura`. El rumbo sigue la tangente del recorrido y `Planeta1::dibujar` traslada el modelo completo, no solo su orientación. En la prueba de 31 s avanzaron 30.92, 24.09, 24.71 y 20.46 unidades respectivamente. La corrección final de G/K/H ya no congela los relojes: permite modular la marcha cerca del Grunt 1, Grunt 2 o Hunter mientras su patrulla continúa. El Titán y ambos Sentinel conservan sus funciones y transformaciones.

La revisión de las seis bases de Planeta 2 capturó cada edificio durante día y noche. El análisis de sus vértices inferiores no encontró huecos: hundimiento mínimo medido de 0.12 unidades en los seis apoyos; máximos por edificio 0.343, 1.181, 0.386, 0.841, 1.923 y 0.753. El modelo 4 queda más enterrado por la pendiente de su huella, pero en las vistas no aparece flotación ni hueco; se mantuvo el margen de apoyo de 0.12 y no cambiamos colisiones.

Las hojas `Capturas/P1_Patrullas_Reacciones.jpg`, `Capturas/P2_Bases_Dia_Noche.jpg` y `Capturas/HUD_Limpio_3_Regiones.jpg` se generaron del ejecutable y muestran movimiento, respuestas, apoyo en ambos ciclos y HUD final. `--pruebas` termina en `CIERRE CORRECTO`; la salida contiene `P1_PATRULLA_OK`, `C_ANIMACIONES_OK` con SaturnV/Holocron como los dos objetos por keyframes, y `PLANETA2_OK`.

## Revisión final de marcha y vuelo — 16 de noviembre de 2026

La auditoría comprobó que las patrullas ya trasladaban el cuerpo completo; para hacer visible el paso primero se había añadido una fase al shader. La revisión siguiente confirmó que los GLTF de Grunt y Hunter eran planos y no tenían skins ni animaciones. La corrección definitiva de 2026-11-17 separó en Blender las dos islas de pierna de cada NPC y las hizo hijas del torso, con pivotes obtenidos de los nodos exportados. No se crearon huesos ni se alteraron materiales, texturas o UV.

`ModeloGltf::indiceNodo` y `origenNodo` permiten resolver nombres y pivotes del archivo GLTF cargado. `Planeta1::dibujar` aplica matrices rígidas Rz de ángulos opuestos, sincronizadas al reloj de patrulla (24 ciclos por vuelta de Grunt, 16 de Hunter). La corrección conserva pivotes, jerarquía y fase; G/K/H ahora se leen mientras se mantienen, modulan la amplitud de paso sin pausar la ruta y retornan gradualmente a ±0.35 rad al soltar. Se quitó la deformación P1 del shader, conservando los modos de Clank y Blarg. Los puntos de pivote de Muse no correspondían a los ejes de los archivos reales; usamos los puntos medidos sobre las islas exportadas, cuyas separaciones laterales se encuentran en Z glTF.

Release x64 se compiló y `--pruebas`/`--pruebas-integrales` terminaron en `CIERRE CORRECTO`. Se capturaron y revisaron en OpenGL tres poses por Grunt 1 y Hunter, incluidas en `Capturas/NPC_Locomocion/`. `--medir-integral` en Intel UHD 620 obtuvo 68.50 FPS medios en P1, p95 17.56 ms (120 muestras); el dato cubre el escenario completo, no aísla el costo de las piernas. El archivo `Pruebas_Cierre.txt` detalla los resultados; no se declara partida manual. Las seis bases de P2 continúan documentadas en `P2_Bases_Dia_Noche.bmp`. Railjack permanece capturado a Y=12.4244 en reposo y Y=31.4244 en vuelo en `P2_Railjack_Suelo.bmp` y `P2_Railjack_Altura.bmp`.

## Corrección del apoyo del Railjack — 17 de noviembre de 2026

La revisión visual detectó que el modelo no aparecía pese a que su reloj calculaba ascenso. La causa estaba en el cargador: los modelos indexados no agregan vértices a `triangulos()`, pero el cálculo del mínimo del Railjack leía esa lista vacía. Añadimos `ModeloGltf::minimoY()`, que acumula el mínimo considerando las transformaciones de nodos sin copiar la malla, y cambiamos `SectorRatchet` para usarlo al calcular el apoyo. La prueba comprueba la base de malla a 0.12 unidades sobre el terreno, su elevación de 19 unidades y el retorno. La nave ahora se ve tanto en el pad como durante el vuelo en `P2_Railjack_Suelo.bmp`, `P2_Railjack_Altura.bmp` y `P2_Railjack_Regreso.bmp`.

