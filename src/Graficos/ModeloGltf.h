#pragma once
#include "Malla.h"
#include <filesystem>
#include <array>
#include <memory>
#include <string>
#include <vector>
class Renderizador;
class Textura {
    GLuint id_ = 0;

  public:
    explicit Textura(const std::filesystem::path &);
    ~Textura();
    Textura(const Textura &) = delete;
    Textura &operator=(const Textura &) = delete;
    GLuint id() const {
        return id_;
    }
};
// glTF estatico: geometria y materiales compartidos entre instancias regionales.
class ModeloGltf {
    struct Parte {
        std::unique_ptr<Malla> malla;
        std::shared_ptr<Textura> textura, emisionTextura;
        glm::mat4 local{1};
        glm::vec3 centro{0};
        float radio=0;
        glm::vec3 color{1}, emision{0};
        bool puerta = false;
        int etapa = -1;
        size_t indiceNodo = 0;
        float rugosidad = .85f, metal = 0;
    };
    std::vector<Parte> partes_;
    std::array<std::vector<glm::vec3>, 4> verticesEtapas_;
    std::vector<glm::vec3> triangulos_, puerta_;
    std::vector<std::string> nombresNodos_;
    std::vector<glm::vec3> origenesNodos_;
    float minimoY_ = 1e20f;

  public:
    const auto &verticesEtapas() const {
        return verticesEtapas_;
    }
    const std::vector<glm::vec3> &triangulos() const {
        return triangulos_;
    }
    const std::vector<glm::vec3> &puerta() const {
        return puerta_;
    }
    float minimoY() const noexcept {
        return minimoY_;
    }
    explicit ModeloGltf(const std::filesystem::path &, bool indexada = false,
                        bool conservarTriangulos = false);
    size_t cantidadNodos() const noexcept {
        return nombresNodos_.size();
    }
    size_t indiceNodo(const std::string &nombre) const;
    glm::vec3 origenNodo(size_t indice) const;
    void dibujar(Renderizador &, const glm::mat4 &, float elevacionPuerta = 0,
                 const std::array<glm::mat4, 4> *etapas = nullptr,
                 const std::vector<glm::mat4> *moverPartes = nullptr) const;
};
