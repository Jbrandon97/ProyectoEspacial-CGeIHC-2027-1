# Revisión integral del cierre

## Alcance y estado

Revisamos el código que está en `main`, las capturas OpenGL y los resultados recientes de `--pruebas` y `--pruebas-integrales`. Conservamos los tres escenarios, las teclas de región 1–3, las cámaras F1–F4 y la recuperación de nave con C. No editamos la lógica de Planeta 2 ni Espacio fuera de las pruebas de Railjack, ni tocamos las ramas de colaboración.

## Auditoría de afirmaciones

| Afirmación revisada | Lo que encontramos en el código y las pruebas |
|---|---|
| La patrulla de Planeta 1 solo rota al NPC. | Incorrecto para el código actual: `SistemasPlaneta1::posicionPatrulla` calcula posiciones elípticas; `Planeta1::dibujar` traslada el modelo completo, ajusta altura y orienta el cuerpo. Añadimos movimiento alternado de piernas en el shader. |
| Los GLTF de Grunt y Hunter no contienen rig o animación. | Los archivos declaran skins y canales de animación, pero `ModeloGltf` no evalúa esas pistas de piel. La marcha implementada aquí es deformación de vértices durante el dibujo; no la presentamos como reproducción del clip GLTF. |
| El HUD conserva renglones de silencio o nombres de reserva. | No aparecen en el código del HUD actual. `actividad()` del Planeta 2 produce tres renglones: título regional y controles; las pruebas muestran `HUD_Limpio_3_Regiones.jpg`. |
| Las bases de Planeta 2 flotan. | La revisión de los seis edificios, de día y de noche, no encontró huecos. Cada modelo se hunde por lo menos 0.12 unidades en su punto inferior. Conservamos apoyos y colisiones. |
| Railjack solo se carga, sin comprobar su trayectoria. | Encontramos un error real: el cargador no guardaba `triangulos_` para modelos indexados, pero el apoyo de Railjack buscaba allí su vértice mínimo y calculaba una altura inválida. Ahora `ModeloGltf::minimoY()` acumula límites con la transformación de nodos y `SectorRatchet` los usa para apoyar el modelo. Las pruebas verifican contacto y ascenso de la geometría, además de capturar ambos estados. |

## Planeta 1: desplazamiento y marcha de NPC

La patrulla no es solo un giro sobre el eje. Cada reloj actualiza el punto del cuerpo sobre una elipse, con `deltaTime`; el modelo completo se dibuja trasladado a esa posición. La altura sale de `TerrenoPlaneta1::altura` y el rumbo sigue la tangente. G, K y H congelan el reloj del personaje durante su reacción.

En esta revisión ampliamos los radios de las cuatro rutas para que el desplazamiento se note desde el patio: Grunt 1, 2 y 3 recorren elipses de 6×5 unidades, con periodos de 22, 25 y 28 segundos; el Hunter recorre 4.5×5 unidades en 30 segundos y mantiene su circuito más lento y corto que el Grunt 3. La prueba de 31 segundos midió 48.57, 42.97, 38.58 y 30.91 unidades recorridas. El cuerpo del Grunt 1 pasó de `(-77,153)` a `(-72.4655,151.274)` en 3 segundos.

Para dar lectura de marcha sin sustituir ni remodelar a los NPC, `Renderizador::locomocionPatrulla` entrega una fase calculada desde el reloj del circuito y `Recursos/Shaders/basico.vert` inclina vértices inferiores alrededor de su pivote. Las mitades izquierda y derecha reciben fases opuestas. Los Grunts usan 24 ciclos por vuelta y Hunter 16. La fase se desactiva mientras el personaje reacciona. Las capturas muestran al mismo Grunt 1 en dos puntos del patio y su reacción con G:

- `Capturas/P1_Grunt1_t0.bmp` — inicio `(-77,153)`.
- `Capturas/P1_Grunt1_t3.bmp` — después de 3 s `(-72.4655,151.274)`.
- `Capturas/P1_Grunt1_Reaccion_G.bmp` — reacción cercana y pausa de la patrulla.

