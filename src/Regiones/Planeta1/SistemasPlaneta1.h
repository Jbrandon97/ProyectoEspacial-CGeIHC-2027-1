#pragma once
#include "Mundo/Tipos.h"
#include <filesystem>
// Estado regional compartido por dibujo, camara, pruebas y eventos. Sin dependencias de GLFW.
class SistemasPlaneta1 {
  public:
    float tiempo = 0, ciclo = 120, faseInicial = .22f, lanzamiento = -1, reaccion = -1;
    unsigned eventoCohete = 0, eventoGrunt = 0;
    void configurar(const std::filesystem::path &);
    void actualizar(float dt);
    void lanzar() {
        if (lanzamiento < 0) {
            lanzamiento = 0;
            ++eventoCohete;
        }
    }
    void reiniciar() {
        lanzamiento = -1;
    }
    float fase() const;
    float faseSolar() const;
    Iluminacion luces() const;
    glm::vec3 cohete() const;
    glm::vec3 sentinel(int i) const;
    float giroSentinel(int i) const;
    glm::mat4 animacionNPC(int i) const;
    bool motor() const {
        return lanzamiento >= 1 && lanzamiento < 17;
    }
};
