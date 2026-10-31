#pragma once
#include "Engineer.h"
#include "Mundo/Escenario.h"
// Engineer articulado en Planeta 1; otras regiones conservan su marcador.
class Avatar {
    glm::vec3 posicion_{0};
    bool sobrePlaneta1_ = false;
    float tiempo_ = 0, actividad_ = 0, orientacion_ = 0;
    mutable std::unique_ptr<Engineer> engineer_;

  public:
    void colocar(glm::vec3 posicion, const Escenario &region);
    void aparecer(const Escenario &region);
    void mover(glm::vec2 direccion, float segundos, const Escenario &region, bool correr = false);
    glm::vec3 posicion() const noexcept {
        return posicion_;
    }
    void dibujar(ContextoEscena &c, const glm::mat4 &alMundo) const;
};
