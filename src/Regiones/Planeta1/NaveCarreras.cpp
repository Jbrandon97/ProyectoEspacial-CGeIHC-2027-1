#include "NaveCarreras.h"
#include "CircuitoCarreras.h"
#include "Graficos/Geometria.h"
#include "TerrenoPlaneta1.h"
#include <algorithm>
#include <cmath>
void NaveCarreras::colocar(float s, float lateral) {
    auto m = CircuitoCarreras::muestra(s, lateral);
    posicion_ = m.posicion;
    rumbo_ = std::atan2(m.tangente.x, -m.tangente.z);
    detener();
}
glm::vec3 NaveCarreras::direccion() const {
    return {std::sin(rumbo_), 0, -std::cos(rumbo_)};
}
void NaveCarreras::actualizar(float dt, const OrdenActividad &o) {
    auto pr = CircuitoCarreras::proyectar(posicion_);
    bool fuera = pr.distancia > CircuitoCarreras::Ancho / 2;
    // Integracion en subpasos de 1/120 en GestorCarrera: aceleracion, rozamiento y giro gradual.
    velocidad_ += (o.acelerador * 18 - o.freno * 28 - velocidad_ * (fuera ? 1.5f : .18f)) * dt;
    velocidad_ = std::clamp(velocidad_, -9.0f, 46.0f);
    if (!o.acelerador && !o.freno && std::abs(velocidad_) < .12f)
        velocidad_ = 0;
    float giroDeseado = o.giro * 1.65f * std::clamp(std::abs(velocidad_) / 16, .0f, 1.0f) *
                        (velocidad_ < 0 ? -1.0f : 1.0f);
    velocidadGiro_ += (giroDeseado - velocidadGiro_) * (1 - std::exp(-9 * dt));
    rumbo_ += velocidadGiro_ * dt;
    posicion_ += direccion() * velocidad_ * dt;
    pr = CircuitoCarreras::proyectar(posicion_);
    auto global = posicion_ + CircuitoCarreras::origen();
    float suelo = TerrenoPlaneta1::altura(global.x, global.z);
    float objetivo = pr.distancia <= CircuitoCarreras::Ancho / 2 + .7f
                         ? CircuitoCarreras::muestra(pr.s).posicion.y
                         : suelo;
    posicion_.y = objetivo;
}
glm::mat4 NaveCarreras::orientar(glm::vec3 p, glm::vec3 f, glm::vec3 n) {
    f = glm::normalize(f - n * glm::dot(f, n));
    auto derecha = glm::normalize(glm::cross(f, n));
    glm::mat4 m(1);
    m[0] = glm::vec4(derecha, 0);
    m[1] = glm::vec4(n, 0);
    m[2] = glm::vec4(-f, 0);
    m[3] = glm::vec4(p + n * .8f, 1);
    return m;
}
glm::mat4 NaveCarreras::matriz() const {
    auto pr = CircuitoCarreras::proyectar(posicion_);
    auto g = posicion_ + CircuitoCarreras::origen();
    auto n = pr.distancia < CircuitoCarreras::Ancho / 2 + .7f
                 ? CircuitoCarreras::muestra(pr.s).normal
                 : TerrenoPlaneta1::normal(g.x, g.z);
    return orientar(posicion_, direccion(), n);
}
