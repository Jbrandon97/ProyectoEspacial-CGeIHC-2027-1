#pragma once
#include "Graficos/ModeloGltf.h"
#include <array>
// Deformacion por piel: diez articulaciones, tres cadenas originales y cuerpo fijo.
class Engineer {
    struct Piel {
        std::vector<Vertice> base;
        std::vector<glm::ivec4> juntas;
        std::vector<glm::vec4> pesos;
        std::unique_ptr<Malla> malla;
        std::unique_ptr<Textura> textura;
    };
    std::vector<Piel> partes_;
    std::array<float, 4> desplazamientos_{};
    std::array<glm::mat4, 10> inversas_, locales_, pose_;
    std::array<int, 10> padres_{};

  public:
    explicit Engineer(const std::filesystem::path &);
    void animar(float tiempo, float actividad);
    void dibujar(Renderizador &, const glm::mat4 &) const;
    float desplazamientoExtremidad() const;
    float deformacionMaxima(int grupo) const {
        return desplazamientos_.at(grupo);
    }
};
