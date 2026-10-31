#version 330 core
out vec2 pantalla;
void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);pantalla=p*2-1;gl_Position=vec4(pantalla,1,1);}
