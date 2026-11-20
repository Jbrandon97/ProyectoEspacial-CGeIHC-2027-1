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

Grunt 1, Grunt 2 y Hunter quedan en sus bases hasta una pulsación cercana (menos de 18 unidades): G activa Grunt 1, K Grunt 2 y H Hunter. Cada NPC tiene reloj independiente; solo avanza durante su ciclo de 14 s. Tras 0.6 s de preparación, el cuerpo completo avanza 5 s por el arco del circuito, se detiene para girar y vuelve por el mismo arco hasta su base; al final gira al frente y se detiene. No repite círculos. Grunt 3 (índice 2) no se puede activar y queda quieto. El cuerpo mantiene altura con `TerrenoPlaneta1::altura` y su orientación sigue la dirección de desplazamiento.

La auditoría confirmó que los archivos anteriores de Grunt y Hunter tenían jerarquía plana, 0 skins y 0 animaciones. Las cotas y pivotes indicados por Muse no concordaban con las coordenadas que realmente usa el glTF. En Blender se detectaron las dos islas de pierna por conectividad de caras, se separaron y se emparentaron al torso sin cambiar la pose global. La comparación visual de reposo entre originales y reexportados no mostró diferencias ni costuras; UV y PNG permanecen iguales.

`ModeloGltf` conserva los nombres y orígenes de los nodos. En `Planeta1::cargar` se resuelven `Grunt_pierna_izquierda/derecha` y `Hunter_pierna_izquierda/derecha`. `Planeta1::dibujar` arma `T(pivote) * Rz(ángulo) * T(-pivote)`, sin cambiar pivotes ni nodos. Rz mueve las piernas hacia delante y atrás en el plano X-Y; Grunts usan 24 ciclos de paso por la vuelta de fase y Hunter 16. La amplitud es cero en reposo y durante los giros; se desvanece al inicio y final de los tramos caminados.

Los pivotes medidos en el glTF final son Grunt derecha `(-0.0280,0.9000,-0.1602)`/izquierda `(-0.2049,0.9000,0.3375)` y Hunter derecha `(-0.2745,1.8000,-0.4413)`/izquierda `(-0.1783,1.8000,0.4417)`. La separación lateral corresponde al eje Z glTF, no a los offsets ±X indicados en la auditoría de Muse; por eso usamos los orígenes reales de los nodos y no esos valores sugeridos.

La revisión visual final detectó que la orientación del cuerpo todavía estaba invertida: el frente real de estos modelos está en `-X` local, con `Z` lateral. Antes alineábamos `+X` con la tangente de la ruta, por lo que el personaje avanzaba mirando hacia atrás y la zancada parecía lateral. `SistemasPlaneta1::orientacionPatrulla` ahora usa `atan2(dz, -dx)`, de modo que la rotación alrededor de Y transforma el frente `-X` hacia el desplazamiento. Se conserva `Rz` para las piernas y no se modifican pivotes, nodos, amplitudes, fase ni temporizadores.

Retiramos el modo `locomocionPatrulla` del shader; las ramas de Clank y Blarg siguen intactas. Titán (B/J) y ambos Sentinel no se modificaron. `Aplicacion.cpp` entrega G/K/H como pulsaciones únicas; `Planeta1::controlar` inicia solo al personaje asignado, quieto y a menos de 18 unidades. La prueba registró al mismo Grunt 1 en `(-76.6076,152.989)` y `(-75.8295,152.904)`, con los pares de ángulos `+0.253655/-0.253655` y `-0.35/+0.35` rad. Al completar 14 s regresa a `(-77,153)` con amplitud cero.

- Grunt 1: [paso A](Capturas/NPC_Locomocion/Grunt1_Caminata_A.bmp), [paso B](Capturas/NPC_Locomocion/Grunt1_Caminata_B.bmp) y [misma base en reposo](Capturas/NPC_Locomocion/Grunt1_Base_Reposo.bmp).

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

