#include "SectorRatchet.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
const glm::vec3 POS_EDIFICIO1 = {-10, 0, -10};
const glm::vec3 POS_EDIFICIO2 = {12, 0, -10};
const glm::vec2 MEDIO_EDIFICIO1 = {2.3f, 1.8f}; // mitad de 4.57 x 3.52
const glm::vec2 MEDIO_EDIFICIO2 = {3.4f, 4.7f}; // mitad de 6.76 x 9.4
const float ESCALA_EDIFICIOS = 2.0f;            // 1 = tamano original (~14-16 de alto)

// Configuración del Railjack
const glm::vec3 POS_RAILJACK = {-200.0f, 15.0f, 0.0f}; // Posición en la escena
const float ESCALA_RAILJACK = 0.5f;                // Escala de la nave

// Configuración de la torre Orokin
const glm::vec3 POS_OROKINTOWER = {-23.0f, 5.0f, 0.0f}; // Posición en la escena
const float ESCALA_OROKINTOWER = 1.5f;                // Escala de la torre

// Configuración del Warframe
const glm::vec3 POS_WARFRAME = {5.0f, 7.0f, 0.0f}; // Posición en la escena
const float ESCALA_WARFRAME = 4.0f;             // Escala de la torre
} // namespace

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    edificio1_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio1.gltf");
    edificio2_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Kerwan_Edificio2.gltf");

    // Carga del Railjack animado
    railjack_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Railjack/RailjackSeparadoVibraAnimado.gltf");
    // Carga de la torre Orokin
    orokintower_ = std::make_unique<ModeloGltf>(c.recursos /"Originales/OrokinTower/TorreOrokin1.gltf");
    // Carga de Warframe
    warframe_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Warframe/Warframe.gltf");

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
    yEdificio1_ = apoyo(POS_EDIFICIO1, MEDIO_EDIFICIO1 * ESCALA_EDIFICIOS);
    yEdificio2_ = apoyo(POS_EDIFICIO2, MEDIO_EDIFICIO2 * ESCALA_EDIFICIOS);
}

void SectorRatchet::actualizar(float dt) {
    // Si tu clase ModeloGltf soporta actualización de animaciones/esqueleto,
    // puedes actualizar el Railjack aquí:
    // if (railjack_) {
    //     railjack_->actualizar(dt);
    // }
}

void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);

    if (edificio1_) {
        auto e1 = glm::translate(m, {POS_EDIFICIO1.x, yEdificio1_, POS_EDIFICIO1.z});
        edificio1_->dibujar(c.renderizador, glm::scale(e1, glm::vec3(ESCALA_EDIFICIOS)));
    }
    if (edificio2_) {
        auto e2 = glm::translate(m, {POS_EDIFICIO2.x, yEdificio2_, POS_EDIFICIO2.z});
        edificio2_->dibujar(c.renderizador, glm::scale(e2, glm::vec3(ESCALA_EDIFICIOS)));
    }

    // Dibujar el Railjack
    if (railjack_) {
        auto rj = glm::translate(m, POS_RAILJACK);
        railjack_->dibujar(c.renderizador, glm::scale(rj, glm::vec3(ESCALA_RAILJACK)));
    }

    // Dibujar la torre Orokin
    if (orokintower_) {
        auto ot = glm::translate(m, POS_OROKINTOWER);
        orokintower_->dibujar(c.renderizador, glm::scale(ot, glm::vec3(ESCALA_OROKINTOWER)));
    }
    // Dibujar el Warframe
    if (warframe_) {
        auto wf = glm::translate(m, POS_WARFRAME);
        warframe_->dibujar(c.renderizador, glm::scale(wf, glm::vec3(ESCALA_WARFRAME)));
    }
}

void SectorRatchet::liberar() noexcept {
    std::vector<Objeto>().swap(objetos_);
    edificio1_.reset();
    edificio2_.reset();
    railjack_.reset();
    orokintower_.reset();
    warframe_.reset();
}

bool SectorRatchet::obstaculo(glm::vec3 p) const {
    auto dentro = [&](glm::vec3 centro, glm::vec2 medio) {
        return std::abs(p.x - centro.x) < medio.x + .5f && std::abs(p.z - centro.z) < medio.y + .5f;
    };
    return dentro(POS_EDIFICIO1, MEDIO_EDIFICIO1 * ESCALA_EDIFICIOS) ||
           dentro(POS_EDIFICIO2, MEDIO_EDIFICIO2 * ESCALA_EDIFICIOS);
}