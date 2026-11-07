#pragma once
#include "Graficos/ModeloGltf.h"
#include "Mundo/ContextoEscena.h"
#include <array>
#include <functional>
#include <memory>

// Objetos de Ratchet y Warframe: se usan coordenadas locales al sector.
// Ningun objeto de esta clase modifica Planeta 1 ni Espacio.
class SectorRatchet {
    std::unique_ptr<ModeloGltf> edificio1_, edificio2_, edificio3_;
    std::unique_ptr<ModeloGltf> railjack_, orokintower_, warframe_, aphelion_;
    std::unique_ptr<ModeloGltf> clank_, blarg_;
    std::unique_ptr<Malla> arbol_, vallas_, humo_, propulsor_;
    std::function<float(float, float)> suelo_;
    std::array<float, 6> apoyos_{};
    glm::vec3 avatarLocal_{500, 0, 500};
    float tiempo_ = 0;

  public:
    void usarSuelo(std::function<float(float, float)> suelo) { suelo_ = std::move(suelo); }
    void cargar(ContextoEscena &servicios);
    void actualizar(float deltaTime);
    void registrarAvatar(glm::vec3 p) { avatarLocal_ = p; }
    void dibujar(ContextoEscena &servicios, const glm::mat4 &sectorAlMundo) const;
    void liberar() noexcept;
    bool obstaculo(glm::vec3 posLocal) const;
    glm::vec3 aphelionPosicion() const;
    glm::vec3 railjackPosicion() const;
};