`C_ANIMACIONES_OK` separa el registro: puerta y caminata de Grunt 1/2 activada con una pulsación de G/K son las tres acciones básicas; Engineer y Sentinel 1/2 forman el grupo de movimiento continuo; Saturn V y holocrón usan keyframes. El regreso de los NPC es adicional y no se cuenta dos veces.

## Resultado de las pruebas

- Compilación de `ProyectoEspacial.sln`, Visual Studio 2022, `Release | x64`: correcta; cero errores. Las advertencias corresponden a GLFW, miniaudio y la definición de `APIENTRY`.
- `ProyectoEspacial.exe --pruebas --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye contratos de regiones, carrera, P1, P2, Freeport, audio, cargadores GLTF, apoyos, rutas y capturas.
- `ProyectoEspacial.exe --pruebas-integrales --evidencias <carpeta>`: salida 0 y `CIERRE CORRECTO`. Incluye 5400 muestras de NPC/suelo/separación de P2, costuras del cielo, mezcla de audio y la verificación del Railjack.
- `--pruebas` resuelve los nodos por nombre y captura al mismo Grunt 1 en dos fases opuestas de marcha y quieto en su base. Comprueba reposo sin entrada, teclas G/K/H asignadas y proximidad, no reinicio por pulsación durante el ciclo, relojes independientes, rumbo y desplazamiento de ida y vuelta, giros en los extremos y retorno al tiempo 14 s. Son pruebas automatizadas con renderizado OpenGL; no equivalen a una partida manual con teclado. No se volvió a medir rendimiento para esta corrección.
- La salida del Railjack registra base en terreno `baseY=3.5444`, base en vuelo `aireBaseY=22.5444`, centros `12.4244/31.4244`; entre las capturas con iluminación diurna controlada cambian 234843 canales RGB en `--pruebas-integrales` y 234526 en `--pruebas`.
- Las capturas de P1, P2 y Railjack se inspeccionaron visualmente. Son capturas automatizadas renderizadas por el ejecutable, no una grabación de partida manual.
- La última medición de `--medir-integral` disponible corresponde al cierre previo, en Intel UHD Graphics 620, con 120 muestras por región: P1 68.5005 FPS medios / p95 17.5629 ms; P2 45.3036 FPS / p95 24.4486 ms; Espacio 82.1008 FPS / p95 19.3006 ms. No volvimos a medir rendimiento después del ciclo de caminata actual; las cifras no representan un benchmark de esta corrección ni aíslan el costo de las piernas.

### Revalidación del rumbo glTF — 2026-11-20

- Release x64 compilado con MSBuild 17.13 / Visual Studio 2022; 0 errores.
- `ProyectoEspacial.exe --pruebas --evidencias <carpeta>`: código 0, `CIERRE CORRECTO`. La regresión verifica que el frente local `-X` siga el desplazamiento tanto al salir como al regresar; las demás comprobaciones de carrera, transiciones de región, P1, P2, Espacio y audio también concluyeron.
- `ProyectoEspacial.exe --pruebas-integrales --evidencias <carpeta>`: código 0, `CIERRE CORRECTO`.
- Capturas OpenGL generadas y revisadas: [marcha A](Capturas/NPC_Locomocion/Grunt1_Caminata_A.bmp), [marcha B](Capturas/NPC_Locomocion/Grunt1_Caminata_B.bmp) y [reposo en base](Capturas/NPC_Locomocion/Grunt1_Base_Reposo.bmp). Son pruebas automatizadas; no afirmamos haber probado manualmente el juego con teclado y mouse.
- No se midió rendimiento de nuevo en esta corrección. La medición histórica indicada arriba no representa este cambio.

## Integración y límites

Los cambios de esta revisión van a `main`. No tocamos ramas de compañeros ni recreamos `mejoras-teoria-final`. El cuarto recorrido académico que incluye estación y cuerpos celestes sigue dependiendo de la integración del equipo. La comprobación del juego completo con teclado y mouse en el equipo del estudiante queda fuera de estas pruebas automatizadas.
