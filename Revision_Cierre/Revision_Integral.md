# Revisión integral del cierre

## Alcance y estado

Revisamos el código que está en `main`, las capturas OpenGL y los resultados recientes de `--pruebas` y `--pruebas-integrales`. Conservamos los tres escenarios, las teclas de región 1–3, las cámaras F1–F4 y la recuperación de nave con C. No editamos la lógica de Planeta 2 ni Espacio fuera de las pruebas de Railjack, ni tocamos las ramas de colaboración.

## Auditoría de afirmaciones

| Afirmación revisada | Lo que encontramos en el código y las pruebas |
|---|---|
| La patrulla de Planeta 1 solo rota al NPC. | Incorrecto: `SistemasPlaneta1::posicionPatrulla` calcula posiciones elípticas; `Planeta1::dibujar` traslada el cuerpo completo, ajusta altura y orienta el modelo. La fase de piernas ahora se aplica como rotación rígida de nodos glTF hijos del torso, no con deformación del shader. |
| Los GLTF de Grunt y Hunter no contienen rig o animación. | Confirmado para los archivos originales: estaban planos, sin nodos hijos, skins ni animaciones. Los nuevos glTF incorporan dos nodos rígidos de pierna como hijos del torso, sin huesos ni clips. |
| El HUD conserva renglones de silencio o nombres de reserva. | No aparecen en el código del HUD actual. `actividad()` del Planeta 2 produce tres renglones: título regional y controles; las pruebas muestran `HUD_Limpio_3_Regiones.jpg`. |
| Las bases de Planeta 2 flotan. | La revisión de los seis edificios, de día y de noche, no encontró huecos. Cada modelo se hunde por lo menos 0.12 unidades en su punto inferior. Conservamos apoyos y colisiones. |
| Railjack solo se carga, sin comprobar su trayectoria. | Encontramos un error real: el cargador no guardaba `triangulos_` para modelos indexados, pero el apoyo de Railjack buscaba allí su vértice mínimo y calculaba una altura inválida. Ahora `ModeloGltf::minimoY()` acumula límites con la transformación de nodos y `SectorRatchet` los usa para apoyar el modelo. Las pruebas verifican contacto y ascenso de la geometría, además de capturar ambos estados. |

## Planeta 1: desplazamiento y marcha de NPC

Cada reloj actualiza el centro del cuerpo sobre una elipse con `deltaTime`; `Planeta1::dibujar` traslada el modelo completo, calcula altura con `TerrenoPlaneta1::altura` y orienta el rumbo sobre la tangente. Grunt 1/2/3 recorren elipses de 6×5 unidades con periodos de 22/25/28 s; Hunter recorre 4.5×5 unidades en 30 s. En 31 s, `--pruebas` midió recorridos de 48.57, 42.97, 38.58 y 30.91 unidades. G/K/H ya no ejecutan reacciones ni pausan relojes: sostienen una marcha algo más amplia cerca del NPC y la patrulla sigue avanzando.

La auditoría confirmó que los archivos anteriores de Grunt y Hunter tenían jerarquía plana, 0 skins y 0 animaciones. Las cotas y pivotes indicados por Muse no concordaban con las coordenadas que realmente usa el glTF. En Blender se detectaron las dos islas de pierna por conectividad de caras, se separaron y se emparentaron al torso sin cambiar la pose global. La comparación visual de reposo entre originales y reexportados no mostró diferencias ni costuras; UV y PNG permanecen iguales.

`ModeloGltf` conserva nombres e inicios de nodo en coordenadas de modelo y permite encontrarlos por nombre. En `Planeta1::cargar` se resuelven los nodos `Grunt_pierna_izquierda/derecha` y `Hunter_pierna_izquierda/derecha` del archivo cargado, sin índices codificados. En cada cuadro, `Planeta1::dibujar` arma `T(pivote) * Rz(ángulo) * T(-pivote)`. Este giro mueve las piernas en el plano X-Y, hacia delante y atrás. Conservamos pivotes, nodos, desfases y fase de patrulla: Grunts usan 24 ciclos por vuelta y Hunter 16. La marcha habitual conserva ±0.35 rad; mientras se mantiene la tecla cercana, la amplitud sube suavemente hasta ±0.50 rad y vuelve al soltarla.

