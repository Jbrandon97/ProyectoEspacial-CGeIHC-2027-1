#version 330 core
in vec3 color;
in float brillo;
out vec4 frag;

void main() {
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float halo = 1.0 - smoothstep(0.05, 1.0, dot(d, d));
    frag = vec4(color, brillo * halo);
}
