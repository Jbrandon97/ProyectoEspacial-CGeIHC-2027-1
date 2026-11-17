from pathlib import Path
import sys, wave, json
import numpy as np

raiz=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parents[1]
destino=raiz/'Recursos/Audio'; destino.mkdir(parents=True,exist_ok=True)
sr=44100; rng=np.random.default_rng(7274); medidas={}
def guardar(nombre,senal):
    if senal.ndim==1: senal=senal[:,None]
    senal=senal-senal.mean(axis=0)
    pico=float(np.max(np.abs(senal)))
    if pico>.65: senal*=.65/pico
    pcm=np.round(senal*32767).astype('<i2')
    with wave.open(str(destino/(nombre+'.wav')),'wb') as w:
        w.setnchannels(senal.shape[1]);w.setsampwidth(2);w.setframerate(sr);w.writeframes(pcm.tobytes())
    medidas[nombre]={'duracion':len(senal)/sr,'pico':float(np.max(np.abs(senal))), 'RMS':float(np.sqrt(np.mean(senal**2))),'salto_bucle_PCM':int(np.max(np.abs(pcm[0].astype(int)-pcm[-1].astype(int))))}
def ruido(n,corte):
    espectro=np.fft.rfft(rng.normal(size=n));f=np.fft.rfftfreq(n,1/sr)
    espectro*=1/(1+(f/corte)**4);espectro[f<30]=0
    a=np.fft.irfft(espectro,n);return a/(np.std(a)+1e-9)

# Cuatro acordes solapados, con graves cálidos y armónicos moderados.
dur=48;t=np.arange(sr*dur)/sr;musica=np.zeros((len(t),2))
for j,notas in enumerate([[36,43,50,55],[33,40,48,52],[29,36,45,48],[31,38,46,50]]):
    fase=((t-j*12+12)%48)-12
    env=np.where((fase>=-6)&(fase<18),np.sin(np.pi*(fase+6)/24)**2,0)
    for k,nota in enumerate(notas):
        f=round(440*2**((nota-69)/12)*dur)/dur
        for c in range(2):
            a=2*np.pi*f*t+c*.1*(k+1)
            musica[:,c]+=.035*env*(np.sin(a)+.12*np.sin(2*a))
guardar('Horizonte_espacial',musica)
dur=32;t=np.arange(sr*dur)/sr;musica=np.zeros((len(t),2))
for j,notas in enumerate([[45,52,59],[41,48,55],[43,50,57],[40,47,54]]):
    fase=((t-j*8+8)%32)-8
    env=np.where((fase>=-4)&(fase<12),np.sin(np.pi*(fase+4)/16)**2,0)
    for k,nota in enumerate(notas):
        f=round(440*2**((nota-69)/12)*dur)/dur
        for c in range(2): musica[:,c]+=.024*env*np.sin(2*np.pi*f*t+c*.15*k)
for k,nota in enumerate([57,64,59,67,55,62,52,59]):
    fase=(t-k*2)%16
    env=np.where(fase<2,(1-np.exp(-fase*35))*np.exp(-fase*2.5),0)
    f=round(440*2**((nota-69)/12)*dur)/dur
    for c in range(2):musica[:,c]+=.018*env*np.sin(2*np.pi*f*t+c*.12)
guardar('Ambiente_Planeta2',musica)
for nombre,corte,base in [('Cabina_espacial',220,48),('Instalaciones_Planeta2',380,72)]:
    t=np.arange(sr*16)/sr
    a=.018*ruido(len(t),corte)+.012*np.sin(2*np.pi*base*t)
    guardar(nombre,np.column_stack([a,np.roll(a,190)]))
t=np.arange(sr*4)/sr
guardar('Motor_trafico',.10*np.sin(2*np.pi*65*t)+.035*ruido(len(t),300))
guardar('Boost',(.075*ruido(len(t),450)+.055*np.sin(2*np.pi*92*t))*(.85+.15*np.cos(2*np.pi*t/4)))
(destino/'Sintesis_integral.json').write_text(json.dumps(medidas,indent=2),encoding='utf-8')
print(json.dumps(medidas,indent=2))
