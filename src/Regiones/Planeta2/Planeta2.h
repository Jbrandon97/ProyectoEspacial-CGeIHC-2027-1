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
    bool reflector_ = true, reflectorOrokin_ = false, reflectoresRailjack_ = true;
    float alturaTerreno(float x, float z) const noexcept;

  public:
    const SectorRatchet &ratchet() const {
        return ratchet_;
    }
    glm::vec3 centroPlaza() const;
    float reloj() const {
        return tiempo_;
    }
    Region id() const noexcept override {
        return Region::Planeta2;
    }
    const char *nombre() const noexcept override {
        return "Planeta2";
    }
    void cargar(ContextoEscena &servicios) override;
    void actualizar(float deltaTime) override;
    void controlar(const OrdenActividad &orden) override;
    void dibujar(ContextoEscena &servicios, const glm::mat4 &alMundo) const override;
    void liberar() noexcept override;
    glm::vec3 aparicion() const noexcept override {
        return {-42, 1, 115};
    }
    float altura(float x, float z) const noexcept override;
    glm::vec3 ajustarMovimiento(glm::vec3 anterior, glm::vec3 propuesta) const override;
    Ambiente ambiente() const noexcept override;
    EstadoActividad actividad() const override;
};
