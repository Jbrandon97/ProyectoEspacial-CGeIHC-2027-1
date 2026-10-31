#pragma once
#include "Graficos/Malla.h"
#include "Mundo/ActividadEscena.h"
// Dinamica arcade LIBRE en XZ. Solo consulta la pista para altura; no sigue su trayectoria.
class NaveCarreras {
    glm::vec3 posicion_{90, 3, 0};
    float rumbo_ = 0, velocidad_ = 0, velocidadGiro_ = 0;

  public:
    void colocar(float s, float lateral = 0);
    void actualizar(float dt, const OrdenActividad &orden);
    glm::vec3 posicion() const {
        return posicion_;
    }
    glm::vec3 direccion() const;
    float velocidad() const {
        return velocidad_;
    }
    void detener() {
        velocidad_ = 0;
        velocidadGiro_ = 0;
    }
    glm::mat4 matriz() const;
    static glm::mat4 orientar(glm::vec3 posicion, glm::vec3 frente, glm::vec3 normal);
};
