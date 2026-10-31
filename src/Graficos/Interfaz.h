#pragma once
#include "Graficos/Malla.h"
#include <string>
// HUD vectorial ligero; tipografia bitmap propia de 5x7, sin DLL ni fuentes externas.
class Interfaz {
    Malla malla_{std::vector<Vertice>{}};
    std::vector<Vertice> vertices_;
    void rectangulo(float x, float y, float ancho, float alto, glm::vec3 color);
    void texto(const std::string &texto, float x, float y, float escala, glm::vec3 color);

  public:
    void preparar(const std::vector<std::string> &lineas, int ancho, int alto, bool pie = true);
    const Malla &malla() const {
        return malla_;
    }
};
