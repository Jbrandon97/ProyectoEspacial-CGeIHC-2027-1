#include "Camaras/GestorCamaras.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
bool GestorCamaras::compatible(ModoCamara m, Region r) {
    switch (m) {
    case ModoCamara::Exploracion:
        return r != Region::Espacio;
    case ModoCamara::Carrera:
        return r == Region::Planeta1;
    case ModoCamara::Viaje:
        return true;
    case ModoCamara::Observacion:
        return r == Region::Espacio;
    }
    return false;
}
bool GestorCamaras::solicitar(ModoCamara m, Region r) {
    if (!compatible(m, r))
        return false;
    modo_ = m;
    return true;
}
void GestorCamaras::alCambiarRegion(Region r) {
    modo_ = r == Region::Espacio ? ModoCamara::Observacion : ModoCamara::Exploracion;
    angulo_ = 0;
    inclinacion_ = .28f;
    preparada_ = false;
}
void GestorCamaras::mirar(glm::vec2 d) {
    if (modo_ != ModoCamara::Carrera) {
        angulo_ += d.x * .003f;
        inclinacion_ = std::clamp(inclinacion_ + d.y * .0025f, -.85f, 1.25f);
    }
}
glm::vec2 GestorCamaras::movimientoRelativo(glm::vec2 e) const {
    glm::vec2 frente(std::sin(angulo_), -std::cos(angulo_)),
        derecha(std::cos(angulo_), std::sin(angulo_));
    auto v = derecha * e.x + frente * e.y;
    return glm::length(v) > 1 ? glm::normalize(v) : v;
}
void GestorCamaras::seguir(glm::vec3 p, glm::vec3 frente, float dt,
                           const std::function<float(float, float)> &altura) {
    glm::vec3 deseado;
    if (modo_ == ModoCamara::Carrera) {
        deseado = p - frente * 14.0f + glm::vec3(0, 6, 0);
        p += frente * 5.0f + glm::vec3(0, 1, 0);
    } else if (modo_ == ModoCamara::Observacion)
        deseado = p + glm::vec3(std::sin(angulo_) * 85, 52, std::cos(angulo_) * 85);
    else {
        float distancia = modo_ == ModoCamara::Viaje ? 24.0f : 10.0f;
        deseado =
            p + glm::vec3(-std::sin(angulo_) * distancia * std::cos(std::max(-.45f, inclinacion_)),
                          2 + distancia * std::sin(inclinacion_),
                          std::cos(angulo_) * distancia * std::cos(std::max(-.45f, inclinacion_)));
        // Al mirar arriba, elevar el enfoque conserva cielo visible aun cuando
        // el suelo limita la altura del ojo. El avatar sigue dentro del encuadre.
        p.y += .7f + (modo_ == ModoCamara::Exploracion ? std::max(0.f, -inclinacion_) * 4.5f : 0.f);
    }
    // Elevar el brazo antes de suavizar, luego asegurar que el ojo final no quede bajo el terreno.
    if (modo_ != ModoCamara::Observacion)
        for (int i = 1; i <= 12; ++i) {
            float t = float(i) / 12;
            auto q = glm::mix(p, deseado, t);
            float minimo = altura(q.x, q.z) + 1.25f;
            if (q.y < minimo)
                deseado.y += (minimo - q.y) / t;
        }
    float k = preparada_ ? 1 - std::exp(-8 * dt) : 1;
    ojo_ = glm::mix(ojo_, deseado, k);
    objetivo_ = glm::mix(objetivo_, p, k);
    if (modo_ != ModoCamara::Observacion)
        ojo_.y = std::max(ojo_.y, altura(ojo_.x, ojo_.z) + 1.25f);
    preparada_ = true;
}
glm::mat4 GestorCamaras::vista(glm::vec3 p) const {
    return preparada_ ? glm::lookAt(ojo_, objetivo_, glm::vec3(0, 1, 0))
                      : glm::lookAt(p + glm::vec3(0, 14, 25), p, glm::vec3(0, 1, 0));
}
glm::mat4 GestorCamaras::proyeccion(float a) const {
    return glm::perspective(glm::radians(60.0f), a, .15f, 900.0f);
}
const char *GestorCamaras::nombre() const {
    switch (modo_) {
    case ModoCamara::Exploracion:
        return "EXPLORACION";
    case ModoCamara::Carrera:
        return "CARRERA";
    case ModoCamara::Viaje:
        return "TRANSPORTE: VISTA PREVIA";
    default:
        return "OBSERVACION";
    }
}

void GestorCamaras::seguirEspacio(glm::vec3 nave, glm::vec3 frente, glm::vec3 centro, float dt) {
    if (modo_ == ModoCamara::Viaje && cabina_) {
        auto derecha = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));
        auto arriba = glm::cross(derecha, frente);
        // El piloto comparte la transformacion de la nave; su giro ya esta amortiguado.
        ojo_ = nave + arriba * 1.6f + frente * .55f;
        objetivo_ = ojo_ + frente * 20.f;
        preparada_ = true;
        return;
    }
    glm::vec3 ojo, objetivo;
    if (modo_ == ModoCamara::Observacion) {
        // Recorrido orbital libre alrededor de Gateway y vista de los cuerpos celestes.
        float elev = std::clamp(inclinacion_, .08f, 1.2f);
        ojo = centro + glm::vec3(std::sin(angulo_) * 285 * std::cos(elev), 285 * std::sin(elev),
                                 std::cos(angulo_) * 285 * std::cos(elev));
        objetivo = centro;
    } else {
        objetivo = nave + frente * 6.f;
        ojo = nave - frente * 18.f + glm::vec3(0, 7, 0);
    }
    float u = preparada_ ? 1 - std::exp(-6 * dt) : 1;
    ojo_ = glm::mix(ojo_, ojo, u);
    objetivo_ = glm::mix(objetivo_, objetivo, u);
    preparada_ = true;
}
