#include "ControlCarrera.h"
#include "CircuitoCarreras.h"
#include <algorithm>
#include <cmath>
void ControlCarrera::reiniciar() {
    puertas_ = 0;
    anterior_ = recorridoTramo_ = progreso_ = 0;
    anteriorValido_ = true;
}
void ControlCarrera::actualizar(float s, bool pista, float maximo) {
    const float L = CircuitoCarreras::Longitud, sector = L / 8;
    float delta = s - anterior_;
    if (delta > L / 2)
        delta -= L;
    if (delta < -L / 2)
        delta += L;
    // Un salto, marcha atras o salida no puede acreditar distancia. Debe recorrerse al menos
    // el 90% del tramo por asfalto antes de cruzar la puerta siguiente en sentido correcto.
    if (pista && anteriorValido_ && std::abs(delta) <= maximo) {
        recorridoTramo_ = std::max(0.0f, recorridoTramo_ + delta);
        float puerta = CircuitoCarreras::envolver((puertas_ + 1) * sector);
        float hasta = puerta - anterior_;
        if (hasta < 0)
            hasta += L;
        if (delta > 0 && hasta > 0 && hasta <= delta + .0001f && recorridoTramo_ >= sector * .90f &&
            !terminada()) {
            ++puertas_;
            recorridoTramo_ = std::max(0.0f, delta - hasta);
        }
        float dentro = CircuitoCarreras::envolver(s - puertas_ * sector);
        progreso_ = puertas_ * sector + std::min(dentro, sector * .99f);
    } else
        recorridoTramo_ = 0;
    anterior_ = s;
    anteriorValido_ = pista;
}
float ControlCarrera::puntoSeguro() const {
    return CircuitoCarreras::envolver(puertas_ * CircuitoCarreras::Longitud / 8);
}
void ControlCarrera::recuperar() {
    anterior_ = puntoSeguro();
    progreso_ = puertas_ * CircuitoCarreras::Longitud / 8;
    recorridoTramo_ = 0;
    anteriorValido_ = true;
}
