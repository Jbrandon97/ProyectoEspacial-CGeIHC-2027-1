#pragma once
#include "Mundo/ActividadEscena.h"
#include "Mundo/ContextoEscena.h"
#include "Mundo/Tipos.h"
// Contrato regional. Todas las posiciones devueltas son LOCALES; Y arriba y suelo XZ.
// cargar puede lanzar: liberar debe ser idempotente, noexcept y limpiar incluso una carga parcial.
// No conservar el ContextoEscena temporal, crear ventanas ni consultar GLFW desde las regiones.
class Escenario {
  public:
    // Extensiones opcionales con valores por defecto: Planeta2 y Espacio no requieren cambios.
    virtual void controlar(const OrdenActividad &) {}
    virtual EstadoActividad actividad() const {
        return {};
    }
    virtual glm::vec3 ajustarMovimiento(glm::vec3, glm::vec3 propuesta) const {
        propuesta.x = glm::clamp(propuesta.x, -43.0f, 43.0f);
        propuesta.z = glm::clamp(propuesta.z, -43.0f, 43.0f);
        return propuesta;
    }
    virtual ~Escenario() = default;
    virtual Region id() const noexcept = 0;
    virtual const char *nombre() const noexcept = 0;
    virtual void cargar(ContextoEscena &servicios) = 0;
    virtual void actualizar(float deltaTime) = 0; // Segundos, limitado por Aplicacion.
    virtual void dibujar(ContextoEscena &servicios, const glm::mat4 &alMundo) const = 0;
    virtual void liberar() noexcept = 0;
    virtual glm::vec3 aparicion() const noexcept = 0;
    virtual float altura(float x, float z) const noexcept = 0; // Superficie caminable local.
    virtual Ambiente ambiente() const noexcept = 0;
};
