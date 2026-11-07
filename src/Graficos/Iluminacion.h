#pragma once
#include <array>
#include <glm/glm.hpp>
struct LuzPuntual {
    glm::vec3 posicion{0}, color{1};
    float intensidad = 0;
    glm::vec3 atenuacion{1, .09f, .018f};
    bool encendida = false;
};
struct Iluminacion {
    bool activa = false;
    glm::vec3 sol{0, 1, 0}, colorSol{1};
    float fuerzaSol = 1, ambiente = .2f, noche = 0;
    std::array<LuzPuntual, 16> puntuales{};
    bool foco = false;
    glm::vec3 focoPos{0}, focoDir{0, 0, -1};
};
