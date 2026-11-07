# Informe técnico de integración final

## Base y comparación

Usamos el archivo proporcionado `PROYECTO_ VERSION FINAL.zip`, cuyo directorio interno se llama ProyectoEspacial_Integrado_v2. Su CRC se verificó antes de extraer. No tratamos sus notas como pruebas de ejecución.

Referencias comparadas: Planeta 1 45cf895, Espacio a868a6c, Pedro ae59cd5, Miguel 7517b62 y main 716a051. Creamos integracion-final desde Espacio e incorporamos Pedro mediante merge normal, conservando sus autores. El ZIP aportó la distribución compacta de Planeta 2, Engineer compartido, NPC, Aphelion, apertura de Railjack, cielo y audio regional, y cambios de tráfico e iluminación espacial. Planeta 1 conservaba el código funcional publicado.

Pedro es la fuente principal de TODO Planeta 2. Comparamos los cuatro commits exclusivos de Miguel (eb3a203, b4b1714, cb09e10 y 7517b62). Railjack contiene los mismos 20 nodos, 20 mallas y 20 animaciones; Orokin, 4 nodos, 3 mallas y 2 animaciones. Sus JSON, excluyendo la lista de imágenes, coinciden exactamente entre ambas ramas. También coinciden sus buffers binarios:

- Railjack: 625a05ab729fef34a88acdb16d5eb2a51ed1903bab17fa177317e15e01b8156a.
- Orokin: ff1aa5214e785fc7127239f6290752d5bf008713267dffc5f7330760e30097bd.

Pedro ya convirtió las texturas a PNG con rutas válidas. Miguel mantiene fuentes Blender, STL y otras exportaciones, pero su código de Ratchet y terreno es anterior al de Pedro. No encontramos una funcionalidad adicional que deba incorporarse. Retiramos de la rama activa el merge redundante que se había preparado antes de la aclaración; su árbol era idéntico al anterior. Conservamos las ramas de todos los integrantes, el respaldo local y el historial de Miguel, sin cherry-picks ni doble carga de modelos. El conflicto provisional de Orokin se resolvió conservando la versión PNG; ya no existe un merge de Miguel en la integración activa.

## Correcciones concretas

| Archivo / función | Cambio y motivo |
| --- | --- |
| src/Regiones/Planeta2/SectorRatchet/SectorRatchet.cpp, cargar | Calculamos los apoyos de los seis edificios con los vértices inferiores reales, transformados por el modelo, y la altura del terreno. Conservamos X/Z, escala y colisiones. |
| SectorRatchet.h, huecosBase/centrosEdificios/luzEdificio | Exponemos datos de comprobación y reutilizamos los apoyos calculados al cargar, sin recorrer mallas en cada fotograma. |
| src/Regiones/Planeta2/Planeta2.cpp, ambiente | Seis luces a 6 unidades sobre el terreno, por delante de la fachada +Z, intensidad 3.5 por factor nocturno y atenuación (1,.09,.018). Reutilizamos las 16 posiciones del sistema existente. |
| Planeta2.h, ratchet/reloj | Acceso de lectura para pruebas de apoyos y persistencia. |
| src/Nucleo/Aplicacion.cpp, entrada y pruebas | Restauramos F1–F4 que el ZIP había retirado; conservamos 1–3, C y la selección contextual. Añadimos la comprobación de Planeta 2. |
| src/Pruebas/PruebasE.cpp, probarEspacio/probarPlaneta2 | Recuperamos pruebas de vuelo y audio; verificamos seis bases, seis colisiones, luces, ciclo de 120 s y reloj regional, con capturas OpenGL. |
| src/Regiones/Planeta2/SectorWarframe/SectorWarframe.cpp | Aclaramos que los modelos Warframe ya se dibujan en la distribución conjunta. Sin duplicarlos. |
| src/Regiones/Planeta1/SistemasPlaneta1.cpp | Una aclaración de comentario, sin cambio de comportamiento. |
| README.md | Estado integrado, compilación, controles reales y responsabilidades. |

Para cada edificio calculamos el mínimo de alturaTerreno(x,z) menos altura del vértice escalado, entre vértices a menos de .05 unidades del mínimo de la malla; hundimos .12 unidades. Esto evita un desplazamiento global arbitrario. La comprobación numérica no reprodujo una flotación general en la aproximación anterior: corregimos el método de apoyo y verificamos el resultado, sin atribuirle un defecto inexistente en todos los modelos. Revisamos visualmente las seis bases.

Revisamos comentarios de los tres escenarios y los servicios compartidos; conservamos explicaciones de cálculos y avisos de terceros. No reformateamos masivamente ni reescribimos sistemas funcionales.

## Arquitectura y funciones

Aplicacion coordina entrada, reloj, regiones, cámaras y audio. GestorEscenarios conserva instancias y estado; MundoEspacial convierte coordenadas locales/globales. GestorCamaras mantiene exploración, carrera, vuelo y observación. ModeloGltf carga geometría, materiales y jerarquías con cgltf; Renderizador aplica normales, luz solar, puntos, foco y cielo regional. El cargador no reproduce automáticamente las animaciones glTF embebidas: Railjack se articula mediante transformaciones de nodos en SectorRatchet::dibujar.

