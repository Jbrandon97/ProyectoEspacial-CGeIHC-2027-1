#pragma once
#include "Mundo/Tipos.h"
#include <array>
// Conserva la colocacion de TODAS las regiones aunque sus recursos no esten cargados.
class MundoEspacial {
    std::array<glm::mat4, 3> transformaciones_;

  public:
    MundoEspacial();
    const glm::mat4 &transformacion(Region region) const;
    glm::vec3 aGlobal(Region region, glm::vec3 local) const;
    glm::vec3 aLocal(Region region, glm::vec3 global) const;
};
