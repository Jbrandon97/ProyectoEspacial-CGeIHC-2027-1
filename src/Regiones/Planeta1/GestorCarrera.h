#pragma once
#include "ControlCarrera.h"
#include "NaveCarreras.h"
#include "RivalCarrera.h"
#include <array>
enum class FaseCarrera { Disponible, CuentaAtras, Corriendo, Terminada };
// Coordinador local: abordaje, pasos fijos, penalizaciones, rivales y resultado.
class GestorCarrera {
    NaveCarreras nave_;
    std::array<RivalCarrera, 2> rivales_{{{37, -3.0f}, {38, 3.0f}}};
    ControlCarrera reglas_;
    FaseCarrera fase_ = FaseCarrera::Disponible;
    OrdenActividad orden_;
    float cuenta_ = 3, reloj_ = 0, castigo_ = 0, fuera_ = 0, acumulador_ = 0;
    int resultado_ = 0;
    bool cerca_ = false;
    void paso(float dt);
    void recuperar();

  public:
    GestorCarrera();
    void controlar(const OrdenActividad &orden);
    void actualizar(float dt);
    void reiniciar();
    bool ocupado() const {
        return fase_ != FaseCarrera::Disponible;
    }
    FaseCarrera fase() const {
        return fase_;
    }
    const NaveCarreras &nave() const {
        return nave_;
    }
    const std::array<RivalCarrera, 2> &rivales() const {
        return rivales_;
    }
    const ControlCarrera &reglas() const {
        return reglas_;
    }
    float tiempo() const {
        return reloj_;
    }
    float recuperacionRestante() const {
        return castigo_;
    }
    int posicion() const;
    EstadoActividad estado() const;
};
