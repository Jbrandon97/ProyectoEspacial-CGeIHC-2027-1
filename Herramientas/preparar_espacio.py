from pathlib import Path
import struct,json,io,numpy as np
from PIL import Image,ImageDraw
import argparse
ap=argparse.ArgumentParser();ap.add_argument('glb',type=Path);ap.add_argument('--salida',type=Path,default=Path(__file__).resolve().parents[1]);args=ap.parse_args()
base=args.salida;b=args.glb.read_bytes();n=struct.unpack_from('<I',b,12)[0];d=json.loads(b[20:20+n]);binary=b[28+n:]
out=base/'Recursos/Modelos/Gateway';out.mkdir(parents=True,exist_ok=True)
for i,im in enumerate(d['images']):
 v=d['bufferViews'][im['bufferView']];o=v.get('byteOffset',0);img=Image.open(io.BytesIO(binary[o:o+v['byteLength']]));img.thumbnail((1024,1024));img.save(out/f'imagen_{i}.png');d['images'][i]={'uri':f'imagen_{i}.png'}
used=sorted({a['bufferView'] for a in d['accessors']});views=[];geo=bytearray();mapping={}
for old in used:
 v=d['bufferViews'][old].copy();at=v.get('byteOffset',0)
 while len(geo)%4:geo.append(0)
 v['byteOffset']=len(geo);geo.extend(binary[at:at+v['byteLength']]);mapping[old]=len(views);views.append(v)
for a in d['accessors']:a['bufferView']=mapping[a['bufferView']]
d['bufferViews']=views;d['buffers']=[{'uri':'Gateway.bin','byteLength':len(geo)}]
def local(n):
 if 'matrix' in n:return np.array(n['matrix']).reshape(4,4).T
 x,y,z,w=n.get('rotation',[0,0,0,1]);m=np.eye(4);m[:3,:3]=[[1-2*y*y-2*z*z,2*x*y-2*z*w,2*x*z+2*y*w],[2*x*y+2*z*w,1-2*x*x-2*z*z,2*y*z-2*x*w],[2*x*z-2*y*w,2*y*z+2*x*w,1-2*x*x-2*y*y]];m[:3,:3]@=np.diag(n.get('scale',[1,1,1]));m[:3,3]=n.get('translation',[0,0,0]);return m
parents={c:i for i,n in enumerate(d['nodes']) for c in n.get('children',[])};cache={}
def world(i):
 if i not in cache:cache[i]=(world(parents[i]) if i in parents else np.eye(4))@local(d['nodes'][i])
 return cache[i]
pts=[]
for i,node in enumerate(d['nodes']):
 if 'mesh' not in node:continue
 for p in d['meshes'][node['mesh']]['primitives']:
  a=d['accessors'][p['attributes']['POSITION']]
  for x in (a['min'][0],a['max'][0]):
   for y in (a['min'][1],a['max'][1]):
    for z in (a['min'][2],a['max'][2]):pts.append((world(i)@np.array([x,y,z,1]))[:3])
pts=np.array(pts);low=pts.min(0);high=pts.max(0);scale=80/max(high-low);center=(high+low)/2
roots=d['scenes'][d.get('scene',0)]['nodes'];d['nodes'].append({'name':'Colocacion_Gateway_80_unidades','children':roots,'translation':(-center*scale).tolist(),'scale':[scale]*3});d['scenes']=[{'nodes':[len(d['nodes'])-1]}];d['scene']=0
(out/'Gateway.bin').write_bytes(geo);(out/'Gateway.gltf').write_text(json.dumps(d,separators=(',',':')))
(out/'PROCEDENCIA.txt').write_text('Gateway Core — NASA/JSC/Gateway Program Office.\nFuente oficial: https://science.nasa.gov/3d-resources/gateway-lunar-space-station/\nModelo: https://assets.science.nasa.gov/content/dam/science/cds/3d/resources/model/gateway/Gateway%20Core.glb\nReferencia de autores en la propuesta del equipo: Alberto G. Bertolin y Bradley W. Reynolds, Graphics and Animation Lab de JSC.\nAdaptacion: conversion GLB a glTF externo, PNG hasta 1024 px, extraccion de imagenes del buffer y escala uniforme a 80 unidades. Geometria original conservada, 92209 triangulos. No se atribuye el modelo al equipo.\n',encoding='utf-8')
print('GATEWAY_OPTIMIZADO',len(geo),'bytes geometria; dimensiones',((high-low)*scale).round(2))
# Superficies originales de los cuerpos y cielo: recursos deterministas, no capturas.
rng=np.random.default_rng(711);tex=base/'Recursos/Modelos/Espacio';tex.mkdir(parents=True,exist_ok=True)
y,x=np.mgrid[0:512,0:1024];lon=x/1024*2*np.pi;lat=y/512*np.pi
for name,colors in [('PlanetaHalo',([20,45,80],[62,92,49])),('PlanetaOrokin',([30,22,65],[100,74,121])),('Luna',([76,78,82],[148,146,139]))]:
 f=np.sin(lon*3+np.sin(lat*5))*np.cos(lat*4)+.5*np.sin(lon*7+lat*5)+.22*np.cos(lon*13-lat*11)
 if name=='Luna':f+=rng.normal(0,.18,f.shape)
 t=np.clip((f+.3)*1.4,0,1)[...,None];arr=np.array(colors[0])*(1-t)+np.array(colors[1])*t
 if name!='Luna':arr+=np.clip((np.abs(np.cos(lat))-.88)/.12,0,1)[...,None]*90
 Image.fromarray(np.uint8(np.clip(arr,0,255))).save(tex/(name+'.png'))
cielo=base/'Recursos/Cielos/Espacio';cielo.mkdir(parents=True,exist_ok=True)
im=Image.new('RGB',(2048,1024),(2,3,9));dr=ImageDraw.Draw(im)
for i in range(1700):
 px=int(rng.integers(0,2048));py=int(np.arccos(rng.uniform(-1,1))/np.pi*1024);v=int(rng.integers(70,235));radio=1 if i%19==0 else 0;dr.ellipse((px-radio,py-radio,px+radio,py+radio),fill=(v,v,min(255,v+18)))
im.save(cielo/'skybox-day.png')
(tex/'PROCEDENCIA.txt').write_text('Texturas procedurales originales para visualizacion esquematica de los dos planetas y la luna; no son mapas astronomicos reales. Cielo estelar sintetico original, semilla 711. Sin recursos de practicas ni capturas generadas como evidencia.\n',encoding='utf-8')
