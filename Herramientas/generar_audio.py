from pathlib import Path
import numpy as np,wave,json
import sys
P=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parents[1];O=P/'Recursos/Audio';O.mkdir(exist_ok=True);sr=44100;rng=np.random.default_rng(61)
def guardar(n,s):
 s=np.asarray(s);s=np.column_stack((s,np.roll(s,211)*.96)) if s.ndim==1 else s;s=np.tanh(s);s*=min(1,.78/max(.001,np.max(abs(s))));data=(s*32767).astype('<i2')
 with wave.open(str(O/(n+'.wav')),'wb') as f:f.setnchannels(2);f.setsampwidth(2);f.setframerate(sr);f.writeframes(data.tobytes())
 print(n,len(s)/sr,'rms',round(float(np.sqrt(np.mean(s*s))),4))
def ruido(n,k=160):
 a=rng.normal(0,1,n);c=np.r_[0,np.cumsum(a)];out=(c[k:]-c[:-k])/np.sqrt(k);return np.pad(out,(0,k-1),mode='wrap')
t=np.arange(sr*32)/sr;music=np.zeros(len(t));chords=[[48,55,60,64],[45,52,57,60],[41,48,53,57],[43,50,55,62]]
for j,ch in enumerate(chords):
 mask=(t>=j*8)&(t<(j+1)*8);u=t[mask]-j*8;env=np.minimum(1,u/.8)*np.minimum(1,(8-u)/.8)
 for note in ch:
  freq=440*2**((note-69)/12);music[mask]+=.035*env*(np.sin(2*np.pi*freq*u)+.23*np.sin(2*np.pi*freq*2.003*u))
 for beat in range(16):
  v=u-beat*.5;active=(v>=0)&(v<1);freq=440*2**((ch[beat%4]+12-69)/12);music[np.where(mask)[0][active]]+=.034*np.exp(-v[active]*4)*np.minimum(1,v[active]*80)*np.sin(2*np.pi*freq*v[active])
guardar('Orbita_ambiental',music*1.6)
t=np.arange(sr*12)/sr;ambient=.045*ruido(len(t),550)*(1+.25*np.sin(2*np.pi*t/12))+.009*np.sin(2*np.pi*73*t);guardar('Viento_instalaciones',ambient)
t=np.arange(int(sr*2))/sr;env=np.sin(np.pi*np.minimum(t/2,1))**.5;guardar('Puerta_hangar',env*(.10*np.sin(2*np.pi*(180*t+50*t*t))+.035*ruido(len(t),25)))
t=np.arange(sr*4)/sr;env=np.minimum(1,t/.04)*np.minimum(1,(4-t)/.04);guardar('Propulsion',env*(.20*ruido(len(t),220)+.06*np.sin(2*np.pi*47*t)+.025*np.sin(2*np.pi*91*t)))
t=np.arange(sr*2)/sr;env=np.minimum(1,t/.02)*np.minimum(1,(2-t)/.02);guardar('Motor_nave',env*(.055*np.sin(2*np.pi*110*t)+.025*np.sin(2*np.pi*221*t)+.018*ruido(len(t),15)))
for name,notes in [('Inicio_carrera',[60,64,67]),('Final_carrera',[67,72,76,79])]:
 t=np.arange(sr*2)/sr;s=np.zeros(len(t))
 for i,n in enumerate(notes):
  u=t-i*.25;m=u>=0;f=440*2**((n-69)/12);s[m]+=.12*np.sin(2*np.pi*f*u[m])*np.exp(-u[m]*5)*np.minimum(1,u[m]*80)
 guardar(name,s)
(O/'PROCEDENCIA.txt').write_text('Composición Orbita ambiental y seis ambientes/efectos originales sintetizados para el proyecto. Sin muestras ni música comercial de Halo/Titanfall. PCM estéreo 44 100 Hz, 16 bits. Generación reproducible en Herramientas/generar_audio.py. Música: progresión armónica de cuatro acordes, arpegios y timbres de pad; bucle de 32 segundos.\n',encoding='utf-8')