Planeta1 conserva terreno, colonia, Covenant, Engineer, carrera de tres vueltas con dos rivales y penalización de dos segundos, hangar, NPC, BT y Saturn V. Sus módulos CircuitoCarreras, ControlCarrera y GestorCarrera separan geometría, reglas y simulación. SistemasPlaneta1 controla ciclo, mecanismos y NPC; KeyframesCohete mantiene la secuencia heredada del cohete; Titan controla el rig de BT.

Planeta2 es una sola región con dos interfaces de sector. El terreno y su mapa de alturas permanecen; SectorRatchet dibuja edificios, árboles, Railjack, Orokin, Warframe, Clank, Blarg y Aphelion. SectorWarframe queda como interfaz compatible, sin una segunda copia de los modelos. Las colisiones de edificios y pendiente se resuelven en ajustarMovimiento/obstaculo. Aphelion sigue una spline de 78 s; Railjack abre, asciende, desciende y cierra en 24 s. No modificamos esas secuencias.

Espacio conserva la nave pilotable, Gateway, luna, planetas, asteroides, satélite y tráfico. El movimiento usa deltaTime y colisiones en subpasos. La cámara de cabina oculta la malla exterior; no existe un interior modelado de la Banshee. El atraque es exterior, no un recorrido interior de estación.

GestorAudio mezcla música, ambiente y eventos, con fundidos regionales. Se cargan recursos una vez, sin reiniciar cada fotograma. Conservamos los WAV y los tres scripts de síntesis. Los ciclos solares de ambos planetas duran 120 s; los puntos nocturnos utilizan atenuación y los shaders admiten 16 luces. F es luz corporal puntual, no spotlight; P controla el reflector del hangar.

## Controles definitivos

| Modo | Controles |
| --- | --- |
| General | 1/2/3 regiones; F1–F4 cámaras compatibles; Tab liberar cursor; Esc salir; M silencio; -/+ volumen |
| Exploración | WASD caminar 15 u/s; Shift correr 30; mouse/flechas cámara; F luz corporal |
| Planeta 1 | X abordar/descender cerca de nave detenida; E hangar cercano; P reflector; G/K Grunts; H Hunter; B caminar BT; J saludo BT |
| Carrera | WASD conducción; R reinicio; C recuperación y penalización de 2 s |
| Cohete | L lanzamiento contextual; T solicitar reposición; O observación/regreso; reposición automática al finalizar su ciclo |
| Espacio | F3 piloto; V primera persona/exterior; W/S acelerar/frenar; A/D giro; mouse orientación; Q/Z altura; C recuperar; Enter visitar planeta cercano; F4 observación orbital |

## Requisitos y alcance de verificación

| Requisito | Estado sustentado |
| --- | --- |
| Tres regiones, modelos y avatar jerárquico | Carga y ejecución OpenGL verificadas; Engineer con 10 articulaciones y 3 cadenas. |
| Movimiento y carrera | Automatizado: 15/30, checkpoints, tres vueltas, dos rivales, rampas, HUD, recuperación y penalización. No sesión manual completa. |
| Sol, puntos, ciclo, skyboxes | Implementados y pruebas de ciclos; vistas reales día/noche. Costuras visibles del panorama P2 suministrado siguen pendientes de retoque. |
| Spotlight | Reflector P del hangar existente; F permanece luz puntual del Engineer. |
| Tres básicas por teclado | Hangar E y dos Grunts G/K, objetos distintos; mecanismos verificados por pruebas. |
| Tres continuas | Dos Sentinels y tercer Grunt; también existen patrullas espaciales y P2. No contamos el mismo objeto en dos categorías. |
| Dos keyframes diferentes de caminar | BT saludo J y Hunter vigilancia H: estados temporales interpolados presentes y repetición comprobada. Hunter es transformación rígida; aceptación formal corresponde al profesor. |
| Saturn V | Lanzamiento, separaciones, restos y reposición probados; deriva de P10, no se reclama como objeto nuevo evaluable según la exclusión de prácticas previas. |
| Cuatro recorridos de cámara | Exploración, carrera y vuelo presentes; observación de cohete y Espacio disponible por separado. El cuarto recorrido completo exige validación formal, no lo declaramos totalmente acreditado. |
| Audio | Dispositivo PCM 44100 estéreo abierto; mezcla, eventos, atenuación, cambios regionales y cierre comprobados. Calidad audible pendiente de escucha humana. |
| Persistencia | Posición, cámara y reloj durante cambios de región probados; no hay guardado al cerrar. |
| Entrega documental oficial | Este informe aporta evidencia técnica. Manuales, bitácoras, video y evaluación individual siguen siendo entregables posteriores del equipo. |

## Participación comprobable