No alteramos las funciones de Titan ni de los dos Sentinel. `--pruebas` informa sus estados existentes y mantiene la comprobación de que la pausa de reacción no teletransporta al NPC al reanudar.

## Planeta 2: apoyos y Railjack

Revisamos seis bases en las dos iluminaciones. La tabla registra hundimiento inferior mínimo y máximo observado en la huella, en unidades del escenario:

| Base | Mínimo | Máximo |
|---:|---:|---:|
| 0 | 0.120 | 0.343 |
| 1 | 0.120 | 1.181 |
| 2 | 0.120 | 0.386 |
| 3 | 0.120 | 0.841 |
| 4 | 0.120 | 1.923 |
| 5 | 0.120 | 0.753 |

La base 1 tiene un pequeño soporte violeta del propio modelo que llega al terreno; en las vistas no deja un hueco. La base 4 se entierra más en el lado alto por la pendiente, pero no queda suspendida. Por esta evidencia no cambiamos offsets ni huellas de colisión. `Capturas/P2_Bases_Dia_Noche.bmp` ordena las seis comparaciones: cada pareja contigua es día (barra amarilla) y noche (barra azul), de izquierda a derecha.

El Railjack permanece asociado a `PatioRailjack` y el apoyo se calcula con el mínimo del GLTF más el margen de 0.12 unidades. La secuencia conserva el reposo inicial, asciende gradualmente entre los segundos 8 y 12, sostiene altura entre 12 y 16, desciende entre 16 y 20 y regresa al suelo para el segundo 22. `P2_RAILJACK_OK` comprueba el mínimo real de la malla a Y=3.5444 (terreno más 0.12), su base a Y=22.5444 en vuelo y el regreso al apoyo; el centro reportado pasa de Y=12.4244 a Y=31.4244. La comparación se renderiza con iluminación diurna controlada y constante. `Capturas/P2_Railjack_Suelo.bmp`, `Capturas/P2_Railjack_Altura.bmp` y `Capturas/P2_Railjack_Regreso.bmp` muestran apoyo, ascenso y regreso. No afirmamos haber hecho una partida manual con teclado en esta máquina.

## Requisitos de animación registrados

`C_ANIMACIONES_OK` identifica tres objetos distintos con animación básica (puerta, Grunt 1 y Grunt 2), tres con movimiento continuo (Engineer, Sentinel 1 y Sentinel 2) y dos mediante keyframes (Saturn V y holocrón). Las patrullas adicionales de NPC no se cuentan por segunda vez para completar estas categorías.

## Resultado de las pruebas

- Compilación de `ProyectoEspacial.sln`, Visual Studio 2022, `Release | x64`: correcta; cero errores. Las advertencias corresponden a GLFW, miniaudio y la definición de `APIENTRY`.
- `ProyectoEspacial.exe --pruebas --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye contratos de regiones, carrera, P1, P2, Freeport, audio, cargadores GLTF, apoyos, rutas y capturas.
- `ProyectoEspacial.exe --pruebas-integrales --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye 5400 muestras de NPC/suelo/separación de P2, costuras del cielo, mezcla de audio y la verificación del Railjack.
- La salida del Railjack registra base en terreno `baseY=3.5444`, base en vuelo `aireBaseY=22.5444`, centros `12.4244/31.4244`; entre las capturas con iluminación diurna controlada cambian 234843 canales RGB en `--pruebas-integrales` y 234526 en `--pruebas`.
- Las capturas de P1, P2 y Railjack se inspeccionaron visualmente. Son capturas automatizadas renderizadas por el ejecutable, no una grabación de partida manual.
- El reporte de rendimiento existente corresponde a mediciones previas en Intel UHD Graphics 620; no se repitió para esta corrección de piernas. No afirmamos una cifra de rendimiento nueva.

## Integración y límites

Los cambios de esta revisión van a `main`. No tocamos ramas de compañeros ni recreamos `mejoras-teoria-final`. El cuarto recorrido académico que incluye estación y cuerpos celestes sigue dependiendo de la integración del equipo. La comprobación del juego completo con teclado y mouse en el equipo del estudiante queda fuera de estas pruebas automatizadas.
