#pragma once
#include "AlturasTerreno.h"
#include "Graficos/ModeloGltf.h"
#include "Mundo/Escenario.h"
#include "SectorRatchet/SectorRatchet.h"
#include "SectorWarframe/SectorWarframe.h"
#include <memory>
#include <vector>
// Region Planeta2: terreno compartido + sectores Warframe y Ratchet.
class Planeta2 final : public Escenario {
    std::vector<Objeto> objetos_;
    SectorWarframe warframe_;
    SectorRatchet ratchet_;
    std::unique_ptr<ModeloGltf> terreno_;     // malla del terreno (Terreno.gltf)
    std::unique_ptr<AlturasTerreno> alturas_; // alturas para avatar y objetos

  public:
    Region id() const noexcept override {
        return Region::Planeta2;
    }
    const char *nombre() const noexcept override {
        return "Planeta2";
    }
    void cargar(ContextoEscena &servicios) override;
    void actualizar(float deltaTime) override;
    void dibujar(ContextoEscena &servicios, const glm::mat4 &alMundo) const override;
    void liberar() noexcept override;
    glm::vec3 aparicion() const noexcept override {
        return {0, 1, 20};
    }
    float altura(float x, float z) const noexcept override {
        return alturas_ ? alturas_->altura(x, z) : 0.0f;
    }
    glm::vec3 ajustarMovimiento(glm::vec3 anterior, glm::vec3 propuesta) const override;
    Ambiente ambiente() const noexcept override;
};