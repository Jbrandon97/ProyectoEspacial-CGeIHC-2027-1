# Música proporcionada para el cierre

Los OGG de `Originales` conservan los bytes de los adjuntos. No son composiciones del equipo.

- **Exploration Theme — Cleyton Kauffman**, CC0. Licencia incluida en el ZIP y conservada como `Originales/Exploration_readme.txt`. Autor: https://soundcloud.com/cleytonkauffman
- **Airy — SRG774**, Dark Sci-Fi Audio Pack, CC0. Fuente del paquete `ogg.zip`: https://opengameart.org/content/dark-sci-fi-audio-pack
- **Our Expanse, versión 2 chill loop — Bobjt**, CC0. Fuente del paquete `version2-files.zip`: https://opengameart.org/content/our-expanse

Las páginas de procedencia se consultaron el 13 de noviembre de 2026. La ficha de Airy recomienda un fundido breve para unir el bucle. La de Our Expanse identifica explícitamente la versión 2 como chill.

`Herramientas/convertir_musica.py` conserva estéreo y frecuencia originales, convierte a PCM16 WAV, ajusta RMS a aproximadamente −22 dBFS sin exceder pico 0.8, y une 50 ms de cola/comienzo. No se comprime de nuevo con pérdida. `Conversion_musica.json` registra hashes, niveles y duración. Las conversiones requieren NumPy y soundfile; la ejecución del juego solo necesita miniaudio incluido.
