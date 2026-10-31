#pragma once
#include "Graficos/Malla.h"
// Referencia analitica 537.0796; recorrido fisico 483.3717. s se expresa en metros fisicos.
struct MuestraCircuito {
    glm::vec3 posicion, tangente, normal;
};
struct ProyeccionCircuito {
    float s, distancia, lateral;
};
class CircuitoCarreras {
  public:
    static constexpr float Pi = 3.14159265358979323846f;
    static constexpr float Escala = .9f;
    static constexpr float LongitudReferencia = 380 + 50 * Pi;
    static constexpr float Longitud = Escala * LongitudReferencia;
    static constexpr float Ancho = 14 * Escala;
    // Traslacion del circuito ya escalado al sector sureste de Planeta 1.
    static glm::vec3 origen() {
        return {112, 0, 135};
    }
    static float envolver(float s);
    static float envolverReferencia(float s);
    static MuestraCircuito muestraReferencia(float s, float lateral = 0);
    static ProyeccionCircuito proyectarReferencia(glm::vec3 punto);
    static float rampa(float x, float z); // Altura relativa, 6 en norte, 4 en sur.
    static float pendiente(float x, float z);
    static MuestraCircuito muestra(float s, float lateral = 0);
    static ProyeccionCircuito proyectar(glm::vec3 puntoLocal);
    static std::vector<Vertice> construir();
    static std::vector<Vertice> detalles();
};
