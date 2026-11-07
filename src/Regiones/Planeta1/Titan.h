#pragma once
#include "Graficos/ModeloGltf.h"
// Rig original: pesos y jerarquia intactos; claves articulares propias de la caminata.
class Titan {
    struct Nodo {
        glm::mat4 local{1}, global{1}, reposo{1};
        int padre = -1;
        std::string nombre;
    };
    struct Piel {
        std::vector<Vertice> base, deformada, dibujo;
        std::vector<glm::ivec4> juntas;
        std::vector<glm::vec4> pesos;
        std::vector<unsigned> indices;
        std::unique_ptr<Malla> malla;
        std::shared_ptr<Textura> textura;
        glm::vec3 color{1};
    };
    std::vector<Nodo> nodos_;
    std::vector<int> juntas_;
    std::vector<glm::mat4> inversas_;
    std::vector<Piel> pieles_;
    float escala_ = 1, apoyo_ = 0, ultimo_ = -100, ultimoSaludo_ = -100, desplazamiento_ = 0;

  public:
    explicit Titan(const std::filesystem::path &);
    void animar(float tiempo, float saludo = -1);
    void dibujar(Renderizador &, const glm::mat4 &) const;
    glm::vec3 extremo(const std::string &nombre) const;
    float alturaMinima() const;
    float desplazamiento() const {
        return desplazamiento_;
    }
    static glm::vec3 posicion(float tiempo);
    static float giro(float tiempo);
};
