#include "Nucleo/Entradas.h"
void Entradas::actualizar(GLFWwindow *v) {
    antes_ = ahora_;
    for (int k = GLFW_KEY_SPACE; k <= GLFW_KEY_LAST; ++k)
        ahora_[k] = glfwGetKey(v, k) == GLFW_PRESS;
    if (!preparado_) {
        glfwSetInputMode(v, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(v, &x_, &y_);
        preparado_ = true;
    }
    if (pulsada(GLFW_KEY_TAB)) {
        capturado_ = !capturado_;
        glfwSetInputMode(v, GLFW_CURSOR, capturado_ ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwGetCursorPos(v, &x_, &y_);
    }
    double x, y;
    glfwGetCursorPos(v, &x, &y);
    raton_ = capturado_ && glfwGetWindowAttrib(v, GLFW_FOCUSED)
                 ? glm::vec2(float(x - x_), float(y - y_))
                 : glm::vec2(0);
    x_ = x;
    y_ = y;
    // Limitar un salto al recuperar foco, sin multiplicar desplazamiento de mouse por dt.
    raton_ = glm::clamp(raton_, glm::vec2(-100), glm::vec2(100));
}
bool Entradas::pulsada(int k) const {
    return ahora_.at(k) && !antes_.at(k);
}
glm::vec2 Entradas::movimiento() const {
    glm::vec2 d(float(ahora_[GLFW_KEY_D]) - float(ahora_[GLFW_KEY_A]),
                float(ahora_[GLFW_KEY_W]) - float(ahora_[GLFW_KEY_S]));
    return glm::length(d) > 0 ? glm::normalize(d) : d;
}
float Entradas::orbita() const {
    return float(ahora_[GLFW_KEY_RIGHT]) - float(ahora_[GLFW_KEY_LEFT]);
}
