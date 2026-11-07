#include "Personajes/Avatar.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
void Avatar::aparecer(const Escenario &e) {
    sobrePlaneta1_ = e.id() == Region::Planeta1 || e.id() == Region::Planeta2;
    posicion_ = e.aparicion();
    posicion_.y = e.altura(posicion_.x, posicion_.z) + 1;
}
void Avatar::mover(glm::vec2 d, float dt, const Escenario &e, bool correr) {
    if (glm::length(d) > 1)
        d = glm::normalize(d);
    auto propuesta =
        posicion_ + glm::vec3(d.x, 0, d.y) *
                        (e.id() == Region::Planeta1 ? (correr ? 30.0f : 15.0f) : 9.0f) * dt;
    auto anterior = posicion_;
    posicion_ = e.ajustarMovimiento(posicion_, propuesta);
    float objetivo = glm::length(posicion_ - anterior) > .0001f ? (correr ? 1.0f : .45f) : 0.0f;
    actividad_ += glm::clamp(objetivo - actividad_, -6 * dt, 6 * dt);
    tiempo_ += dt * glm::mix(1.5f, 5.5f, actividad_);
    if (objetivo > 0)
        orientacion_ = std::atan2(-d.y, d.x);
    posicion_.y = e.altura(posicion_.x, posicion_.z) + 1;
}
void Avatar::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    if (sobrePlaneta1_) {
        if (!engineer_)
            engineer_ = std::make_unique<Engineer>(c.recursos / "Modelos/Engineer.gltf");
        engineer_->animar(tiempo_, actividad_);
        auto modelo = glm::translate(m, posicion_ + glm::vec3(0, .52f, 0));
        modelo = glm::rotate(modelo, orientacion_, glm::vec3(0, 1, 0));
        engineer_->dibujar(c.renderizador, glm::scale(modelo, glm::vec3(.9f)));
        return;
    }
    auto apoyo = posicion_ + glm::vec3(0, sobrePlaneta1_ ? -.3f : 0.0f, 0);
    c.renderizador.dibujar({apoyo, {1.4f, 1.4f, 1.4f}, {1, .8f, .15f}}, m);
    c.renderizador.dibujar({apoyo + glm::vec3(0, .9f, 0), {.7f, .3f, .7f}, {.2f, 1, 1}}, m);
}

void Avatar::colocar(glm::vec3 p, const Escenario &e) {
    sobrePlaneta1_ = e.id() == Region::Planeta1 || e.id() == Region::Planeta2;
    posicion_ = e.ajustarMovimiento(p, p);
    posicion_.y = e.altura(posicion_.x, posicion_.z) + 1;
}
