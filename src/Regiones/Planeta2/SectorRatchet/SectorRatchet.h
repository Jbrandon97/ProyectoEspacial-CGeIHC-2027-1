#pragma once
#include "Graficos/ModeloGltf.h"
#include "Mundo/ContextoEscena.h"
#include "Mundo/Tipos.h"
#include <memory>
#include <vector>
// Submodulo de Planeta2. Sus coordenadas son locales al sector; Planeta2 lo coloca.
class SectorRatchet {
    std::vector<Objeto> objetos_;
    std::unique_ptr<ModeloGltf> edificio1_, edificio2_; // Metropolis (Kerwan)

  public:
    void cargar(ContextoEscena &servicios);
    void actualizar(float deltaTime);
    void dibujar(ContextoEscena &servicios, const glm::mat4 &sectorAlMundo) const;
    void liberar() noexcept;
};