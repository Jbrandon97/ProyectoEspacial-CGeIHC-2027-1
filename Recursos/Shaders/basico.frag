#version 330 core
in vec3 normalMundo,material,posMundo;
in vec2 coordenada;
uniform vec3 color,luz,opciones,texturizado,emision;
uniform sampler2D albedo,mapaEmision;
uniform int usaEmision;
uniform vec3 sistema,solDir,solColor,ojo,propiedades;
struct Puntual{vec3 pos,color,atenuacion;};
uniform Puntual pLuces[16];
uniform int cantidadPuntuales;
uniform vec3 recorte;
uniform vec3 humo;
uniform vec3 focoPos,focoDir,focoDatos;
struct Focal { vec3 pos,dir,color,atenuacion,cono; };
uniform Focal focales[7];
uniform int cantidadFocales;
uniform vec3 volumen;
out vec4 colorFinal;
vec3 iluminar(vec3 base,vec3 n,vec3 v,vec3 dir,vec3 energia){
 float dif=max(dot(n,dir),0);vec3 h=normalize(dir+v);
 float r=clamp(propiedades.x,.08,1),metal=clamp(propiedades.y,0,1);
 float potencia=mix(100,5,r);vec3 ks=mix(vec3(.045),base,metal);
 float esp=pow(max(dot(n,h),0),potencia)*(1-r*.8)*step(.0001,dif);
 return energia*(base*dif*(1-metal*.35)+ks*esp*3);
}
vec3 aporteFocal(Focal f,vec3 base,vec3 n,vec3 v){
 if(dot(f.color,f.color)<.000001)return vec3(0);
 vec3 delta=f.pos-posMundo;float d=length(delta);vec3 l=delta/max(d,.001);
 float angulo=dot(-l,f.dir);
 if(angulo<=f.cono.y)return vec3(0);
 float cono=smoothstep(f.cono.y,f.cono.x,angulo);
 return iluminar(base,n,v,l,f.color*cono/max(.001,f.atenuacion.x+f.atenuacion.y*d+f.atenuacion.z*d*d));
}
void main(){
 if(volumen.x>.5){
  vec3 vista=normalize(ojo-posMundo);
  float cara=dot(normalize(normalMundo),vista);
  float alpha=volumen.x>1.5
   ? pow(abs(cara),.8)*(1-smoothstep(.15,1,coordenada.y))*volumen.y*.55
   : pow(max(cara,0),1.8)*volumen.y;
  colorFinal=vec4(color*material,alpha);return;
 }
 if(humo.x>0 && fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453)<humo.x)discard;
 if(posMundo.y<recorte.x)discard;
 vec4 tex=texturizado.x>.5?texture(albedo,coordenada):vec4(1);if(tex.a<.4)discard;
 vec3 base=color*material*tex.rgb;
 if(opciones.x>.5){colorFinal=vec4(base,1);return;}
 vec3 n=normalize(normalMundo);if(!gl_FrontFacing)n=-n;
 if(sistema.x<.5){colorFinal=vec4(base*(.38+.62*max(dot(n,normalize(luz)),0))+emision,1);return;}
 vec3 v=normalize(ojo-posMundo);vec3 total=base*sistema.z;
 total+=iluminar(base,n,v,normalize(solDir),solColor*sistema.y);
 for(int i=0;i<16;i++){if(i>=cantidadPuntuales)break;if(dot(pLuces[i].color,pLuces[i].color)<.000001)continue;vec3 delta=pLuces[i].pos-posMundo;float d=length(delta);vec3 at=pLuces[i].atenuacion;float factor=1/max(.001,at.x+at.y*d+at.z*d*d);total+=iluminar(base,n,v,delta/max(d,.001),pLuces[i].color*factor);}
 for(int i=0;i<cantidadFocales;i++)total+=aporteFocal(focales[i],base,n,v);
 vec3 delta=focoPos-posMundo;float d=length(delta);vec3 ld=delta/max(d,.001);float cono=smoothstep(focoDatos.z,focoDatos.y,dot(-ld,normalize(focoDir)));
 total+=iluminar(base,n,v,ld,vec3(1,.94,.79)*focoDatos.x*cono/(1+.055*d+.010*d*d));
 colorFinal=vec4(total+emision*(usaEmision!=0?texture(mapaEmision,coordenada).rgb:vec3(1)),1);
}
