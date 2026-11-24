#version 330 core
layout(location = 0) in vec3 posicion;
layout(location = 2) in vec3 colorVertice;

uniform mat4 vistaProyeccion, modelo;
uniform vec3 efecto;
out vec3 color;
out float brillo;

void main() {
    float azar = fract(sin(float(gl_VertexID) * 12.9898 + 17.0) * 43758.5453);
    gl_Position = vistaProyeccion * modelo * vec4(posicion, 1.0);
    gl_PointSize = mix(1.25, 2.15, efecto.y) + 1.7 * pow(azar, 9.0);
    color = colorVertice;
    brillo = efecto.x * (0.07 + 0.37 * pow(azar, 5.0))
             * (0.91 + 0.09 * cos(efecto.z * 0.62831853 + azar * 58.0));
}
