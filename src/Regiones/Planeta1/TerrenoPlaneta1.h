#pragma once
#include "Graficos/Malla.h"
// Altura determinista compartida por malla, avatar, camara y objetos del paisaje.
class TerrenoPlaneta1 {
  public:
    static float altura(float x, float z);
    static glm::vec3 normal(float x, float z);
    static bool lago(float x, float z, float margen = 0);
    static bool reservaPista(float x, float z, float margen = 0);
    static glm::vec3 permitir(glm::vec3 anterior, glm::vec3 propuesta);
    static std::vector<Vertice> construir();
    static std::vector<Vertice> paisaje();
    static std::vector<Vertice> agua();
};
