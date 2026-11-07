#pragma once
#include "Graficos/ModeloGltf.h"
#include "Mundo/ContextoEscena.h"
#include "Mundo/Tipos.h"
#include <functional>
#include <memory>
#include <vector>
// Submodulo de Planeta2. Sus coordenadas son locales al sector; Planeta2 lo coloca.
class SectorRatchet {
    std::vector<Objeto> objetos_;
    std::unique_ptr<ModeloGltf> edificio1_, edificio2_; // Metropolis (Kerwan)
    std::unique_ptr<ModeloGltf> railjack_;              // Railjack animado
    std::unique_ptr<ModeloGltf> orokintower_;           // Torre Orokin
    std::unique_ptr<ModeloGltf> warframe_;           // Warframe
    std::function<float(float, float)> suelo_;          // altura del terreno en coords locales
    float yEdificio1_ = 0, yEdificio2_ = 0;

  public:
    void usarSuelo(std::function<float(float, float)> suelo) {
        suelo_ = std::move(suelo);
    }
    void cargar(ContextoEscena &servicios);
    void actualizar(float deltaTime);
    void dibujar(ContextoEscena &servicios, const glm::mat4 &sectorAlMundo) const;
    void liberar() noexcept;
    bool obstaculo(glm::vec3 posLocal) const; // true si choca con un edificio
};