La responsabilidad regional no equivale a la autoría de cada línea. Git registra a Brandon Campos en 4f6c113 (Planeta 1), 2ba803b (integración espacial) y a868a6c (vuelo/cabina/audio). Pedro Alberto Solis Espinosa aparece en ae59cd5 (tercer edificio), 93bc57d/6736650/db77f4f (avances de terreno/Engineer/edificios). Miguel Angel Asencio Morales figura en 46ccc38 (Railjack), 829277d (ubicación y Orokin) y f69d710 (NPC/torretas), ya presentes en Pedro; sus cuatro commits exclusivos permanecen en su rama. Luis David Pinto Santillan aparece en 49a0c19 (TerrenoArenoso). No asignamos horas ni atribuimos a David cambios que Git registra con otro autor. La autoría individual de cambios sin commit dentro del ZIP no puede determinarse.

## Pruebas realizadas antes del empaquetado

Compilación nativa MSBuild de Visual Studio 2022, Release x64, salida 0. Ejecutamos el EXE real con --pruebas, creando contexto OpenGL 3.3 en Intel UHD Graphics 620. La batería finalizó con CIERRE CORRECTO y salida 0. Incluye 5000 muestras de circuito, carrera simulada completa, accesos, mecanismos, tres ciclos del Saturn, BT/Hunter, nueve transiciones regionales, 36 intentos de cámara, vuelo equivalente a 30/120 pasos por segundo, cambios de música y seis apoyos P2.

Las capturas proceden del framebuffer real; inspeccionamos el montaje de día/noche y las seis bases. Son pruebas automatizadas con revisión visual, no una partida manual con teclado ni escucha humana. Las muestras de vistas P1 registraron aproximadamente 22–40 ms por muestra en esta máquina; no son una medición sostenida de FPS ni un benchmark completo de las tres regiones.

## Archivos incluidos y excluidos

Fuente: solución, dos proyectos VS, filtros, CMake, C++/cabeceras, dependencias realmente usadas, recursos, shaders, audio, scripts, licencias, README e informe. No incluimos .git, .vs, .user, obj, bin, ZIP previos, PDF/Word, cachés ni registros temporales. Los originales no usados de Miguel permanecen en su rama y en el respaldo local, no se importan duplicados. Conservamos los recursos heredados de Pedro para no retirar posibles fuentes de trabajo del equipo.

Ejecutable: EXE y PDB Release correspondientes, Recursos completos y README de apertura. GLFW/GLAD se enlazan estáticamente y Release usa /MT. No se requiere Assimp: el código usa cgltf. Las DLL de Windows y un controlador OpenGL 3.3 son requisitos del sistema. No se cambió el estándar ni se añadió una dependencia ajena a la base.

La integración queda local, preparada para revisión. No hicimos push ni merge a main. Las ramas individuales permanecen conservadas.

## Cierre de portabilidad y distribución

Extraímos ambos ZIP en carpetas nuevas y verificamos CRC y SHA-256 de cada archivo contra su origen. Comprobamos todas las referencias externas de los glTF y la ausencia de rutas personales absolutas en código/configuración. El ZIP de fuentes contiene 927 archivos; la distribución ejecutable contiene 288.

Recompilamos la solución extraída con MSBuild de VS2022 en Release x64: salida 0. Ejecutamos tanto el paquete independiente como el EXE recompilado desde el directorio de usuario, fuera de sus carpetas: ambos finalizaron con salida 0. Las tres ejecuciones (trabajo, distribución y fuente recompilada) completaron la batería de las tres regiones. No fue necesario copiar archivos desde el proyecto de desarrollo a las carpetas extraídas.

Extracto real de resultados, presente en ambas ejecuciones independientes:

```
VUELO_OK mouse coherente, matriz alineada, 30/120 FPS equivalentes, cabina anclada, cambio exterior
MUSICA_ESPACIO_OK tres transiciones, sin reinicios, fundido y exclusividad regional
ESPACIO_OK navegacion avance giro altura, colision Gateway, trafico40s, persistencia, camaras, portal Planeta1
PLANETA2_OK seis apoyos geometricos sin hueco, colisiones, seis luces de fachada, ciclo120s y persistencia
CIERRE CORRECTO
```

También revisamos vistas reales del conjunto de Planeta 1, Gateway y los cuerpos celestes. No declaramos una sesión manual ni escucha humana. No repetimos Debug ni CTest en este cierre: la verificación ejecutada fue Release x64 y la batería nativa --pruebas.

El EXE entregado importa únicamente WINMM, KERNEL32, GDI32, USER32 y SHELL32. Verificamos que su registro CodeView y el PDB comparten GUID y edad; los hashes extraídos coinciden con el par original:

- EXE SHA-256: 096e33dbdfa2a598c9890d1759985a5d20df923a24b4c107a55da192ae2236b1.
- PDB SHA-256: faecbcfea29b8bfdaefe1e7f218d872da34e63fe7e49558ec8e10161cd10ad53.
- GUID: b19a002a-9d78-4c81-a353-9eeff8b0975e; edad: 2.

main permanece en 716a051. Pedro, Planeta 1 y Espacio son ancestros de integracion-final. Miguel se conserva como rama independiente y sus aportaciones ya incorporadas por Pedro mantienen sus commits originales. No publicamos cambios remotos.
