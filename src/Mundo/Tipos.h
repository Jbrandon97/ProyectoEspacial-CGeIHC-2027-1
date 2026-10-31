#pragma once
#include "Graficos/Iluminacion.h"
#include <glm/glm.hpp>
#include <string>
// Identificadores estables, independientes de las teclas y de las clases concretas.
enum class Region { Planeta1, Planeta2, Espacio };
enum class ModoCamara { Exploracion, Carrera, Viaje, Observacion };
// Instancia ligera de la malla compartida. Posicion y dimensiones en unidades locales.
struct Objeto {
    glm::vec3 posicion;
    glm::vec3 escala;
    glm::vec3 color;
    float giro = 0.0f;
};
struct Ambiente {
    glm::vec3 fondo;
    glm::vec3 luz;
    Iluminacion iluminacion{};
    std::string cielo;
};
