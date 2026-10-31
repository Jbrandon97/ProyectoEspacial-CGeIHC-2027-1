#pragma once
#include "Mundo/Escenario.h"
#include "SectorRatchet/SectorRatchet.h"
#include "SectorWarframe/SectorWarframe.h"
#include <vector>
// Region provisional Planeta2; construye instancias CPU y utiliza la malla GPU compartida.
class Planeta2 final : public Escenario {
    std::vector<Objeto> objetos_;
    SectorWarframe warframe_;
    SectorRatchet ratchet_;

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
    float altura(float, float) const noexcept override {
        return 0;
    }
    Ambiente ambiente() const noexcept override;
};
