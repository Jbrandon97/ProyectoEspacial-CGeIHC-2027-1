#include "SectorRatchet.h"
#include "Graficos/Renderizador.h"
#include <glm/gtc/matrix_transform.hpp>

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    objetos_.push_back(
        {{0, .1f, 0}, {30, .2f, 45}, {.15f, .7f, .9f}}); // plataforma (superficie en y = 0.2)
    edificio1_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio1.gltf");
    edificio2_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio2.gltf");
}

void SectorRatchet::actualizar(float) {} // Aqui ira la nave.

void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
    if (edificio1_)
        edificio1_->dibujar(c.renderizador, glm::translate(m, {-8, .2f, -10}));
    if (edificio2_)
        edificio2_->dibujar(c.renderizador, glm::translate(m, {8, .2f, -10}));
}

void SectorRatchet::liberar() noexcept {
    std::vector<Objeto>().swap(objetos_);
    edificio1_.reset();
    edificio2_.reset();
}