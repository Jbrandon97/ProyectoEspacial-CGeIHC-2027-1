#include "Planeta2.h"
#include "Graficos/Renderizador.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {
const glm::vec3 POS_WARFRAME = {-18, 0, 0};
const glm::vec3 POS_RATCHET = {18, 0, 0};
const float LIMITE = 230.0f;      // el terreno mide 600 (+-300); la cordillera empieza ~235
const float PENDIENTE_MAX = 1.2f; // ~50 grados: no deja subir laderas mas empinadas
} // namespace

void Planeta2::cargar(ContextoEscena &c) {
    liberar();
    terreno_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Terreno2.gltf");
    alturas_ = std::make_unique<AlturasTerreno>(c.recursos / "Modelos/Alturas_201x201.f32");
    warframe_.cargar(c);
    // el sector Ratchet pregunta la altura en sus coordenadas locales
    ratchet_.usarSuelo(
        [this](float x, float z) { return altura(x + POS_RATCHET.x, z + POS_RATCHET.z); });
    ratchet_.cargar(c); // El gestor libera ambos si una carga falla.
}
void Planeta2::actualizar(float dt) {
    warframe_.actualizar(dt);
    ratchet_.actualizar(dt);
}
void Planeta2::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    if (terreno_)
        terreno_->dibujar(c.renderizador, m);
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
    warframe_.dibujar(c, glm::translate(m, POS_WARFRAME));
    ratchet_.dibujar(c, glm::translate(m, POS_RATCHET));
}
void Planeta2::liberar() noexcept {
    warframe_.liberar();
    ratchet_.liberar();
    std::vector<Objeto>().swap(objetos_);
    terreno_.reset();
    alturas_.reset();
}
glm::vec3 Planeta2::ajustarMovimiento(glm::vec3 anterior, glm::vec3 p) const {
    p.x = glm::clamp(p.x, -LIMITE, LIMITE);
    p.z = glm::clamp(p.z, -LIMITE, LIMITE);
    // pendiente: solo bloquea al SUBIR una ladera muy inclinada (bajar si se puede)
    auto empinado = [this](glm::vec3 a, glm::vec3 b) {
        float dist = glm::length(glm::vec2(b.x - a.x, b.z - a.z));
        return dist > 1e-4f && (altura(b.x, b.z) - altura(a.x, a.z)) / dist > PENDIENTE_MAX;
    };
    auto choca = [&](glm::vec3 q) {
        return ratchet_.obstaculo(q - POS_RATCHET) || empinado(anterior, q);
    };
    if (!choca(p))
        return p;
    glm::vec3 soloX{p.x, p.y, anterior.z}, soloZ{anterior.x, p.y, p.z};
    if (!choca(soloX))
        return soloX; // se desliza
    if (!choca(soloZ))
        return soloZ;
    return anterior;
}
Ambiente Planeta2::ambiente() const noexcept {
    return {{.1f, .065f, .19f}, {-.5f, 1, .3f}};
}