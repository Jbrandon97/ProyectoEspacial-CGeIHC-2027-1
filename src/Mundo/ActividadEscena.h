#pragma once
#include "Mundo/Tipos.h"
#include <string>
#include <vector>
// Datos de entrada independientes de GLFW. Las pruebas y el teclado usan el mismo contrato.
struct OrdenActividad {
    float acelerador = 0, freno = 0, giro = 0;
    bool interactuar = false, reiniciar = false, recuperar = false;
    glm::vec3 avatar{0};
    bool titan = false, hunter = false, saludo = false, reflector = false, inspeccion = false;
    bool puerta = false, lanzar = false, reiniciarCohete = false, reaccion = false;
};
struct EstadoActividad {
    bool ocupado = false;
    glm::vec3 objetivo{0}, direccion{0, 0, -1};
    std::vector<std::string> mensajes;
};