Los pivotes medidos en el glTF final son Grunt derecha `(-0.0280,0.9000,-0.1602)`/izquierda `(-0.2049,0.9000,0.3375)` y Hunter derecha `(-0.2745,1.8000,-0.4413)`/izquierda `(-0.1783,1.8000,0.4417)`. La separación lateral corresponde al eje Z glTF, no a los offsets ±X indicados en la auditoría de Muse; por eso usamos los orígenes reales de los nodos y no esos valores sugeridos.

Retiramos el modo `locomocionPatrulla` del shader; las ramas de locomoción de Clank y Blarg siguen intactas. Titan y los dos Sentinel no se modificaron. G/K/H se leen como teclas mantenidas y solo tienen efecto en proximidad: G asigna Grunt 1, K Grunt 2 y H Hunter. Al soltar o alejarse, una interpolación temporal devuelve la amplitud de piernas a la patrulla normal. Las capturas OpenGL de dos fases por NPC están en `Capturas/NPC_Locomocion/`. La prueba registra para Grunt 1 `+0.476/-0.476` y `-0.499/+0.499` rad; el Hunter alcanza `+0.496/-0.496` y `-0.500/+0.500` rad. El reloj y la posición continúan avanzando mientras se sostiene la tecla.

- Grunt 1: [paso A](Capturas/NPC_Locomocion/Grunt1_EjeZ_Paso_A.bmp), [paso B](Capturas/NPC_Locomocion/Grunt1_EjeZ_Paso_B.bmp).
- Hunter: [paso A](Capturas/NPC_Locomocion/Hunter_EjeZ_Paso_A.bmp), [paso B](Capturas/NPC_Locomocion/Hunter_EjeZ_Paso_B.bmp).

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

`C_ANIMACIONES_OK` separa el registro: puerta y caminata de Grunt 1/2 activada con G/K son las tres acciones básicas; Engineer y Sentinel 1/2 forman el grupo de movimiento continuo; Saturn V y holocrón usan keyframes. El test describe también la patrulla y la modulación con G/K/H, sin volver a contar esos NPC como objetos continuos.

## Resultado de las pruebas

- Compilación de `ProyectoEspacial.sln`, Visual Studio 2022, `Release | x64`: correcta; cero errores. Las advertencias corresponden a GLFW, miniaudio y la definición de `APIENTRY`.
- `ProyectoEspacial.exe --pruebas --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye contratos de regiones, carrera, P1, P2, Freeport, audio, cargadores GLTF, apoyos, rutas y capturas.
- `ProyectoEspacial.exe --pruebas-integrales --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye 5400 muestras de NPC/suelo/separación de P2, costuras del cielo, mezcla de audio y la verificación del Railjack.
- `--pruebas` resuelve los nodos por nombre y captura al Grunt 1 y al Hunter en dos fases de marcha; además verifica la asignación de G/K/H, proximidad, continuidad del reloj y retorno gradual al soltar. No se volvió a medir rendimiento en esta corrección; las cifras existentes son mediciones anteriores.
- La salida del Railjack registra base en terreno `baseY=3.5444`, base en vuelo `aireBaseY=22.5444`, centros `12.4244/31.4244`; entre las capturas con iluminación diurna controlada cambian 234843 canales RGB en `--pruebas-integrales` y 234526 en `--pruebas`.
- Las capturas de P1, P2 y Railjack se inspeccionaron visualmente. Son capturas automatizadas renderizadas por el ejecutable, no una grabación de partida manual.
- `--medir-integral` se ejecutó después de la corrección, en Intel UHD Graphics 620, con 120 muestras por región: P1 68.5005 FPS medios / p95 17.5629 ms; P2 45.3036 FPS / p95 24.4486 ms; Espacio 82.1008 FPS / p95 19.3006 ms. Es el escenario completo y no aísla el costo de las piernas.

## Integración y límites

Los cambios de esta revisión van a `main`. No tocamos ramas de compañeros ni recreamos `mejoras-teoria-final`. El cuarto recorrido académico que incluye estación y cuerpos celestes sigue dependiendo de la integración del equipo. La comprobación del juego completo con teclado y mouse en el equipo del estudiante queda fuera de estas pruebas automatizadas.
