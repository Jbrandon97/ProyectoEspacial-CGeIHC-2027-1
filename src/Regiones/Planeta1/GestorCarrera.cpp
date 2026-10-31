#include "GestorCarrera.h"
#include "CircuitoCarreras.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
GestorCarrera::GestorCarrera() {
    nave_.colocar(0);
}
void GestorCarrera::reiniciar() {
    reglas_.reiniciar();
    nave_.colocar(0);
    for (auto &r : rivales_)
        r.reiniciar();
    cuenta_ = 3;
    reloj_ = castigo_ = fuera_ = acumulador_ = 0;
    resultado_ = 0;
    fase_ = FaseCarrera::CuentaAtras;
}
void GestorCarrera::controlar(const OrdenActividad &o) {
    orden_ = o;
    cerca_ = glm::length(o.avatar - (nave_.posicion() + CircuitoCarreras::origen())) < 7;
    if (o.interactuar) {
        if (!ocupado() && cerca_)
            reiniciar();
        else if (ocupado() && std::abs(nave_.velocidad()) < 2.5f) {
            fase_ = FaseCarrera::Disponible;
            nave_.detener();
        }
    }
    if (ocupado() && o.reiniciar)
        reiniciar();
    if (fase_ == FaseCarrera::Corriendo && o.recuperar)
        recuperar();
}
void GestorCarrera::recuperar() {
    if (castigo_ > 0)
        return;
    nave_.colocar(reglas_.puntoSeguro());
    reglas_.recuperar();
    castigo_ = 2;
    fuera_ = 0;
}
void GestorCarrera::actualizar(float dt) {
    acumulador_ += std::clamp(dt, 0.0f, .1f);
    while (acumulador_ >= 1.0f / 120) {
        paso(1.0f / 120);
        acumulador_ -= 1.0f / 120;
    }
}
void GestorCarrera::paso(float dt) {
    if (fase_ == FaseCarrera::CuentaAtras) {
        cuenta_ -= dt;
        if (cuenta_ <= 0)
            fase_ = FaseCarrera::Corriendo;
        return;
    }
    if (fase_ != FaseCarrera::Corriendo)
        return;
    reloj_ += dt;
    for (auto &r : rivales_)
        r.actualizar(dt);
    if (castigo_ > 0) {
        castigo_ = std::max(0.0f, castigo_ - dt);
        return;
    }
    nave_.actualizar(dt, orden_);
    auto p = CircuitoCarreras::proyectar(nave_.posicion());
    reglas_.actualizar(p.s, p.distancia <= CircuitoCarreras::Ancho / 2,
                       std::abs(nave_.velocidad()) * dt + 0.4f);
    fuera_ = p.distancia > CircuitoCarreras::Ancho / 2 + 2 ? fuera_ + dt : 0;
    if (fuera_ > 1.5f)
        recuperar();
    if (reglas_.terminada()) {
        resultado_ = posicion();
        fase_ = FaseCarrera::Terminada;
        nave_.detener();
    }
}
int GestorCarrera::posicion() const {
    if (fase_ == FaseCarrera::Terminada)
        return resultado_;
    int p = 1;
    for (auto &r : rivales_)
        if (r.avance() >= 3 * CircuitoCarreras::Longitud - .001f ||
            r.avance() > reglas_.progreso() + .1f)
            ++p;
    return p;
}
EstadoActividad GestorCarrera::estado() const {
    EstadoActividad e;
    e.ocupado = ocupado();
    e.objetivo = nave_.posicion() + CircuitoCarreras::origen();
    e.direccion = nave_.direccion();
    if (!ocupado()) {
        e.mensajes = {"PLANETA 1 / HALO + TITANFALL",
                      "WASD CAMINAR | SHIFT CORRER | MOUSE MIRAR | TAB LIBERAR",
                      cerca_ ? "PRESIONA X PARA ABORDAR LA NAVE"
                             : "ACERCATE A LA BANSHEE JUNTO AL ARCO DE META"};
        return e;
    }
    std::ostringstream s;
    s << std::fixed << std::setprecision(1) << "TIEMPO " << tiempo() << " S";
    e.mensajes = {"VUELTA " + std::to_string(std::min(3, reglas_.vueltas() + 1)) + "/3", s.str()};
    if (fase_ == FaseCarrera::CuentaAtras)
        e.mensajes.push_back("SALIDA EN " + std::to_string(std::max(1, int(std::ceil(cuenta_)))));
    else if (fase_ == FaseCarrera::Terminada)
        e.mensajes.push_back("CARRERA TERMINADA");
    else if (reloj_ < 1.5f)
        e.mensajes.push_back("SALIDA");
    return e;
}
