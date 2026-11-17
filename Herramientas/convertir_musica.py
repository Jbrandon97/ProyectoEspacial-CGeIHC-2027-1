"""Convierte las tres pistas aprobadas; los originales permanecen intactos."""
from pathlib import Path
import hashlib,json
import numpy as np
import soundfile as sf
base=Path(__file__).resolve().parents[1]/'Recursos/Audio'
registros=[]
for original,salida in [('Exploration Theme.ogg','Exploration_Theme.wav'),('airy.ogg','Airy.wav'),('our_expanse_2_-_chill_-_loop.ogg','Our_Expanse_Chill.wav')]:
 p=base/'Originales'/original;x,sr=sf.read(p,dtype='float32',always_2d=True)
 rms=float(np.sqrt(np.mean(x*x)));pico=float(np.max(np.abs(x)))
 ganancia=min(.07943/max(rms,1e-8),.8/max(pico,1e-8));x*=ganancia
 # Fundir la cola con el comienzo y desplazar el inicio evita un salto en la union.
 n=int(.05*sr);u=np.linspace(0,1,n,dtype=np.float32)[:,None]
 y=np.concatenate((x[n:-n],x[-n:]*(1-u)+x[:n]*u))
 sf.write(base/salida,y,sr,subtype='PCM_16')
 registros.append(dict(original=original,salida=salida,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),frecuencia=sr,canales=x.shape[1],segundos=len(y)/sr,ganancia=ganancia,rms=float(np.sqrt(np.mean(y*y))),pico=float(np.max(np.abs(y))),fundido_bucle_ms=50))
(base/'Conversion_musica.json').write_text(json.dumps(registros,indent=2),encoding='utf-8')
print('MUSICA_CONVERTIDA',len(registros))
