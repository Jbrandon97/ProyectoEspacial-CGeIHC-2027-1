#include "Espacio.h"
#include "Graficos/Renderizador.h"
void Espacio::cargar(ContextoEscena &) {
    liberar();
    objetos_ = {{{0, 4, 0}, {18, 6, 7}, {.6f, .7f, .82f}},
                {{-18, 4, 0}, {16, .5f, 12}, {.12f, .3f, .7f}},
                {{18, 4, 0}, {16, .5f, 12}, {.12f, .3f, .7f}},
                {{0, 8, 0}, {4, 8, 4}, {.8f, .85f, .9f}},
                {{-11, .3f, 9}, {5, .6f, 8}, {.2f, .75f, .95f}}};
    for (int i = 0; i < 18; ++i)
        objetos_.push_back(
            {{float((i * 31) % 100) - 50, float((i * 17) % 50) - 12, -45 - float(i % 3) * 5},
             {.35f, .35f, .35f},
             {.8f, .9f, 1}});
    for (int i = 0; i < 5; ++i)
        objetos_.push_back({{25 + float(i) * 5, -9 + float(i), -20 - float(i) * 4},
                            {2, 3, 2},
                            {.4f, .45f, .5f},
                            float(i)});
}
void Espacio::actualizar(float) {}
void Espacio::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    for (const auto &o : objetos_)
        c.renderizador.dibujar(o, m);
}
void Espacio::liberar() noexcept {
    std::vector<Objeto>().swap(objetos_);
}
Ambiente Espacio::ambiente() const noexcept {
    return {{.009f, .015f, .04f}, {.5f, 1, .2f}};
}
