# Proyecto Espacial — integración final

Computación Gráfica e IHC, FI UNAM, semestre 2027-1. Integramos Halo/Titanfall, Ratchet & Clank/Warframe y Espacio en C++17, OpenGL 3.3 y GLFW.

## Ejecutar 

Descargar `ProyectoEspacial_Ejecutable_Final.zip` desde [GitHub Releases — v1.0-final](https://github.com/Jbrandon97/ProyectoEspacial-CGeIHC-2027-1/releases/tag/v1.0-final). Extraer el ZIP completo y abrir `ProyectoEspacial.exe` con doble clic dentro de `Entrega_Ejecutable`. Mantener `Recursos` junto al EXE. No se necesita un archivo CMD ni compilar para jugar.

## Abrir, compilar y ejecutar

1. Abrir `ProyectoEspacial.sln` con Visual Studio 2022, carga de trabajo Desarrollo para el escritorio con C++, MSVC v143 y Windows SDK.
2. Seleccionar **Release | x64**, proyecto de inicio **ProyectoEspacial**, compilar y ejecutar.
3. La compilación genera `bin/x64/Release/ProyectoEspacial.exe` y copia `Recursos` junto a él. Las dependencias están incluidas.
4. En la carpeta preparada para revisión también puede abrirse directamente `ProyectoEspacial.exe`, situado junto a `Recursos` para evitar duplicarlos.
5. Pruebas automatizadas: `ProyectoEspacial.exe --pruebas --evidencias carpeta_destino`.

Para verificar sin generar capturas: `ProyectoEspacial.exe --pruebas --sin-capturas`. Conservamos los tests porque forman parte de la solución. No se necesita Python para jugar; las herramientas de síntesis y conversión sólo se usan para regenerar recursos.

GLFW y GLAD se compilan como biblioteca estática; GLM es de cabeceras. No hacen falta DLL de terceros ni redistribuible de MSVC: Release utiliza /MT. Windows y un controlador compatible con OpenGL 3.3 son necesarios. Los filtros organizan los archivos en Visual Studio; el proyecto ejecutable es el proyecto de inicio. El depurador usa la carpeta del EXE y el PDB correspondiente se genera en cada compilación.

## Controles

| Tecla | Acción |
| --- | --- |
| 1 / 2 / 3 | Planeta 1 / Planeta 2 / Espacio |
| F1–F4 | Cámaras originales, según compatibilidad de región |
| WASD / Shift | Caminar 15 / correr 30 unidades/s, relativo a cámara |
| Mouse / flechas / Tab | Cámara / órbita / liberar cursor |
| X | Abordar o descender de Banshee, en proximidad y detenida |
| W/S/A/D en carrera | Acelerar / frenar / dirigir |
| R / C | Reiniciar carrera / recuperar con penalización de 2 s |
| E / P | Hangar cercano / reflector fijo del hangar |
| F | Energía corporal cian omnidireccional del Engineer |
| G / K / H | Reacción de Grunt 1 / inspección de Grunt 2 / vigilancia de Hunter; en proximidad |
| B / J | Caminata / saludo articulado del BT-7274, en proximidad y sin otra animación suya activa |
| L / T / O | Lanzar / solicitar reposición / observar o regresar |
| M / - / + / Esc | Silencio / volumen / salir |

El cohete conserva dos separaciones principales, a los 7 y 11 s. S-IVB y cápsula siguen juntas hacia el cielo; no orbitan. Los restos permanecen apoyados hasta reiniciar y la reposición automática empieza a los 70 s; la compuerta/elevador tarda 8,5 s. L funciona a 8–65 unidades de la plataforma; T a 9–90. Si T se pulsa antes de finalizar, espera al cierre del vuelo. No se transporta al avatar.

## Estructura e integración

- `src/Regiones/Planeta1/`: Halo/Titanfall, base de `planeta1--BRAND-ON`.
- `src/Regiones/Planeta2/`: región única Ratchet & Clank/Warframe; fuente principal `planeta2--PEDRO`. La rama `planeta2--MIGUEL` conserva el historial y los originales del colaborador, sin otro merge redundante.
- `src/Regiones/Espacio/`: Gateway, navegación, cuerpos celestes y tráfico; base `espacio-DAVID` y revisión del ZIP.
- `src/Mundo`, `src/Camaras`, `src/Nucleo`, `src/Graficos`, `src/Audio`: servicios compartidos; coordinar cambios entre integrantes.
- `Recursos`, `Dependencias`, `Herramientas`: recursos relativos, bibliotecas y scripts de preparación/audio.
- `integracion-final`: base validada e incorporada a `main` mediante merge, conservando los historiales.

Conservamos las interfaces de ambos sectores de Planeta 2; su distribución conjunta se dibuja una sola vez desde SectorRatchet. No duplicar los modelos Warframe en SectorWarframe. Mantener las rutas relativas, revisar cambios compartidos y probar las tres regiones antes de integrar.

En Espacio: F3 piloto, V primera persona/exterior, F4 observación; W/S aceleración/freno, A/D giro, mouse orientación, Q/Z ascenso/descenso, C recuperación y Enter visita contextual. F4 permite órbita con mouse/flechas. Las teclas 1–3 y F1–F4 se conservan; la selección automática de cámara sigue activa según el modo.

La posición, orientación, cámara y reloj regional se conservan durante la sesión; no se guardan al cerrar. Los WAV originales se regeneran con los scripts `Herramientas/generar_audio*.py` (Python y NumPy), innecesarios para jugar.

El cargador real es **cgltf**, acompañado de stb_image, GLM, GLAD y GLFW. Esta base no utiliza Assimp; no se añadió una biblioteca ni DLL que el código no necesita.

## Entrega y atribuciones

El paquete fuente permite compilar. `ProyectoEspacial_Ejecutable_Final.zip` contiene `Entrega_Ejecutable` con EXE, PDB y Recursos; extraer la carpeta completa y abrir el EXE. No ejecutar dentro del ZIP ni separar el EXE de sus recursos.

Las procedencias y licencias permanecen en Recursos y Dependencias. Los modelos de terceros no se presentan como propios; la finalidad académica no cambia sus derechos. La revisión y las limitaciones verificadas están en `Informe_Tecnico_Integracion.md`.
