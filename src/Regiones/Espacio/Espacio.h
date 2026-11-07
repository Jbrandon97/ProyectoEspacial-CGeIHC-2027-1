#pragma once
#include "Mundo/Escenario.h"
#include "Graficos/ModeloGltf.h"
#include <optional>
// Modulo independiente: coordenadas locales, tiempo propio y recursos cargados una vez.
class Espacio final : public Escenario {
    std::unique_ptr<ModeloGltf> gateway_, nave_;
    std::unique_ptr<Malla> esfera_, satelite_;
    std::unique_ptr<Textura> halo_, orokin_, luna_;
    glm::vec3 posicion_{-210, 0, 0};
    float tiempo_ = 0, rumbo_ = 1.5707963f, velocidad_ = 0;
    float rumboObjetivo_ = 1.5707963f, cabeceo_ = 0, cabeceoObjetivo_ = 0, ascenso_ = 0;
    bool exterior_ = true;
    OrdenActividad orden_{};
    std::optional<Region> destino_;

  public:
    Region id() const noexcept override {
        return Region::Espacio;
    }
    const char *nombre() const noexcept override {
        return "Espacio";
    }
    void cargar(ContextoEscena &) override;
    void actualizar(float dt) override;
    void dibujar(ContextoEscena &, const glm::mat4 &) const override;
    void liberar() noexcept override;
    glm::vec3 aparicion() const noexcept override {
        return posicion_;
    }
    float altura(float, float) const noexcept override {
        return 0;
    }
    Ambiente ambiente() const noexcept override;
    void controlar(const OrdenActividad &) override;
    EstadoActividad actividad() const override;
    glm::vec3 direccion() const;
    glm::mat4 matrizNave() const;
    void mostrarNave(bool valor) { exterior_ = valor; }
    float velocidad() const { return velocidad_; }
    glm::vec3 trafico(float segundos) const;
    glm::vec3 ajustarMovimiento(glm::vec3 anterior, glm::vec3 propuesta) const override;
    std::optional<Region> consumirDestino();
    float reloj() const {
        return tiempo_;
    }
};
