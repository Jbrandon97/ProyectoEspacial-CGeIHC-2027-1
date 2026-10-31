# Proyecto Espacial — CGeIHC 2027-1

Proyecto final de Computación Gráfica e Interacción Humano-Computadora, Facultad de Ingeniería, UNAM. Base inicial: **Bloque D** en C++17 y OpenGL. Repositorio privado de colaboración académica; no incluye cambios del Bloque E.

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
