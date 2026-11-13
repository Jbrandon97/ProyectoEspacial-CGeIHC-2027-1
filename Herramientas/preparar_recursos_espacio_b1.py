from pathlib import Path
import json,struct,io,copy,numpy as np,sys
from PIL import Image
sys.stdout.reconfigure(encoding='utf-8')
import argparse
parser=argparse.ArgumentParser();parser.add_argument('origen',type=Path);parser.add_argument('proyecto',type=Path);args=parser.parse_args()
SRC=args.origen;DST=args.proyecto/'Recursos/Modelos/EspacioBloque1';DST.mkdir(parents=True,exist_ok=True)
credits=[];stats=[]
def convert(name,title,size,split=False):
 raw=(SRC/(name+'.glb')).read_bytes();n=struct.unpack_from('<I',raw,12)[0];j=json.loads(raw[20:20+n]);binary=raw[28+n:];credits.append(j['asset']['extras'])
 def acc(i):
  a=j['accessors'][i];v=j['bufferViews'][a['bufferView']];dt={5126:'<f4',5125:'<u4',5123:'<u2',5121:'u1'}[a['componentType']];sz={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']];return np.ndarray((a['count'],sz),dtype=dt,buffer=binary,offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',np.dtype(dt).itemsize*sz),np.dtype(dt).itemsize)).copy()
 def matrix(node):
  if 'matrix'in node:return np.array(node['matrix']).reshape(4,4).T
  x,y,z,w=node.get('rotation',[0,0,0,1]);m=np.eye(4);m[:3,:3]=np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],[2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],[2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])@np.diag(node.get('scale',[1,1,1]));m[:3,3]=node.get('translation',[0,0,0]);return m
 imgs=[]
 for im in j.get('images',[]):
  v=j['bufferViews'][im['bufferView']];imgs.append(Image.open(io.BytesIO(binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']])).convert('RGBA'))
 materials=[];images=[];textures=[];used={}
 def tex(info):
  idx=j['textures'][info['index']]['source']
  if idx not in used:
   filename=f'{title}_textura{idx}.png';pic=imgs[idx].copy();pic.thumbnail((1024,1024));pic.save(DST/filename);used[idx]=len(images);images.append({'uri':filename});textures.append({'source':used[idx]})
  return {'index':used[idx]}
 for original in j.get('materials',[]):
  m={'name':original.get('name','Material'),'doubleSided':True};p=copy.deepcopy(original.get('pbrMetallicRoughness',{}));spec=original.get('extensions',{}).get('KHR_materials_pbrSpecularGlossiness')
  if spec:p={'baseColorFactor':spec.get('diffuseFactor',[1,1,1,1]),'baseColorTexture':spec['diffuseTexture'],'metallicFactor':.35,'roughnessFactor':.35}
  if 'baseColorTexture'in p:p['baseColorTexture']=tex(p['baseColorTexture'])
  if 'metallicRoughnessTexture'in p:
   im=np.asarray(imgs[j['textures'][p.pop('metallicRoughnessTexture')['index']]['source']])/255
   p['roughnessFactor']=float(np.clip(im[:,:,1].mean(),.25,.9));p['metallicFactor']=float(im[:,:,2].mean())
  p.setdefault('roughnessFactor',.65);p.setdefault('metallicFactor',.3)
  if p.get('baseColorFactor',[1,1,1,1])[3]<.4:p['baseColorFactor']=[.08,.15,.22,1]
  m['pbrMetallicRoughness']=p
  if 'emissiveFactor'in original:
   factor=np.array(original['emissiveFactor']);strength=original.get('extensions',{}).get('KHR_materials_emissive_strength',{}).get('emissiveStrength',1)
   m['emissiveFactor']=(factor*min(1.3,float(strength))*.65).tolist()
  if 'emissiveTexture'in original:m['emissiveTexture']=tex(original['emissiveTexture'])
  materials.append(m)
 parts=[]
 def walk(i,parent):
  node=j['nodes'][i];mat=parent@matrix(node)
  if 'mesh'in node:
   for pr in j['meshes'][node['mesh']]['primitives']:
    assert pr.get('mode',4)==4
    pos=acc(pr['attributes']['POSITION']);pos=(np.c_[pos,np.ones(len(pos))]@mat.T)[:,:3]
    nor=acc(pr['attributes']['NORMAL'])@np.linalg.inv(mat[:3,:3]);nor/=np.maximum(np.linalg.norm(nor,axis=1)[:,None],1e-8)
    uv=acc(pr['attributes']['TEXCOORD_0']) if 'TEXCOORD_0'in pr['attributes'] else np.zeros((len(pos),2))
    mi=pr.get('material',0);orig=j['materials'][mi];info=orig.get('pbrMetallicRoughness',{}).get('baseColorTexture') or orig.get('emissiveTexture',{})
    tr=info.get('extensions',{}).get('KHR_texture_transform',{});angle=tr.get('rotation',0);r=np.array([[np.cos(angle),-np.sin(angle)],[np.sin(angle),np.cos(angle)]]);uv=(uv*np.array(tr.get('scale',[1,1])))@r.T+tr.get('offset',[0,0])
    ix=acc(pr['indices']).ravel().astype(np.uint32) if 'indices'in pr else np.arange(len(pos),dtype=np.uint32)
    if np.linalg.det(mat[:3,:3])<0:ix=ix.reshape(-1,3)[:,[0,2,1]].ravel()
    parts.append((mi,pos,nor,uv,ix))
  for child in node.get('children',[]):walk(child,mat)
 for i in j['scenes'][j.get('scene',0)]['nodes']:walk(i,np.eye(4))
 subsets=[[p] for p in parts] if split else [parts]
 for num,sub in enumerate(subsets):
  allpos=np.concatenate([p[1] for p in sub]);lo=allpos.min(0);hi=allpos.max(0);center=(lo+hi)/2;scale=size/max(hi-lo)
  groups={}
  for mi,pos,nor,uv,ix in sub:groups.setdefault(mi,[]).append(((pos-center)*scale,nor,uv,ix))
  doc={'asset':{'version':'2.0','extras':j['asset']['extras']},'scene':0,'scenes':[{'nodes':[0]}],'nodes':[{'mesh':0}],'meshes':[{'primitives':[]}],'materials':materials,'images':images,'textures':textures,'bufferViews':[],'accessors':[]};data=bytearray()
  def add(a,typ,component):
   while len(data)%4:data.append(0)
   view=len(doc['bufferViews']);doc['bufferViews'].append({'buffer':0,'byteOffset':len(data),'byteLength':a.nbytes});data.extend(a.tobytes());ac={'bufferView':view,'componentType':component,'count':len(a),'type':typ}
   if typ=='VEC3':ac.update(min=a.min(0).tolist(),max=a.max(0).tolist())
   doc['accessors'].append(ac);return len(doc['accessors'])-1
  vcount=0;tcount=0
  for mi,group in groups.items():
   va=[];indices=[];offset=0
   for pos,nor,uv,ix in group:va.append(np.c_[pos,nor,uv]);indices.append(ix+offset);offset+=len(pos)
   vertices=np.concatenate(va).astype('<f4');ix=np.concatenate(indices)
   unique,inverse=np.unique(vertices,axis=0,return_inverse=True);ix=inverse[ix].astype('<u4');vcount+=len(unique);tcount+=len(ix)//3
   doc['meshes'][0]['primitives'].append({'attributes':{'POSITION':add(unique[:,:3].copy(),'VEC3',5126),'NORMAL':add(unique[:,3:6].copy(),'VEC3',5126),'TEXCOORD_0':add(unique[:,6:8].copy(),'VEC2',5126)},'indices':add(ix,'SCALAR',5125),'material':mi})
  fname=title+(str(num)if split else '');(DST/(fname+'.bin')).write_bytes(data);doc['buffers']=[{'uri':fname+'.bin','byteLength':len(data)}];(DST/(fname+'.gltf')).write_text(json.dumps(doc,separators=(',',':')),encoding='utf-8');stats.append({'modelo':fname,'triangulos':tcount,'vertices_indexados':vcount,'lotes':len(groups),'dimensiones':((hi-lo)*scale).tolist(),'bytes_geometria':len(data)})
convert('freeport_space_station_2','Freeport',130)
convert('cool_alien_spaceship_by_jungle_jim','NaveAlien',13)
convert('jedi_holocron','Holocron',1.5)
convert('asteroids_-_low_poly','Asteroide',2,True)
# El cielo se extrae como panorama equirectangular; no se dibuja la esfera del GLB.
raw=(SRC/'alien_space_nebula_1_skybox.glb').read_bytes();n=struct.unpack_from('<I',raw,12)[0];j=json.loads(raw[20:20+n]);binary=raw[28+n:];credits.append(j['asset']['extras']);im=j['images'][j['textures'][0]['source']];v=j['bufferViews'][im['bufferView']];pic=Image.open(io.BytesIO(binary[v.get('byteOffset',0):v.get('byteOffset',0)+v['byteLength']]));pic.convert('RGB').save(args.proyecto/'Recursos/Cielos/Espacio/skybox-day.png')
(DST/'ATRIBUCIONES.txt').write_text('\n\n'.join('\n'.join(f'{k}: {v}' for k,v in c.items())for c in credits)+'\n\nAdaptacion: transformaciones aplicadas, escala y centrado, agrupacion por material, soldadura exacta de vertices, indices, PNG externos, UV de ventanas corregidas y materiales aproximados al shader existente. Emision limitada para conservar detalle sin HDR. No atribuimos la creacion de estos modelos al equipo.\n',encoding='utf-8')
(DST/'conversion.json').write_text(json.dumps(stats,indent=2),encoding='utf-8');print(json.dumps(stats,indent=2))
