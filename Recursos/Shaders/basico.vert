#version 330 core
layout(location=0) in vec3 posicion;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 colorVertice;
layout(location=3) in vec2 uv;
uniform mat4 modelo,vistaProyeccion;
out vec3 normalMundo,material,posMundo;
out vec2 coordenada;
void main(){vec4 p=modelo*vec4(posicion,1);posMundo=p.xyz;normalMundo=mat3(transpose(inverse(modelo)))*normal;material=colorVertice;coordenada=uv;gl_Position=vistaProyeccion*p;}
