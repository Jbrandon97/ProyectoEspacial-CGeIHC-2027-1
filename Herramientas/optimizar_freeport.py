import bpy,sys,json
from pathlib import Path
args=sys.argv[sys.argv.index('--')+1:];base=Path(args[0]);out=Path(args[1]);out.mkdir(parents=True,exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(base/'Recursos/Modelos/EspacioBloque1/Freeport.gltf'))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];stats=[]
for o in meshes:
 bpy.context.view_layer.objects.active=o;o.select_set(True);before=len(o.data.polygons)
 mod=o.modifiers.new('Reducir detalle subpixel','DECIMATE');mod.ratio=.28;mod.use_collapse_triangulate=True
 bpy.ops.object.modifier_apply(modifier=mod.name);stats.append({'antes':before,'despues':len(o.data.polygons)})
print('OPTIMIZACION',stats,flush=True)
bpy.ops.export_scene.gltf(filepath=str(out/'Freeport.gltf'),export_format='GLTF_SEPARATE',export_image_format='AUTO',export_animations=False,export_yup=True,export_materials='EXPORT')
(out/'reduccion.json').write_text(json.dumps(stats))

original=json.loads((base/"Recursos/Modelos/EspacioBloque1/Freeport.gltf").read_text());nuevo=json.loads((out/"Freeport.gltf").read_text());nuevo["asset"]["extras"]=original["asset"].get("extras",{});(out/"Freeport.gltf").write_text(json.dumps(nuevo),encoding="utf-8")
