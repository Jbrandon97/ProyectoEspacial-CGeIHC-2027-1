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
 if(locomocionClank.y>1.5){
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
