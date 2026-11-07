# Proyecto Espacial — Planeta 1 Halo / Titanfall

Proyecto de Computación Gráfica e IHC, FI UNAM, semestre 2027-1. Esta versión continúa el Bloque E y la integración local del Saturn V; incorpora la terminación de Planeta 1. Conservamos la arquitectura C++17/OpenGL/GLFW. Espacio está integrado en una rama propia; Planeta 2 conserva la base provisional y espera la integración del trabajo remoto de sus integrantes.

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

## Organización del equipo

- `src/Regiones/Planeta1/`: rama `planeta1--BRAND-ON`.
- `src/Regiones/Planeta2/`: ramas `planeta2-integrante1` y `planeta2-integrante2`.
- `src/Regiones/Espacio/`: rama `espacio-DAVID`.
- `main`: integración mediante revisión del equipo.
- `src/Mundo`, `src/Camaras`, `src/Nucleo`, `src/Graficos`, `src/Audio`: interfaces y servicios compartidos; coordinar cambios.
- `Recursos`, `Dependencias`, `Herramientas`: recursos relativos, bibliotecas y preparación reproducible.

Rama de trabajo: `espacio-DAVID`, carpeta independiente. Incorporamos la base funcional publicada conservando el historial; el remoto anterior de David sólo contenía el módulo provisional. Los cambios nuevos están en Espacio y sus interfaces de entrada, cámara y audio. Planeta 1 conserva su base y Planeta 2 no se modifica.

En Espacio: F3 piloto, V alterna primera persona/exterior, F4 observación de Gateway. W acelera y S frena; A/D giran; mouse orienta la nave horizontal y verticalmente. Q/Z conservan ascenso/descenso, C recuperación y Enter visita contextual al planeta cercano. F4 conserva órbita con mouse/flechas; los mandos de vuelo sólo se aplican en F3. 1–3 mantienen el cambio de región.

El piloto y la nave comparten posición y orientación. Los giros se amortiguan, velocidad limitada a 40 u/s y colisiones en subpasos. El modelo Banshee no contiene una cabina interior: en primera persona ocultamos su exterior para evitar ver polígonos desde dentro, sin inventar otra nave ni una cámara libre. La cámara sigue fija al piloto.

`Horizonte_espacial.wav` es una composición ambiental original de 48 s con frecuencias y envolventes periódicas. Entra y sale mediante fundidos; la música de Planeta 1 conserva su recurso. El mezclador existente maneja ambas pistas por región. Generación: `Herramientas/generar_audio_espacio.py` con NumPy.

La posición, orientación, cámara y sistemas regionales se conservan durante la ejecución. No hay guardado al cerrar. La entrega `Proyecto_Espacial_Final_1.zip` corresponde exclusivamente a la rama `planeta1--BRAND-ON`, por lo que estas mejoras nuevas se prueban y desarrollan desde esta rama de David.

## Atribuciones

Conservamos las atribuciones de recursos Halo/Titanfall y Saturn V en `Recursos/Modelos`, la licencia CC0 de Kenney en `Recursos/Cielos` y las licencias de bibliotecas en `Dependencias`. BT-7274: Palurdas Arts, licencia CC-BY-4.0 declarada en el archivo; fuente y adaptación en `Recursos/Modelos/BT7274/ATRIBUCION.txt`. Los WAV se sintetizan para el proyecto y no contienen música comercial. La revisión académica privada no cambia los derechos de recursos de terceros.

Gateway: recurso oficial NASA/JSC/Gateway Program Office; procedencia y adaptación en `Recursos/Modelos/Gateway/PROCEDENCIA.txt`. Texturas planetarias y cielo espacial: síntesis propia, no fotografías científicas.
