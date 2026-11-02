# Derivados de los recursos originales: cortar caras conservando normales, UV y materiales.
from pathlib import Path
import json,copy,numpy as np
P=Path(__file__).resolve().parents[1]/'Recursos/Modelos'
def load(name):
 d=json.loads((P/(name+'.gltf')).read_text());buf=[(P/b['uri']).read_bytes() for b in d['buffers']]
 def acc(i):
  a=d['accessors'][i];v=d['bufferViews'][a['bufferView']];n={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4}[a['type']];dt=np.dtype({5126:'<f4',5123:'<u2',5125:'<u4',5121:'u1'}[a['componentType']]);stride=v.get('byteStride',dt.itemsize*n)
  return np.ndarray((a['count'],n),dtype=dt,buffer=buf[v['buffer']],offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(stride,dt.itemsize)).copy()
 groups=[]
 for node in d['nodes']:
  if 'mesh' not in node:continue
  for pr in d['meshes'][node['mesh']]['primitives']:
   pos=acc(pr['attributes']['POSITION']);nor=acc(pr['attributes']['NORMAL']);uv=acc(pr['attributes']['TEXCOORD_0'])
   if name=='SaturnV':pos=pos*4+np.array([.04902,.56356,-2.972212])
   idx=acc(pr['indices']).flatten() if 'indices' in pr else np.arange(len(pos))
   groups.append((np.column_stack((pos,nor,uv))[idx].reshape(-1,3,8),pr['material']))
 return d,groups

def clip(poly,axis,value,greater):
 out=[]
 for a,b in zip(poly,poly[1:]+poly[:1]):
  da=(a[axis]-value)*(1 if greater else -1);db=(b[axis]-value)*(1 if greater else -1)
  if da>=-1e-8:out.append(a)
  if (da<0 and db>0) or (da>0 and db<0):out.append(a+(b-a)*(da/(da-db)))
 return out

def tri(poly):return [np.array([poly[0],poly[i],poly[i+1]]) for i in range(1,len(poly)-1)]
def area(t):return np.linalg.norm(np.cross(t[1,:3]-t[0,:3],t[2,:3]-t[0,:3]))*.5

def write(name,source,groups):
 d={k:copy.deepcopy(source[k]) for k in ['asset','materials','textures','images','samplers'] if k in source};d.update({'scene':0,'scenes':[{'nodes':[]}],'nodes':[],'meshes':[],'buffers':[],'bufferViews':[],'accessors':[]});b=bytearray()
 for stage,mat,ts in groups:
  if not ts:continue
  v=np.array(ts,dtype=np.float32).reshape(-1,8);norm=np.linalg.norm(v[:,3:6],axis=1);v[:,3:6]/=np.maximum(norm[:,None],1e-8)
  attrs={}
  for label,start,end in [('POSITION',0,3),('NORMAL',3,6),('TEXCOORD_0',6,8)]:
   a=v[:,start:end].astype('<f4');off=len(b);b.extend(a.tobytes());view=len(d['bufferViews']);d['bufferViews'].append({'buffer':0,'byteOffset':off,'byteLength':a.nbytes,'target':34962});ix=len(d['accessors']);ac={'bufferView':view,'componentType':5126,'count':len(a),'type':'VEC'+str(end-start)}
   if label=='POSITION':ac.update(min=a.min(0).tolist(),max=a.max(0).tolist())
   d['accessors'].append(ac);attrs[label]=ix
  mesh=len(d['meshes']);d['meshes'].append({'primitives':[{'attributes':attrs,'material':mat,'mode':4}]});node=len(d['nodes']);d['nodes'].append({'name':('Saturn_Etapa_'+str(stage)) if stage>=0 else 'Plataforma_con_hueco','mesh':mesh});d['scenes'][0]['nodes'].append(node)
 d['buffers']=[{'uri':name+'.bin','byteLength':len(b)}];(P/(name+'.bin')).write_bytes(b);(P/(name+'.gltf')).write_text(json.dumps(d,separators=(',',':')),encoding='utf-8');print(name,'triangulos',sum(len(g[2]) for g in groups),'bytes',len(b))

s,groups=load('SaturnV');cuts=[4*4.346598148345947+.56356,4*7.742214202880859+.56356];out=[];oldarea=newarea=0
for ts,mat in groups:
 oldarea+=sum(area(t) for t in ts)
 for stage,(lo,hi) in enumerate(zip([-100]+cuts,cuts+[100])):
  result=[]
  for t in ts:
   poly=clip(list(t),1,lo,True);poly=clip(poly,1,hi,False) if poly else [];result+=tri(poly)
  newarea+=sum(area(t) for t in result);out.append((stage,mat,result))
assert abs(oldarea-newarea)<oldarea*1e-5,(oldarea,newarea)
# Tapas discretas en las secciones cortadas: cierran el volumen sin cambiar su silueta.
capmat=len(s['materials']);s['materials'].append({'name':'Seccion_estructural','pbrMetallicRoughness':{'baseColorFactor':[.24,.27,.29,1],'metallicFactor':.5,'roughnessFactor':.7}})
for cutindex,y in enumerate(cuts):
 points=[]
 for _,_,ts in out:
  for t in ts:
   points.extend((float(v[0]),float(v[2])) for v in t if abs(v[1]-y)<1e-5)
 points=sorted(set((round(x,5),round(z,5)) for x,z in points))
 def cross(o,a,b):return (a[0]-o[0])*(b[1]-o[1])-(a[1]-o[1])*(b[0]-o[0])
 lower=[];upper=[]
 for q in points:
  while len(lower)>1 and cross(lower[-2],lower[-1],q)<=0:lower.pop()
  lower.append(q)
 for q in reversed(points):
  while len(upper)>1 and cross(upper[-2],upper[-1],q)<=0:upper.pop()
  upper.append(q)
 hull=lower[:-1]+upper[:-1];cx,cz=np.mean(hull,axis=0)
 for stage,normal in [(cutindex,1),(cutindex+1,-1)]:
  ts=[]
  for a,b in zip(hull,hull[1:]+hull[:1]):
   vs=np.array([[cx,y,cz,0,normal,0,0,0],[a[0],y,a[1],0,normal,0,0,0],[b[0],y,b[1],0,normal,0,0,0]])
   if np.cross(vs[1,:3]-vs[0,:3],vs[2,:3]-vs[0,:3])[1]*normal<0:vs[[1,2]]=vs[[2,1]]
   ts.append(vs)
  out.append((stage,capmat,ts))
write('SaturnV_Etapas',s,out);print('SUPERFICIE_ORIGINAL_CONSERVADA',oldarea,newarea,'CORTES',cuts)
s,groups=load('Plataforma');out=[]
for ts,mat in groups:
 result=[]
 for t in ts:
  inside=list(t)
  for axis,value,greater in [(0,-5,True),(0,5,False),(2,1,True),(2,11,False)]:
   if not inside:break
   result+=tri(clip(inside,axis,value,not greater));inside=clip(inside,axis,value,greater)
 out.append((-1,mat,result))
write('Plataforma_E',s,out)
