from pathlib import Path
import sys
import wave
import numpy as np

# Todas las frecuencias completan ciclos enteros: el bucle conserva valor y pendiente.
raiz = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
sr, duracion = 44100, 48
n = sr * duracion
t = np.arange(n) / sr
salida = np.zeros((n, 2))
for j, nota in enumerate([36, 43, 50, 55, 60, 62, 67]):
    f = round(440 * 2 ** ((nota - 69) / 12) * duracion) / duracion
    envolvente = .55 + .25 * np.cos(2 * np.pi * t / duracion + j * .8)
    modulacion = .08 * np.sin(2 * np.pi * (j % 3 + 1) * t / duracion)
    for canal in range(2):
        fase = 2 * np.pi * f * t + modulacion + canal * .09 * (j + 1)
        salida[:, canal] += .030 * envolvente * (np.sin(fase) + .16 * np.sin(2 * fase))
        brillo = (.5 + .5 * np.cos(2 * np.pi * t / duracion + j)) ** 6
        salida[:, canal] += .002 * brillo * np.sin(2 * np.pi * f * 4 * t + canal * .2)
salida -= salida.mean(axis=0)
pcm = np.round(salida * 32767).astype("<i2")
ruta = raiz / "Recursos/Audio/Horizonte_espacial.wav"
ruta.parent.mkdir(parents=True, exist_ok=True)
with wave.open(str(ruta), "wb") as f:
    f.setnchannels(2)
    f.setsampwidth(2)
    f.setframerate(sr)
    f.writeframes(pcm.tobytes())
print("Horizonte espacial", "segundos", duracion, "pico", float(np.max(np.abs(salida))),
      "RMS", float(np.sqrt(np.mean(salida ** 2))), "salto_union_PCM", int(np.max(np.abs(pcm[0].astype(int) - pcm[-1].astype(int)))))
