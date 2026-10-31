#pragma once
#include <glm/glm.hpp>
// Rival automatico: distancia acumulada y carril fijo, velocidad reducida en curvas.
class RivalCarrera {
    float avance_ = 9, rapidez_ = 37, carril_ = 0, velocidad_ = 0;

  public:
    RivalCarrera(float rapidez = 27, float carril = 0) : rapidez_(rapidez), carril_(carril) {}
    void reiniciar() {
        avance_ = 9;
        velocidad_ = 0;
    }
    void actualizar(float dt);
    float velocidad() const {
        return velocidad_;
    }
    float avance() const {
        return avance_;
    }
    glm::vec3 posicion() const;
    glm::mat4 matriz() const;
};
