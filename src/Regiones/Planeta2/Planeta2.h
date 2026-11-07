#pragma once
#include "AlturasTerreno.h"
#include "Graficos/ModeloGltf.h"
#include "Mundo/Escenario.h"
#include "SectorRatchet/SectorRatchet.h"
#include "SectorWarframe/SectorWarframe.h"
#include <memory>

// Planeta 2 comparte el renderizador y las entradas del proyecto.
// Su escena, iluminacion y tiempo se administran localmente.
class Planeta2 final : public Escenario {
    SectorWarframe warframe_;
    SectorRatchet ratchet_;
    std::unique_ptr<ModeloGltf> terreno_;
    std::unique_ptr<AlturasTerreno> alturas_;
    float tiempo_ = 24.0f;

  public:
    Region id() const noexcept override { return Region::Planeta2; }
    const char *nombre() const noexcept override { return "Planeta2"; }
    void cargar(ContextoEscena &servicios) override;
    void actualizar(float deltaTime) override;
    void controlar(const OrdenActividad &orden) override;
    void dibujar(ContextoEscena &servicios, const glm::mat4 &alMundo) const override;
    void liberar() noexcept override;
    glm::vec3 aparicion() const noexcept override { return {4, 1, 24}; }
    float altura(float x, float z) const noexcept override {
        return alturas_ ? .60f * alturas_->altura(x / .60f, z / .60f) : 0.0f;
    }
    glm::vec3 ajustarMovimiento(glm::vec3 anterior, glm::vec3 propuesta) const override;
    Ambiente ambiente() const noexcept override;
};
