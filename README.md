# Proyecto Espacial — revisión integral

Computación Gráfica e IHC, FI UNAM, 2027-1. C++17, OpenGL 3.3, GLFW y miniaudio. Conservamos Halo/Titanfall, Ratchet & Clank/Warframe y Freeport. Este cierre reúne las correcciones de código y sus evidencias de prueba.

## Ejecutar y compilar

Para jugar, extrae **ProyectoEspacial_Release_Final.zip** y abre **ProyectoEspacial.exe** con la carpeta **Recursos** a su lado. El paquete incluye el PDB de esa compilación. Se necesita Windows x64, un controlador OpenGL 3.3 y un dispositivo de audio. El código de trabajo y la solución completa están en `main`.

Para compilar, abrir **ProyectoEspacial.sln** en Visual Studio 2022 con Desarrollo para el escritorio con C++, MSVC v143 y Windows SDK. Seleccionar **Release | x64** y ProyectoEspacial como proyecto de inicio. Las dependencias están incluidas; no se requieren descargas ni DLL adicionales de terceros. La compilación copia Recursos a `bin/x64/Release`.

## Controles principales

| Contexto | Controles |
|---|---|
| General | 1/2/3 regiones; Esc salir; Tab liberar cursor |
| Audio | M silencio; N selecciona general/música/ambiente/efectos; −/+ ajustan el canal seleccionado |
| Planetas | WASD movimiento; Shift correr; mouse cámara; F1 exploración; F3 vista existente |
| Planeta 1 | X abordar/descender; R reiniciar carrera; C recuperar con penalización; E hangar; P reflector; I reflector Covenant; F energía corporal |
| Interacciones de P1 | Pulsa una vez G: Grunt 1; K: Grunt 2; H: Hunter. Cada uno camina hacia adelante, gira, vuelve por la misma ruta y queda en su base dentro de un ciclo de 14 s; requiere estar a menos de 18 unidades. Grunt 3 permanece quieto. B caminar BT; J saludo BT; L lanzar; T reponer; Y holocrón; O observar/regresar |
| Planeta 2 | F energía corporal, P foco de plaza, I foco Orokin, U habilita o deshabilita los cuatro reflectores nocturnos del Railjack, F3 plaza, F4 Warframe |
| Espacio, F3 pilotaje | W/S acelerar/frenar; A/D girar; mouse orientación; Q/Z subir/bajar; Shift boost; C recuperar; V alterna exterior/cabina; Enter portal cercano |
| Observación espacial | F4 órbita; F5 estación; F6 asteroides; F3 volver a pilotaje; U/I focos independientes |

La primera entrada a Espacio inicia en tercera persona. Al regresar se conserva la cámara elegida. F2 conserva la cámara de carrera y requiere abordar. No se reasignaron 1–3, F1–F4 ni C. El HUD muestra controles según región y modo; el listado detallado está en `Revision_Cierre/Controles_Cierre.md`.

## Comprobaciones y evidencia

- `ProyectoEspacial.exe --pruebas --evidencias carpeta`: regresión de las tres regiones y capturas reales.
- `--pruebas --sin-capturas`: misma regresión sin guardar imágenes (las comprobaciones de píxeles siguen ejecutándose).
- `--pruebas-integrales --evidencias carpeta`: comprobaciones específicas, incluido Railjack en reposo/vuelo con luz fija, continuidad del cielo y mezcla PCM de audio.
- `--pruebas-planeta2 --evidencias carpeta`: patrullas, caminos, apoyo de edificios, torre, Aphelion y reflectores del puerto con capturas de día y noche.
- `--medir-integral`: medición de fotogramas con simulación, HUD y audio en las tres regiones.
- `--medir-espacio`: medición de dibujo en tres vistas fijas, comparable al bloque anterior.

Diagnóstico, matriz de rúbrica, cambios, límites y demostración manual: **Revision_Cierre/Revision_Integral.md**. La presentación **Revision_Cierre/Revision_visual.html** reúne capturas y enlaces. **Revision_Cierre/Audio_Tecnico.md** contiene la evidencia de audio; no es el reporte académico final.

Release x64 pasa `--pruebas` y `--pruebas-integrales` con `CIERRE CORRECTO`. G/K/H son pulsaciones únicas que inician un ciclo independiente de 14 s con preparación, ida, giro, regreso a base y reposo; las piernas quedan inmóviles fuera de los tramos caminados. La rotación ahora alinea el frente local `-X` del modelo con la ruta; las capturas del mismo Grunt 1 en dos fases de marcha y de regreso a su base están en **Revision_Cierre/Capturas/NPC_Locomocion**. Son renderizados OpenGL automatizados, no una sesión manual. Los registros actuales están en `Revision_Cierre/Pruebas_Salida_2026-11-20.txt` y `Revision_Cierre/Pruebas_Integrales_Salida_2026-11-20.txt`.

## Organización y recursos

`src/Regiones` conserva los módulos de cada región; `src/Audio`, `src/Graficos`, `src/Camaras` y `src/Nucleo` contienen servicios compartidos. `Recursos` y `Dependencias` conservan sus licencias y procedencias. No atribuimos modelos de terceros ni cambios actuales a integrantes concretos.

miniaudio 0.11.23 se compila una sola vez desde `src/Audio/Audio.cpp`; no queda un motor waveOut propio. Las pistas musicales entregadas son CC0, sus originales y atribuciones se conservan en `Recursos/Audio/Originales` y `Recursos/Audio/Musica_licencias.md`; no incorporamos música comercial de Halo ni Titanfall. Para regenerar las conversiones y ambientes, ejecutar `Herramientas/convertir_musica.py` y `Herramientas/generar_audio_integral.py`. Python y NumPy sólo son necesarios para esa regeneración, no para jugar.

Las posiciones, cámaras y relojes se conservan durante la sesión; no se guardan al cerrar. El inventario, las pruebas y los límites de verificación de este cierre están en `Revision_Cierre`.

En Planeta 2 los caminos conectan la plaza de Qwark, Kerwan, la torre Orokin, el corral y el puerto Railjack. Cuatro muñecos recorren el perímetro en sentido horario. Las balizas y los reflectores del puerto siguen el ciclo de 120 segundos. La procedencia de los modelos añadidos está en `Recursos/Modelos/P2_Kerwan/Procedencia.md`.
