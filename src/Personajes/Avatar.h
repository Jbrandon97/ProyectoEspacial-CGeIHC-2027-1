#pragma once
#include "Engineer.h"
#include <cmath>
#include "Mundo/Escenario.h"
// Engineer articulado compartido por Planeta 1 y Planeta 2.
class Avatar {
    glm::vec3 posicion_{0};
    bool sobrePlaneta1_ = false;
    float tiempo_ = 0, actividad_ = 0, orientacion_ = 0;
    mutable std::unique_ptr<Engineer> engineer_;

  public:
    struct Estado {
        glm::vec3 posicion;
        float tiempo, actividad, orientacion;
        bool planeta1;
    };
    Estado guardar() const {
        return {posicion_, tiempo_, actividad_, orientacion_, sobrePlaneta1_};
    }
    void restaurar(const Estado &e) {
        posicion_ = e.posicion;
        tiempo_ = e.tiempo;
        actividad_ = e.actividad;
        orientacion_ = e.orientacion;
        sobrePlaneta1_ = e.planeta1;
    }

    void colocar(glm::vec3 posicion, const Escenario &region);
    void aparecer(const Escenario &region);
    void mover(glm::vec2 direccion, float segundos, const Escenario &region, bool correr = false);
    glm::vec3 posicion() const noexcept {
        return posicion_;
    }
    glm::vec3 direccion() const {
        return {std::cos(orientacion_), 0, -std::sin(orientacion_)};
    }
    float actividad() const {
        return actividad_;
    }
    void dibujar(ContextoEscena &c, const glm::mat4 &alMundo, bool energia = false) const;
};
