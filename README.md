# Proyecto Espacial — CGeIHC 2027-1

Proyecto final de Computación Gráfica e Interacción Humano-Computadora, Facultad de Ingeniería, UNAM. Base inicial: **Bloque D** en C++17 y OpenGL. Esta rama incorpora el **Bloque E de Planeta 1**; la integración con el equipo permanece pendiente.

## Abrir y compilar

1. Clonar el repositorio completo con una cuenta que tenga acceso.
2. En Windows, instalar Visual Studio 2022 con **Desarrollo para el escritorio con C++**, herramientas **MSVC v143** y Windows SDK 10/11.
3. Abrir `ProyectoEspacial.sln`, seleccionar **Release | x64** y establecer **ProyectoEspacial** como proyecto de inicio.
4. Compilar la solución y ejecutar. Se genera `bin/x64/Release/ProyectoEspacial.exe`; la compilación copia automáticamente `Recursos/` junto al ejecutable. No se distribuyen binarios compilados.

Las dependencias están incluidas; no hace falta descargar modelos ni configurar rutas absolutas. No se requiere Git LFS para esta base. CMake es una alternativa opcional mediante `CMakePresets.json`. El script `Herramientas/generar_audio.py` requiere Python y NumPy solo si se desea regenerar el audio; los WAV ya están incluidos.

## Estructura y responsabilidades

| Carpeta | Función |
| --- | --- |
| `src/Regiones/Planeta1/` | Planeta 1, responsabilidad de John |
| `src/Regiones/Planeta2/` | Planeta 2, trabajo coordinado entre dos integrantes |
| `src/Regiones/Espacio/` | Región Espacio, responsabilidad del tercer compañero |
| `src/Mundo/`, `src/Nucleo/` | Interfaces regionales, registro, ciclo de aplicación e integración |
| `src/Camaras/`, `src/Graficos/`, `src/Audio/`, `src/Personajes/` | Sistemas compartidos; coordinar sus cambios |
| `src/Pruebas/` | Comprobaciones existentes del proyecto |
| `Recursos/` | Modelos, texturas, shaders, cielos, audio y originales suministrados |
| `Dependencias/`, `Herramientas/` | Bibliotecas incluidas y generación de audio |

## Ramas

Todas las ramas parten del mismo commit inicial completo:

- `main`: base funcional e integración del equipo.
- `planeta1-john`: desarrollo de Planeta 1.
- `planeta2-integrante1`: primer compañero de Planeta 2.
- `planeta2-integrante2`: segundo compañero de Planeta 2.
- `espacio-integrante3`: compañero encargado de Espacio.

## Reglas de integración

Trabajamos en nuestra rama y proponemos cambios a `main` mediante pull request, con revisión de otro integrante y descripción de las pruebas realizadas. Evitamos subir compilaciones, ZIP o archivos personales y no forzamos cambios sobre las ramas de los demás. Los dos responsables de Planeta 2 acuerdan previamente qué archivos modifica cada uno.

Conservamos la estructura y el contrato `Escenario` de `src/Mundo/Escenario.h`: posiciones locales, eje Y hacia arriba, actualización con `deltaTime`, carga y liberación de recursos por región. `RegistroEscenarios` registra los tres módulos; `ContextoEscena` proporciona los servicios comunes. Las regiones no crean otra ventana ni otro ciclo principal. Los cambios en gestores, cámaras, renderizador, núcleo e interfaces se coordinan con todo el equipo. Al agregar fuentes, actualizamos el proyecto de Visual Studio y CMake de manera compatible.

**Pendientes prioritarios de integración:** persistencia independiente de cada región; conservación de posición del avatar, cámara y orientación al regresar; ciclos de día/noche que no se reinicien; y completar los cuatro recorridos de cámara exigidos por el profesor. Actualmente `GestorEscenarios::activar` carga/libera regiones, `Aplicacion::cambiarRegion` restablece avatar y cámara, y `Planeta1::cargar` reinicia sus sistemas. Estos puntos quedan identificados, sin modificar su funcionamiento en esta publicación.

## Recursos y atribuciones

Incluimos los recursos completos del Bloque D para el trabajo académico privado del equipo: Banshee, Ghost, Engineer, NPC, espadas, Saturn V, texturas, skyboxes y audio. Conservamos los archivos de licencia y procedencia existentes. Los recursos de terceros no son de nuestra autoría; el carácter privado del repositorio no se presenta como una licencia de redistribución. No cambiar la visibilidad ni redistribuir los recursos fuera del equipo sin revisar sus permisos.

