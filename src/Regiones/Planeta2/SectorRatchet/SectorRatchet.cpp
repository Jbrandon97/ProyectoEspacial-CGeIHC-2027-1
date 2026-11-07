#include "SectorRatchet.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
const glm::vec3 POS_EDIFICIO1 = {-8, 0, -10}; // la Y se calcula con el terreno
const glm::vec3 POS_EDIFICIO2 = {8, 0, -10};
const glm::vec2 MEDIO_EDIFICIO1 = {2.3f, 1.8f}; // mitad de 4.57 x 3.52
const glm::vec2 MEDIO_EDIFICIO2 = {3.4f, 4.7f}; // mitad de 6.76 x 9.4
} // namespace

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    edificio1_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio1.gltf");
    edificio2_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio2.gltf");
    // apoyo: el punto mas bajo bajo la base, para que ninguna esquina quede flotando
    auto apoyo = [this](glm::vec3 centro, glm::vec2 medio) {
        if (!suelo_)
            return 0.0f;
        float h = suelo_(centro.x, centro.z);
        for (float sx : {-medio.x, medio.x})
            for (float sz : {-medio.y, medio.y})
                h = std::min(h, suelo_(centro.x + sx, centro.z + sz));
        return h;
    };
    yEdificio1_ = apoyo(POS_EDIFICIO1, MEDIO_EDIFICIO1);
    yEdificio2_ = apoyo(POS_EDIFICIO2, MEDIO_EDIFICIO2);
}

void SectorRatchet::actualizar(float) {} // Aqui ira la nave.

void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
    if (edificio1_)
        edificio1_->dibujar(c.renderizador,
                            glm::translate(m, {POS_EDIFICIO1.x, yEdificio1_, POS_EDIFICIO1.z}));
    if (edificio2_)
        edificio2_->dibujar(c.renderizador,
                            glm::translate(m, {POS_EDIFICIO2.x, yEdificio2_, POS_EDIFICIO2.z}));
}

void SectorRatchet::liberar() noexcept {
    std::vector<Objeto>().swap(objetos_);
    edificio1_.reset();
    edificio2_.reset();
}

bool SectorRatchet::obstaculo(glm::vec3 p) const {
    auto dentro = [&](glm::vec3 centro, glm::vec2 medio) {
        return std::abs(p.x - centro.x) < medio.x + .5f && std::abs(p.z - centro.z) < medio.y + .5f;
    };
    return dentro(POS_EDIFICIO1, MEDIO_EDIFICIO1) || dentro(POS_EDIFICIO2, MEDIO_EDIFICIO2);
}