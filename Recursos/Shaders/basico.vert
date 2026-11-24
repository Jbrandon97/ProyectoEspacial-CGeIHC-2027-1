#version 330 core
layout(location=0) in vec3 posicion;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 colorVertice;
layout(location=3) in vec2 uv;
uniform mat4 modelo,vistaProyeccion;
uniform mat4 matrizNormal;
uniform vec3 locomocionClank;
out vec3 normalMundo,material,posMundo;
out vec2 coordenada;
void main(){
 vec3 local=posicion,n=normal;
 if(locomocionClank.y>3.5){
  bool vertical=locomocionClank.z>.5;
  float lado=sign(posicion.x);
  float peso=vertical
    ? smoothstep(.67,1.36,abs(posicion.x))*smoothstep(1.83,2.02,posicion.y)*(1-smoothstep(2.49,2.68,posicion.y))
    : smoothstep(.90,1.60,abs(posicion.x))*smoothstep(2.50,2.65,posicion.y)*(1-smoothstep(3.10,3.25,posicion.y));
  vec3 pivote=vertical?vec3(lado*.71,2.24,-.02):vec3(lado*.88,2.87,-.12);
  float a=lado*(-.19 + (vertical?.48*locomocionClank.x:0.0));
  mat3 elevacion=mat3(cos(a),sin(a),0,-sin(a),cos(a),0,0,0,1);
  float b=vertical?0.0:.50*locomocionClank.x;
  mat3 vaiven=mat3(cos(b),0,-sin(b),0,1,0,sin(b),0,cos(b));
  mat3 giro=vaiven*elevacion;
  local=mix(local,pivote+giro*(local-pivote),peso);
  n=normalize(mix(n,giro*n,peso));
 }else if(locomocionClank.y>2.5){
  // El Warframe original esta posado en Z y sus armas comparten las coordenadas del cuerpo.
  float lado=sign(posicion.x);
  float peso=smoothstep(10.0,20.0,abs(posicion.x))*(1-smoothstep(-24.0,-19.0,posicion.z));
  float a=lado*(0.12+0.40*locomocionClank.x);
  mat3 giro=mat3(cos(a),0,-sin(a),0,1,0,sin(a),0,cos(a));
  vec3 pivote=vec3(lado*10.5,1,-43);
  local=mix(local,pivote+giro*(local-pivote),peso);
  n=normalize(mix(n,giro*n,peso));
  float fase=fract(locomocionClank.z+(lado<0.0?.5:0.0));
  float pierna=smoothstep(-34.0,-13.0,posicion.z)*smoothstep(.8,4.0,abs(posicion.x));
  float paso=fase<.5?mix(-1.0,1.0,smoothstep(0.0,.5,fase)):3.0-4.0*fase;
  local.y+=4.25*paso*pierna;
  if(fase<.5) local.z-=3.5*sin(fase*6.2831853)*pierna;
 }else if(locomocionClank.y>1.5){
  float lado=sign(posicion.z);
  if(posicion.y<4.2){
   float a=lado*.22*sin(locomocionClank.x);
   mat3 giro=mat3(cos(a),sin(a),0,-sin(a),cos(a),0,0,0,1);
   float peso=1-smoothstep(3.2,4.2,posicion.y);vec3 pivote=vec3(0,4.2,lado*.65);
   local=mix(local,pivote+giro*(local-pivote),peso);n=normalize(mix(n,giro*n,peso));
  }else if(abs(posicion.z)>1.6 && posicion.y>4.9){
   float a=lado*(.65+.10*sin(locomocionClank.x+lado));
   mat3 giro=mat3(1,0,0,0,cos(a),sin(a),0,-sin(a),cos(a));
   float peso=smoothstep(1.6,2.5,abs(posicion.z));vec3 pivote=vec3(0,6.4,lado*1.4);
   local=mix(local,pivote+giro*(local-pivote),peso);n=normalize(mix(n,giro*n,peso));
  }
 }else if(locomocionClank.y>0.5){
  float lado=sign(posicion.x);
  if(posicion.y>7.0 && abs(posicion.x)>2.5){
   float a=-lado*(.95+.15*sin(locomocionClank.x+lado));
   mat3 giro=mat3(cos(a),sin(a),0,-sin(a),cos(a),0,0,0,1);
   float peso=smoothstep(2.5,3.7,abs(posicion.x));vec3 pivote=vec3(lado*2.5,10.4,0);
   local=mix(local,pivote+giro*(local-pivote),peso);n=normalize(mix(n,giro*n,peso));
  }else if(posicion.y<5.2){
   float a=lado*.28*sin(locomocionClank.x);mat3 giro=mat3(1,0,0,0,cos(a),sin(a),0,-sin(a),cos(a));
   float peso=1-smoothstep(4.2,5.2,posicion.y);vec3 pivote=vec3(lado*1.3,5.2,0);
   local=mix(local,pivote+giro*(local-pivote),peso);n=normalize(mix(n,giro*n,peso));
  }
 }
 vec4 p=modelo*vec4(local,1);posMundo=p.xyz;normalMundo=mat3(matrizNormal)*n;material=colorVertice;coordenada=uv;gl_Position=vistaProyeccion*p;
}