Los skyboxes de Kenney conservan su licencia CC0 en `Recursos/Cielos/License.txt`. El audio sintetizado conserva su procedencia y generador. Las atribuciones de modelos están en `Recursos/Modelos/` y las licencias de bibliotecas, en `Dependencias/`.

## Bloque E — resumen de Planeta 1

- Saturn V: ascenso de 20 s, separación de dos etapas reales a los 7 y 11 s, llamas dobles y reposición mediante compuerta en 8,5 s. Se conservan el modelo y la plataforma originales junto a sus derivados. Si pulsamos T durante el vuelo, la reposición espera al final; si el cohete está preparado, primero desciende. No transporta al avatar.
- Engineer: escala 1,05, pose completamente inmóvil en reposo y articulaciones activas al caminar/correr. Velocidades 15/30, sin salto.
- Iluminación: ciclo 120 s conservado; mayor presencia local de las luces Sentinel y linterna más amplia que gira con el avatar.
- Audio: música/ambiente independientes, efectos condicionados y volumen de propulsión según distancia a su motor, suavizado por el mezclador existente. Sin bibliotecas nuevas.

### Controles conservados

| Teclas | Acción |
| --- | --- |
| 1 / 2 / 3 | Planeta 1 / Planeta 2 / Espacio, igual que en D |
| F1 / F2 / F3 / F4 | Exploración / carrera / vista previa / observación, según compatibilidad original; F4 permanece reservado a Espacio |
| WASD / Shift | Movimiento relativo a cámara / correr; en carrera W acelera, S frena, A/D dirigen |
| Mouse / flechas izquierda-derecha / Tab | Cámara / órbita / liberar o capturar cursor |
| X / R / C | Abordar o descender con nave detenida / reiniciar carrera / recuperar con penalización de 2 s |
| E / G | Hangar cercano / reacción del Grunt cercano |
| L / T / O | Lanzar / solicitar reposición / observar cohete y regresar |
| F / M / - / + | Linterna / silencio / volumen |
| Esc | Salir |

L requiere estar a 8–65 unidades de la plataforma; T, a 9–90 unidades, fuera del brocal. Las interacciones del cohete no se activan durante la carrera. O permite observar el evento, sin viaje interplanetario.

### Inventario de animaciones (sin doble conteo)

| Objeto | Categoría | Implementación y actualización |
| --- | --- | --- |
| Puerta del hangar | Básica por teclado | `Planeta1.cpp`: `controlar` con E, `actualizar` interpola altura |
| Saturn V | Básica por teclado | `SistemasPlaneta1`: L inicia y `actualizar` integra el tiempo; `Planeta1::dibujar` aplica posición y llama |
| Grunt 1 | Básica por teclado | G cercano; `SistemasPlaneta1::animacionNPC(0)` |
| Sentinel 1 | Continua | `SistemasPlaneta1::sentinel(0)` y `giroSentinel(0)` |
| Sentinel 2 | Continua | `SistemasPlaneta1::sentinel(1)` y `giroSentinel(1)` |
| Grunt 3 | Continua | `SistemasPlaneta1::animacionNPC(2)`, giro de vigilancia |
| Grunt 2 | Keyframes | `SistemasPlaneta1::animacionNPC(1)`: cinco estados temporales e interpolación |
| Hunter | Keyframes | `SistemasPlaneta1::animacionNPC(3)`: cinco estados temporales e interpolación |
| Engineer | Locomoción jerárquica, fuera del conteo de keyframes | `Avatar::mover`, `Engineer::animar` |

La separación del Saturn V también usa keyframes explícitos (`etapas`), pero no contamos nuevamente el cohete para completar las cantidades. La compuerta forma parte de su reposición. No se cuenta el satélite de Espacio como contenido de Planeta 1 ni se modifica.

### Validación y límites

Se compilaron los conjuntos de cambios en Visual Studio 2022 Release x64 y se ejecutó `--pruebas`: regresión de carrera (3 vueltas, 24 checkpoints, penalización), colisiones, modelos, iluminación, cámaras y nuevas pruebas E de separación, reposición a 30/60/144 pasos por segundo, inmovilidad y velocidades del avatar, eventos y atenuación de audio. Las pruebas generan sus evidencias fuera de la entrega.

Se inspeccionaron capturas reales de OpenGL: etapas separadas, compuerta y elevación, Engineer y luces. Las dos imágenes de reposo son idénticas. La revisión interactiva de ventana comprobó X, C, F, movimiento W, F3/F4 y 1/2/3; no equivale a una partida completa jugada manualmente. No se realizó evaluación auditiva humana: se verificaron el dispositivo, los buffers de audio, los eventos y los niveles calculados. Los cuatro recorridos completos, la persistencia entre regiones y el contenido de los compañeros siguen siendo asuntos de integración general.
