#include "Mundo/RegistroEscenarios.h"
#include "Mundo/GestorEscenarios.h"
#include "Regiones/Espacio/Espacio.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Planeta2/Planeta2.h"
void registrarEscenarios(GestorEscenarios &g) {
    g.registrar(std::make_unique<Planeta1>());
    g.registrar(std::make_unique<Planeta2>());
    g.registrar(std::make_unique<Espacio>());
}
