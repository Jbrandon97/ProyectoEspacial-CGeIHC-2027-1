#pragma once
#include "Graficos/Malla.h"
#include "Graficos/Shader.h"
#include <memory>

class Renderizador;

class GalaxiaPlaneta2 {
    std::unique_ptr<Malla> nube_;
    std::unique_ptr<Shader> shader_;
    size_t puntos_ = 0;
    int pulsaciones_ = 0;
    double primeraPulsacion_ = 0;
    bool activa_ = false;
    float camara_ = 0, aparicion_ = 0, giro_ = 0;

  public:
    static constexpr glm::vec3 Ojo{105, 104, 165};
    static constexpr glm::vec3 Encuadre{-34, 75, -68};
    void cargar(const std::filesystem::path &recursos);
    void liberar() noexcept;
    bool pulsarG(double ahora);
    void interrumpir() { pulsaciones_ = 0; }
    void cerrar();
    void reiniciar();
    void actualizar(float dt);
    void dibujar(Renderizador &r, const glm::mat4 &mundo, float noche) const;
    bool activa() const { return activa_; }
    bool enCurso() const { return activa_ || camara_ > 0 || aparicion_ > 0; }
    float mezclaCamara() const;
    float presencia() const;
    size_t cantidadPuntos() const { return puntos_; }
};
