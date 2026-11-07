#pragma once
#include "Mundo/Tipos.h"
#include <functional>
// Gestor conservado: exploracion orbital y seguimiento suave del vehiculo, mas modos futuros.
class GestorCamaras {
    ModoCamara modo_ = ModoCamara::Exploracion;
    float angulo_ = 0, inclinacion_ = .28f;
    glm::vec3 ojo_{0}, objetivo_{0};
    bool preparada_ = false, cabina_ = false;

  public:
    void alternarCabina() { cabina_ = !cabina_; preparada_ = false; }
    bool cabina() const { return cabina_; }
    static bool compatible(ModoCamara modo, Region region);
    bool solicitar(ModoCamara modo, Region region);
    void alCambiarRegion(Region region);
    void orbitar(float delta) {
        angulo_ += delta;
    }
    void mirar(glm::vec2 desplazamiento);
    void encuadrarEvento(glm::vec3 centro) {
        objetivo_ = centro;
        ojo_ = centro + glm::vec3(60, 22, 75);
        preparada_ = true;
    }
    glm::vec2 movimientoRelativo(glm::vec2 entrada) const;
    // Altura recibe coordenadas globales; resuelve colision del brazo de camara con terreno.
    void seguir(glm::vec3 objetivo, glm::vec3 direccion, float dt,
                const std::function<float(float, float)> &altura);
    void seguirEspacio(glm::vec3 nave, glm::vec3 frente, glm::vec3 centro, float dt);
    glm::mat4 vista(glm::vec3 objetivoGlobal) const;
    glm::mat4 proyeccion(float aspecto) const;
    ModoCamara modo() const noexcept {
        return modo_;
    }
    const char *nombre() const;
    glm::vec3 ojo() const {
        return ojo_;
    }
};
