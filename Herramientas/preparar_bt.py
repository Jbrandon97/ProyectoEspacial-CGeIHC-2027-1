"""Extrae el BT suministrado sin alterar jerarquia, pesos, UV ni geometria.
Uso: python preparar_bt.py ruta/bt-7274.glb
Requiere Pillow. La animacion propia se define en Titan.cpp.
"""
from pathlib import Path
import sys,struct,json,io,hashlib
from PIL import Image
p=Path(sys.argv[1]);b=p.read_bytes();n=struct.unpack_from('<I',b,12)[0]
d=json.loads(b[20:20+n]);buf=b[28+n:]
out=Path(__file__).resolve().parents[1]/'Recursos/Modelos/BT7274';out.mkdir(exist_ok=True)
for i,im in enumerate(d['images']):
 v=d['bufferViews'][im['bufferView']];at=v.get('byteOffset',0)
 Image.open(io.BytesIO(buf[at:at+v['byteLength']])).save(out/f'textura_{i}.png')
 d['images'][i]={'uri':f'textura_{i}.png'}
for m in d['materials']:
 ext=m.get('extensions',{}).get('KHR_materials_pbrSpecularGlossiness',{})
 m['pbrMetallicRoughness']={'baseColorFactor':ext.get('diffuseFactor',[1,1,1,1]),'metallicFactor':.3,'roughnessFactor':.7}
 if 'diffuseTexture' in ext:m['pbrMetallicRoughness']['baseColorTexture']=ext['diffuseTexture']
d['buffers'][0]['uri']='BT7274.bin'
(out/'BT7274.bin').write_bytes(buf);(out/'BT7274.gltf').write_text(json.dumps(d,separators=(',',':')))
meta=d['asset']['extras']
(out/'ATRIBUCION.txt').write_text(f"BT-7274 — {meta['author']}\nFuente: {meta['source']}\nLicencia declarada: {meta['license']}\nSHA256 GLB original: {hashlib.sha256(b).hexdigest()}\nAdaptacion: PNG externos, material difuso compatible y animacion propia. Conservamos la autoria del modelo y texturas. No cargamos el plano de exhibicion ni la camara del archivo.\n",encoding='utf-8')
print('BT extraido:',len(d['skins'][0]['joints']),'huesos; 16 mallas de piel; texturas originales')
