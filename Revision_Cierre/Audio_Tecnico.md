# Audio técnico

El motor usa miniaudio 0.11.23 una sola vez en `src/Audio/Audio.cpp`. Las pistas se convierten a WAV PCM16 estéreo 44100 Hz y se cargan una vez; las mezclas no dependen del número de FPS. Se conservan los originales en `Recursos/Audio/Originales` y el registro de conversión en `Conversion_musica.json`.

- Planeta 1: `Exploration_Theme.wav`.
- Espacio: `Airy.wav`.
- Planeta 2: `Our_Expanse_Chill.wav`.
- Efectos existentes: WAV de hangar, carrera, cohete y ambiente.

Las procedencias CC0 y la conversión se documentan en `Recursos/Audio/Musica_licencias.md`. La prueba automatizada comprobó formato, RMS, canales y cierre sin dispositivo; la escucha subjetiva en el equipo de entrega sigue siendo una comprobación manual del estudiante.
