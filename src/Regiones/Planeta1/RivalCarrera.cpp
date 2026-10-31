#include "RivalCarrera.h"
#include "CircuitoCarreras.h"
#include "NaveCarreras.h"
#include <algorithm>
#include <cmath>
void RivalCarrera::actualizar(float dt) {
    if (avance_ >= 3 * CircuitoCarreras::Longitud) {
        velocidad_ = 0;
        return;
    }
    float meta = rapidez_;
    // Anticipar giro y pendiente en los siguientes 14 m; frenado gradual, no velocidad instantanea.
    for (float anticipacion : {0.0f, 7.0f, 14.0f}) {
        auto a = CircuitoCarreras::muestra(avance_ + anticipacion),
             b = CircuitoCarreras::muestra(avance_ + anticipacion + 3);
        glm::vec2 ta = glm::normalize(glm::vec2(a.tangente.x, a.tangente.z)),
                  tb = glm::normalize(glm::vec2(b.tangente.x, b.tangente.z));
        if (glm::length(ta - tb) > .07f)
            meta = std::min(meta, rapidez_ * .79f);
        if (std::abs(a.tangente.y) > .14f)
            meta = std::min(meta, rapidez_ * .88f);
    }
    velocidad_ += std::clamp(meta - velocidad_, -18.0f * dt, 12.0f * dt);
    avance_ = std::min(3 * CircuitoCarreras::Longitud, avance_ + velocidad_ * dt);
}
glm::vec3 RivalCarrera::posicion() const {
    return CircuitoCarreras::muestra(avance_, carril_).posicion;
}
glm::mat4 RivalCarrera::matriz() const {
    auto m = CircuitoCarreras::muestra(avance_, carril_);
    return NaveCarreras::orientar(m.posicion, m.tangente, m.normal);
}
