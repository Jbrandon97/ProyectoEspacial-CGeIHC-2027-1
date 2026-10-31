#pragma once
#include "Graficos/Malla.h"
// Utilidades CPU para geometria propia: no conocen escenarios ni crean objetos GPU.
namespace Geometria {
void triangulo(std::vector<Vertice> &m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color);
void caja(std::vector<Vertice> &m, glm::vec3 centro, glm::vec3 dimensiones, glm::vec3 color);
void cono(std::vector<Vertice> &m, glm::vec3 base, float radio, float alto, glm::vec3 color,
          int lados = 8);
void elipsoide(std::vector<Vertice> &m, glm::vec3 centro, glm::vec3 radios, glm::vec3 color,
               int lados = 10, int anillos = 6);
} // namespace Geometria
