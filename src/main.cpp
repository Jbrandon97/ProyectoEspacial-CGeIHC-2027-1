#include "Nucleo/Aplicacion.h"
// Punto de entrada: toda la inicializacion, ejecucion y gestion de errores vive en Aplicacion.
int main(int argc, char **argv) {
    return Aplicacion::ejecutar(argc, argv);
}
