#include "SectorWarframe.h"
#include "Graficos/Renderizador.h"
void SectorWarframe::cargar(ContextoEscena &) {
    liberar();
    objetos_.push_back({{0, .1f, 0}, {30, .2f, 45}, {.65f, .35f, .85f}});
    for (int i = 0; i < 3; ++i)
        objetos_.push_back({{-9 + float(i) * 9, 3 + float(i), -8},
                            {4, 6 + float(i) * 2, 4},
                            {.65f, .35f, .85f},
                            .3f * float(i)});
}
void SectorWarframe::actualizar(float) {} // Sin animacion propia todavia.
void SectorWarframe::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
}
void SectorWarframe::liberar() noexcept {
    std::vector<Objeto>().swap(objetos_);
}
