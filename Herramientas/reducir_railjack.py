"""Ejecutar con Blender en segundo plano; conserva nodos, materiales y animaciones originales."""
import bpy,sys,json,copy,shutil
from pathlib import Path
base=Path(sys.argv[sys.argv.index('--')+1]).resolve()
carpeta=base/'Recursos/Modelos/Railjack';tmp=Path(sys.argv[sys.argv.index('--')+2]).resolve();tmp.mkdir(parents=True,exist_ok=True)
origen=carpeta/'RailjackSeparadoVibraAnimado.gltf';j=json.loads(origen.read_text())
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(origen))
stats=[]
for o in bpy.context.scene.objects:
 if o.type!='MESH':continue
 bpy.context.view_layer.objects.active=o
 antes=len(o.data.polygons)
 if antes>300:
  m=o.modifiers.new('Detalle de distancia','DECIMATE');m.ratio=.30;m.use_collapse_triangulate=True
  bpy.ops.object.modifier_apply(modifier=m.name)
 stats.append(dict(nombre=o.name,antes=antes,despues=len(o.data.polygons)))
bpy.ops.export_scene.gltf(filepath=str(tmp/'reducido.gltf'),export_format='GLTF_SEPARATE',export_animations=False,export_yup=True,export_materials='EXPORT')
n=json.loads((tmp/'reducido.gltf').read_text());out=copy.deepcopy(j)
ab=len(out['accessors']);vb=len(out['bufferViews']);bb=len(out['buffers'])
for i,b in enumerate(n['buffers']):
 nuevo=f'Railjack_detalle_{i}.bin';shutil.copy2(tmp/b['uri'],carpeta/nuevo);b['uri']=nuevo;out['buffers'].append(b)
for v in n['bufferViews']:
 v['buffer']+=bb;out['bufferViews'].append(v)
for a in n['accessors']:
 assert 'sparse' not in a
 a['bufferView']+=vb;out['accessors'].append(a)
porNombre={x.get('name'):x for x in n['nodes'] if 'mesh' in x}
materiales={m['name']:i for i,m in enumerate(j['materials'])}
for nodo in j['nodes']:
 if 'mesh' not in nodo:continue
 nuevo=copy.deepcopy(n['meshes'][porNombre[nodo['name']]['mesh']])
 for pr in nuevo['primitives']:
  pr['indices']+=ab
  pr['attributes']={k:v+ab for k,v in pr['attributes'].items()}
  pr['material']=materiales[n['materials'][pr['material']]['name']]
 out['meshes'][nodo['mesh']]=nuevo
assert out['nodes']==j['nodes'] and out['animations']==j['animations'] and out['materials']==j['materials']
(carpeta/'Railjack_Optimizado.gltf').write_text(json.dumps(out),encoding='utf-8')
(carpeta/'Reduccion.json').write_text(json.dumps(stats,indent=2),encoding='utf-8')
print('REDUCCION_RAILJACK',sum(x['antes'] for x in stats),sum(x['despues'] for x in stats),flush=True)
