#pragma once
#include "Graficos/Malla.h"
#include "Graficos/ModeloGltf.h"
#include "Graficos/Shader.h"
#include "Mundo/Tipos.h"
#include <memory>
// Servicio GPU unico. Las regiones envian instancias, no crean renderizadores.
class Renderizador {
    Shader shader_;
    Malla cubo_;
    Shader cielo_;
    std::unique_ptr<Textura> cieloDia_, cieloNoche_;
    std::filesystem::path recursos_;
    GLuint vaoCielo_ = 0;
    std::string seleccionCielo_;
    unsigned dibujos_ = 0;
    glm::mat4 vp_{1};

  public:
    explicit Renderizador(const std::filesystem::path &recursos);
    void iniciar(int ancho, int alto, const glm::mat4 &vistaProyeccion, Ambiente ambiente,
                 glm::vec3 ojo = glm::vec3(0));
    ~Renderizador();
    void material(float rugosidad, float metal = 0);
    void dibujar(const Objeto &objeto, const glm::mat4 &alMundo);
    void dibujar(const Malla &malla, const glm::mat4 &modelo, glm::vec3 tinte = glm::vec3(1));
    void texturada(const Malla &, const glm::mat4 &, GLuint textura, glm::vec3 color,
                   glm::vec3 emision = glm::vec3(0), float rugosidad = .85f, float metal = 0);
    bool visible(glm::vec3 centro, float radio) const;
    void interfaz(const Malla &malla, int ancho, int alto);
    void capturar(const std::filesystem::path &archivo, int ancho, int alto) const;
    unsigned dibujos() const noexcept {
        return dibujos_;
    }
    static void comprobarErrores();
};
