#pragma once
#include "Mundo/Escenario.h"
#include <map>
#include <memory>
// Un propietario por escenario. Solo la region activa consume recursos regionales.
class GestorEscenarios {
    std::map<Region, std::unique_ptr<Escenario>> regiones_;
    Escenario *actual_ = nullptr;

  public:
    ~GestorEscenarios();
    void registrar(std::unique_ptr<Escenario> escenario);   // Rechaza duplicados.
    bool activar(Region region, ContextoEscena &servicios); // Carga antes de abandonar el anterior.
    Escenario &actual() const;
    size_t cantidad() const noexcept {
        return regiones_.size();
    }
    void liberar() noexcept;
};
