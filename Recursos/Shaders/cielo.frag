#version 330 core
in vec2 pantalla;
uniform mat4 inversaVP,rotDia,rotNoche;
uniform vec3 ojo,mezcla;
uniform sampler2D dia,noche;
out vec4 colorFinal;
vec2 uv(vec3 d){d=normalize(d);return vec2(atan(d.z,d.x)/6.2831853+.5,.5-asin(clamp(d.y,-1,1))/3.14159265);}
void main(){vec4 p=inversaVP*vec4(pantalla,1,1);vec3 r=normalize(p.xyz/p.w-ojo);vec3 a=texture(dia,uv(mat3(rotDia)*r)).rgb;vec3 b=texture(noche,uv(mat3(rotNoche)*r)).rgb;colorFinal=vec4(mix(a,b,mezcla.x),1);}
