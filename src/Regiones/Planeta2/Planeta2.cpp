#include "Planeta2.h"
#include "Graficos/Renderizador.h"
#include <glm/gtc/matrix_transform.hpp>
void Planeta2::cargar(ContextoEscena &c) {
    liberar();
    objetos_.push_back({{0, -1, 0}, {90, 2, 90}, {.2f, .18f, .3f}});
    objetos_.push_back({{-11, .6f, 9}, {5, .6f, 8}, {.2f, .75f, .95f}});
    warframe_.cargar(c);
    ratchet_.cargar(c); // El gestor libera ambos si una carga falla.
}
void Planeta2::actualizar(float dt) {
    warframe_.actualizar(dt);
    ratchet_.actualizar(dt);
}
void Planeta2::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
    warframe_.dibujar(c, glm::translate(m, glm::vec3(-18, 0, 0)));
    ratchet_.dibujar(c, glm::translate(m, glm::vec3(18, 0, 0)));
}
void Planeta2::liberar() noexcept {
    warframe_.liberar();
    ratchet_.liberar();
    std::vector<Objeto>().swap(objetos_);
}
Ambiente Planeta2::ambiente() const noexcept {
    return {{.1f, .065f, .19f}, {-.5f, 1, .3f}};
}
