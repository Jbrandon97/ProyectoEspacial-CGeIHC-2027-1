#pragma once
#include <filesystem>
class Renderizador;
// Servicios prestados; la region no los posee ni puede destruirlos. Valido durante su ciclo de
// vida.
struct ContextoEscena {
    Renderizador &renderizador;
    const std::filesystem::path &recursos;
};
