#pragma once
#include <GLFW/glfw3.h>
#include <string>
// RAII: el contexto se destruye DESPUES de los recursos GPU por el orden de miembros de Aplicacion.
class Ventana {
    GLFWwindow *ventana_ = nullptr;

  public:
    Ventana();
    ~Ventana();
    Ventana(const Ventana &) = delete;
    Ventana &operator=(const Ventana &) = delete;
    GLFWwindow *nativa() const noexcept {
        return ventana_;
    }
    bool abierta() const;
    void cerrar();
    void eventos();
    void presentar();
    void dimensiones(int &ancho, int &alto) const;
    void titulo(const std::string &texto);
};
