#pragma once
#include "Mundo/ContextoEscena.h"
#include "Mundo/Tipos.h"
#include <vector>
// Submodulo de Planeta2. Sus coordenadas son locales al sector; Planeta2 lo coloca.
class SectorWarframe {
    std::vector<Objeto> objetos_;

  public:
    void cargar(ContextoEscena &servicios);
    void actualizar(float deltaTime);
    void dibujar(ContextoEscena &servicios, const glm::mat4 &sectorAlMundo) const;
    void liberar() noexcept;
};
