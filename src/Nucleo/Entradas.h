#pragma once
#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
// Entrada por cuadro: pulsaciones, teclas sostenidas y desplazamiento relativo del mouse.
class Entradas {
    std::array<bool, GLFW_KEY_LAST + 1> antes_{}, ahora_{};
    double x_ = 0, y_ = 0;
    bool preparado_ = false, capturado_ = true;
    glm::vec2 raton_{0};

  public:
    void actualizar(GLFWwindow *ventana);
    bool pulsada(int tecla) const;
    bool mantenida(int tecla) const {
        return ahora_.at(tecla);
    }
    glm::vec2 movimiento() const;
    glm::vec2 raton() const {
        return raton_;
    }
    float orbita() const;
};
