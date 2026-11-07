"""Recrea la pista ambiental original de Planeta 2 (opcional, requiere NumPy)."""
from pathlib import Path
import wave
import numpy as np
rate, dur = 44100, 12.0
t = np.arange(int(rate * dur), dtype=np.float64) / rate
base = .20 * np.sin(2*np.pi*73.42*t) + .16 * np.sin(2*np.pi*110*t) + .09 * np.sin(2*np.pi*146.83*t)
mod = .70 + .30*np.sin(2*np.pi*.09*t)**2
fade = np.minimum(1, np.minimum(t, dur-t)/.5)
left = np.clip(base*mod*fade*(1+.1*np.sin(2*np.pi*.13*t))*5200,-32000,32000).astype('<i2')
right = np.clip(base*mod*fade*(1-.1*np.sin(2*np.pi*.13*t))*5200,-32000,32000).astype('<i2')
out = Path(__file__).resolve().parents[1]/'Recursos'/'Audio'/'Ambiente_Planeta2.wav'
with wave.open(str(out), 'wb') as w:
    w.setnchannels(2)
    w.setsampwidth(2)
    w.setframerate(rate)
    w.writeframes(np.column_stack([left,right]).ravel().tobytes())
print(out)
