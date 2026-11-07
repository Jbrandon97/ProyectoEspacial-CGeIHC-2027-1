#pragma once
#include "Mundo/Escenario.h"
#include <map>
#include <memory>
#include <set>
// Un propietario por escenario. Las regiones visitadas conservan estado y recursos hasta cerrar.
class GestorEscenarios {
    std::map<Region, std::unique_ptr<Escenario>> regiones_;
    std::set<Region> cargadas_;
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
