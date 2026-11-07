#include "Mundo/GestorEscenarios.h"
#include <iostream>
#include <stdexcept>
GestorEscenarios::~GestorEscenarios() {
    liberar();
}
void GestorEscenarios::registrar(std::unique_ptr<Escenario> e) {
    if (!e)
        throw std::invalid_argument("Escenario nulo");
    const auto id = e->id();
    if (regiones_.count(id))
        throw std::logic_error("Region duplicada");
    regiones_.emplace(id, std::move(e));
}
bool GestorEscenarios::activar(Region r, ContextoEscena &c) {
    auto it = regiones_.find(r);
    if (it == regiones_.end())
        throw std::out_of_range("Region no registrada");
    auto *destino = it->second.get();
    if (destino == actual_)
        return false;
    try {
        if(!cargadas_.count(r)){destino->cargar(c);cargadas_.insert(r);}
    } catch (...) {
        destino->liberar();
        throw;
    }
    // Conservamos recursos y estado de las regiones visitadas hasta el cierre.
    actual_ = destino;
    std::cout << "REGION " << actual_->nombre() << " preparada; una region activa, estado conservado\n";
    return true;
}
Escenario &GestorEscenarios::actual() const {
    if (!actual_)
        throw std::logic_error("No hay region activa");
    return *actual_;
}
void GestorEscenarios::liberar() noexcept {
    for (auto &par : regiones_)
        par.second->liberar();
    cargadas_.clear();
    actual_ = nullptr;
}
