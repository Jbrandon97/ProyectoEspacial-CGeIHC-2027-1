#pragma once
#include "ColisionesPlaneta1.h"
#include "GestorCarrera.h"
#include "Graficos/Malla.h"
#include "Graficos/ModeloGltf.h"
#include "Mundo/Escenario.h"
#include "SistemasPlaneta1.h"
#include "Titan.h"
#include <map>
#include <memory>
// Composicion regional: terreno, paisaje, pista y competencia comparten servicios graficos.
class Planeta1 final : public Escenario {
    std::unique_ptr<Malla> terreno_, paisaje_, agua_, pista_, detalles_;
    std::unique_ptr<Titan> titan_;
    std::unique_ptr<Malla> humo_, marcasPista_;
    GestorCarrera carrera_;
    SistemasPlaneta1 sistemas_;
    std::unique_ptr<Malla> propulsion_, hojaCompuerta_, marcoCompuerta_;
    ColisionesPlaneta1 colisionCohete_;
    unsigned eventoPuerta_ = 0;
    ColisionesPlaneta1 colisiones_, colisionPuerta_;
    std::unique_ptr<Malla> rampaPeatonal_;
    float puertaAltura_ = 7, puertaDestino_ = 7;
    glm::vec3 visitante_{0};
    void actualizarPuerta();
    // Una geometria Banshee y una Ghost compartida por ambos rivales.
    std::unique_ptr<ModeloGltf> banshee_, ghost_;
    std::map<std::string, std::unique_ptr<ModeloGltf>> modelos_;
    std::unique_ptr<Textura> suelo_, texturaAgua_;

  public:
    const Titan &titan() const {
        return *titan_;
    }
    Region id() const noexcept override {
        return Region::Planeta1;
    }
    const char *nombre() const noexcept override {
        return "Planeta1";
    }
    void cargar(ContextoEscena &) override;
    void actualizar(float dt) override;
    void controlar(const OrdenActividad &orden) override;
    EstadoActividad actividad() const override;
    float elevacionPuerta() const {
        return puertaAltura_;
    }
    void dibujar(ContextoEscena &, const glm::mat4 &) const override;
    void liberar() noexcept override;
    glm::vec3 aparicion() const noexcept override {
        return {198, 2, 139};
    }
    float altura(float x, float z) const noexcept override;
    glm::vec3 ajustarMovimiento(glm::vec3 anterior, glm::vec3 propuesta) const override;
    Ambiente ambiente() const noexcept override {
        Ambiente a{{.21f, .34f, .43f}, {.5f, 1, .3f}};
        a.iluminacion = sistemas_.luces();
        a.cielo = "Planeta1";
        return a;
    }
    SistemasPlaneta1 &sistemas() {
        return sistemas_;
    }
    const SistemasPlaneta1 &sistemas() const {
        return sistemas_;
    }
    unsigned eventoPuerta() const {
        return eventoPuerta_;
    }
    const GestorCarrera &carrera() const {
        return carrera_;
    }
};
