# Proyecto Espacial — revisión integral

Computación Gráfica e IHC, FI UNAM, 2027-1. C++17, OpenGL 3.3, GLFW y miniaudio. Conservamos Halo/Titanfall, Ratchet & Clank/Warframe y Freeport. Esta revisión es local: no está publicada en GitHub.

## Ejecutar y compilar

Extraer el ZIP completo y abrir **ProyectoEspacial.exe**. Mantener **Recursos** junto al EXE; no ejecutar dentro del ZIP. Incluimos el PDB correspondiente. Se necesita Windows x64 y un controlador OpenGL 3.3 con dispositivo de reproducción de audio disponible.

Para compilar, abrir **ProyectoEspacial.sln** en Visual Studio 2022 con Desarrollo para el escritorio con C++, MSVC v143 y Windows SDK. Seleccionar **Release | x64** y ProyectoEspacial como proyecto de inicio. Las dependencias están incluidas; no se requieren descargas ni DLL adicionales de terceros. La compilación copia Recursos a `bin/x64/Release`.

## Controles principales

| Contexto | Controles |
|---|---|
| General | 1/2/3 regiones; Esc salir; Tab liberar cursor |
| Audio | M silencio; N selecciona general/música/ambiente/efectos; −/+ ajustan el canal seleccionado |
| Planetas | WASD movimiento; Shift correr; mouse cámara; F1 exploración; F3 vista existente |
| Planeta 1 | X abordar/descender; R reiniciar carrera; C recuperar con penalización; E hangar; P reflector; F energía corporal |
| Interacciones de P1 | G Grunt; K inspección; H Hunter; B caminar BT; J saludo BT; L lanzar; T reponer; O observar/regresar. Las acciones de personajes, hangar y cohete requieren proximidad |
| Planeta 2 | F energía corporal; P reflector del recinto; NPC, Railjack y Aphelion tienen ciclos propios |
| Espacio, F3 pilotaje | W/S acelerar/frenar; A/D girar; mouse orientación; Q/Z subir/bajar; Shift boost; C recuperar; V alterna exterior/cabina; Enter portal cercano |
| Observación espacial | F4 órbita; F5 estación; F6 asteroides; F3 volver a pilotaje; U/I focos independientes |

La primera entrada a Espacio inicia en tercera persona. Al regresar se conserva la cámara elegida. F2 conserva la cámara de carrera y requiere abordar. No se reasignaron 1–3, F1–F4 ni C. El HUD muestra controles según región y modo; el listado detallado está en `Evidencias/Controles.csv`.

## Comprobaciones y evidencia

- `ProyectoEspacial.exe --pruebas --evidencias carpeta`: regresión de las tres regiones y capturas reales.
- `--pruebas --sin-capturas`: misma regresión sin guardar imágenes (las comprobaciones de píxeles siguen ejecutándose).
- `--pruebas-integrales --evidencias carpeta`: comprobaciones específicas de esta revisión y mezcla PCM de audio.
- `--medir-integral`: medición de fotogramas con simulación, HUD y audio en las tres regiones.
- `--medir-espacio`: medición de dibujo en tres vistas fijas, comparable al bloque anterior.

Diagnóstico, matriz de rúbrica, cambios, límites y demostración manual: **Evidencias/Revision_Integral.md**. La presentación **Evidencias/Revision_visual.html** reúne capturas y enlaces. **Evidencias/Audio_Tecnico.md** contiene la evidencia para la futura investigación; no es el reporte académico final.

## Organización y recursos

`src/Regiones` conserva los módulos de cada región; `src/Audio`, `src/Graficos`, `src/Camaras` y `src/Nucleo` contienen servicios compartidos. `Recursos` y `Dependencias` conservan sus licencias y procedencias. No atribuimos modelos de terceros ni cambios actuales a integrantes concretos.

miniaudio 0.11.23 se compila una sola vez desde `src/Audio/Audio.cpp`; no queda un motor waveOut propio. Música y ambientes son síntesis original: no incorporamos música comercial. Para regenerar las revisiones de Espacio y Planeta 2, ejecutar `Herramientas/generar_audio_integral.py` al final de los scripts de audio previos. Python y NumPy sólo son necesarios para esa regeneración, no para jugar.

Las posiciones, cámaras y relojes se conservan durante la sesión; no se guardan al cerrar. Antes de integrar estos cambios en las ramas del equipo, revisar el inventario y las pruebas de `Evidencias`. Esta entrega no hace commits ni modifica `main`.
