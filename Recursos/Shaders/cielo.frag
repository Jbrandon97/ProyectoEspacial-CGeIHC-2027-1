#version 330 core
in vec2 pantalla;
uniform mat4 inversaVP,rotDia,rotNoche;
uniform vec3 ojo,mezcla;
uniform sampler2D dia,noche;
out vec4 colorFinal;
vec2 uv(vec3 d){d=normalize(d);return vec2(atan(d.z,d.x)/6.2831853+.5,.5-asin(clamp(d.y,-1,1))/3.14159265);}
vec3 panorama(sampler2D mapa, vec3 dir){
 vec2 q=uv(dir),dx=dFdx(q),dy=dFdy(q);
 dx.x-=round(dx.x);dy.x-=round(dy.x);
 return textureGrad(mapa,q,dx,dy).rgb;
}
void main(){vec4 p=inversaVP*vec4(pantalla,1,1);vec3 r=normalize(p.xyz/p.w-ojo);vec3 a=panorama(dia,mat3(rotDia)*r);vec3 b=panorama(noche,mat3(rotNoche)*r);vec3 tinte=mix(vec3(1),mix(vec3(.72,.85,1.0),vec3(.85,.63,1.0),mezcla.x),mezcla.z);colorFinal=vec4(mix(a,b,mezcla.x)*mezcla.y*tinte,1);}
