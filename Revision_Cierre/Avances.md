# Registro de avances del cierre

## Base

Rama `mejoras-teoria-final`, desde main `9eb34c8`. El commit `dce86e6` incorpora la revision local anterior ya probada; no representa trabajo historico inventado. Main y ramas individuales se conservan.

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

