#include "Mundo/MundoEspacial.h"
#include <glm/gtc/matrix_transform.hpp>
MundoEspacial::MundoEspacial() {
    transformaciones_[0] = glm::mat4(1);
    transformaciones_[1] = glm::translate(glm::mat4(1), glm::vec3(1600, 0, 0));
    transformaciones_[2] = glm::translate(glm::mat4(1), glm::vec3(800, 350, 0));
}
const glm::mat4 &MundoEspacial::transformacion(Region r) const {
    return transformaciones_.at(static_cast<size_t>(r));
}
glm::vec3 MundoEspacial::aGlobal(Region r, glm::vec3 p) const {
    return glm::vec3(transformacion(r) * glm::vec4(p, 1));
}
glm::vec3 MundoEspacial::aLocal(Region r, glm::vec3 p) const {
    return glm::vec3(glm::inverse(transformacion(r)) * glm::vec4(p, 1));
}
