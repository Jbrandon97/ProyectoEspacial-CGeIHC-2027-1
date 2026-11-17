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

En Planeta 1 agregamos cuatro circuitos elípticos independientes: Grunt 1 (22 s), Grunt 2 (25 s), Grunt 3 (28 s) y Hunter (30 s). Todos parten de las posiciones acordadas, se mantienen dentro de los límites del Recinto y calculan altura mediante `TerrenoPlaneta1::altura`. El rumbo sigue la tangente del recorrido y `Planeta1::dibujar` traslada el modelo completo, no solo su orientación. En la prueba de 31 s avanzaron 30.92, 24.09, 24.71 y 20.46 unidades respectivamente. G/K/H congelan el reloj del NPC que reacciona (5 s para G; 8 s para K/H), de modo que la patrulla reanuda desde el mismo punto. El Titán y ambos Sentinel conservan sus funciones y transformaciones.

La revisión de las seis bases de Planeta 2 capturó cada edificio durante día y noche. El análisis de sus vértices inferiores no encontró huecos: hundimiento mínimo medido de 0.12 unidades en los seis apoyos; máximos por edificio 0.343, 1.181, 0.386, 0.841, 1.923 y 0.753. El modelo 4 queda más enterrado por la pendiente de su huella, pero en las vistas no aparece flotación ni hueco; se mantuvo el margen de apoyo de 0.12 y no cambiamos colisiones.

Las hojas `Capturas/P1_Patrullas_Reacciones.jpg`, `Capturas/P2_Bases_Dia_Noche.jpg` y `Capturas/HUD_Limpio_3_Regiones.jpg` se generaron del ejecutable y muestran movimiento, respuestas, apoyo en ambos ciclos y HUD final. `--pruebas` termina en `CIERRE CORRECTO`; la salida contiene `P1_PATRULLA_OK`, `C_ANIMACIONES_OK` con SaturnV/Holocron como los dos objetos por keyframes, y `PLANETA2_OK`.

## Revisión final de marcha y vuelo — 16 de noviembre de 2026

La auditoría comprobó que las patrullas ya trasladaban el cuerpo completo; para hacer visible el paso añadimos una fase alternada de piernas a Grunts y Hunter en `Renderizador::locomocionPatrulla` y `Recursos/Shaders/basico.vert`. La prueba reciente sitúa al Grunt 1 en `(-77,153)` al inicio y `(-72.4655,151.274)` a los tres segundos; recorre 48.57 unidades en 31 segundos. Sus capturas individuales `P1_Grunt1_t0.bmp` y `P1_Grunt1_t3.bmp` muestran al mismo NPC en ambos momentos.

El código fuente auditado no reproduce los canales de piel/animación GLTF de Grunt/Hunter; implementa el paso con deformación de vértices. Las seis bases de P2 vuelven a inspeccionarse en la hoja `P2_Bases_Dia_Noche.bmp`. Railjack se capturó a Y=12.4244 en reposo y Y=31.4244 en vuelo con iluminación constante; las evidencias son `P2_Railjack_Suelo.bmp` y `P2_Railjack_Altura.bmp`. Las pruebas `--pruebas` y `--pruebas-integrales` terminan en `CIERRE CORRECTO` en la compilación Release x64.

## Corrección del apoyo del Railjack — 17 de noviembre de 2026

La revisión visual detectó que el modelo no aparecía pese a que su reloj calculaba ascenso. La causa estaba en el cargador: los modelos indexados no agregan vértices a `triangulos()`, pero el cálculo del mínimo del Railjack leía esa lista vacía. Añadimos `ModeloGltf::minimoY()`, que acumula el mínimo considerando las transformaciones de nodos sin copiar la malla, y cambiamos `SectorRatchet` para usarlo al calcular el apoyo. La prueba comprueba la base de malla a 0.12 unidades sobre el terreno, su elevación de 19 unidades y el retorno. La nave ahora se ve tanto en el pad como durante el vuelo en `P2_Railjack_Suelo.bmp`, `P2_Railjack_Altura.bmp` y `P2_Railjack_Regreso.bmp`.

