#pragma once
// Checkpoints ordenados independientes del render y del control de la nave.
class ControlCarrera {
    int puertas_ = 0;
    float anterior_ = 0, recorridoTramo_ = 0, progreso_ = 0;
    bool anteriorValido_ = true;

  public:
    void reiniciar();
    void actualizar(float s, bool enPista, float desplazamientoMaximo);
    void recuperar();
    int vueltas() const {
        return puertas_ / 8;
    }
    int siguiente() const {
        return puertas_ % 8 + 1;
    }
    bool terminada() const {
        return puertas_ >= 24;
    }
    float progreso() const {
        return progreso_;
    }
    float puntoSeguro() const;
};
