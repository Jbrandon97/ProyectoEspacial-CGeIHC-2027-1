#pragma once
#include "Graficos/Malla.h"
#include <functional>
#include <map>
// Malla estatica real + rejilla XZ: permite rampas y respeta paredes sin cajas de exclusion
// amplias.
class ColisionesPlaneta1 {
    struct Triangulo {
        glm::vec3 a, b, c, n;
    };
    std::vector<Triangulo> triangulos_;
    std::map<std::pair<int, int>, std::vector<size_t>> celdas_;
    const std::vector<size_t> &celda(float x, float z) const;

  public:
    void limpiar();
    void agregar(const std::vector<glm::vec3> &, glm::vec3 desplazamiento = {0, 0, 0});
    void agregar(const std::vector<Vertice> &, glm::vec3 desplazamiento = {0, 0, 0});
    float apoyo(float x, float z, float base) const;
    bool obstaculo(glm::vec3 pies) const;
    glm::vec3 mover(glm::vec3 previo, glm::vec3 destino,
                    const std::function<float(float, float)> &altura) const;
};
