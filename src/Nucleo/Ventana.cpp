#include "Nucleo/Ventana.h"
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>
Ventana::Ventana() {
    glfwSetErrorCallback(
        [](int c, const char *s) { std::cerr << "GLFW " << c << ": " << s << '\n'; });
    if (!glfwInit())
        throw std::runtime_error("No se pudo iniciar GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    ventana_ = glfwCreateWindow(1280, 800, "Proyecto Espacial - Etapa 0", nullptr, nullptr);
    if (!ventana_) {
        glfwTerminate();
        throw std::runtime_error("Se requiere OpenGL 3.3 Core");
    }
    glfwMakeContextCurrent(ventana_);
    // Conserva pulsaciones breves entre cuadros (X, R y cambios de region).
    glfwSetInputMode(ventana_, GLFW_STICKY_KEYS, GLFW_TRUE);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(ventana_);
        glfwTerminate();
        throw std::runtime_error("GLAD no pudo cargar OpenGL");
    }
    glfwSwapInterval(1);
    std::cout << "OpenGL " << glGetString(GL_VERSION) << " | GPU " << glGetString(GL_RENDERER)
              << '\n';
}
Ventana::~Ventana() {
    glfwDestroyWindow(ventana_);
    glfwTerminate();
}
bool Ventana::abierta() const {
    return !glfwWindowShouldClose(ventana_);
}
void Ventana::cerrar() {
    glfwSetWindowShouldClose(ventana_, GLFW_TRUE);
}
void Ventana::eventos() {
    glfwPollEvents();
}
void Ventana::presentar() {
    glfwSwapBuffers(ventana_);
}
void Ventana::dimensiones(int &a, int &b) const {
    glfwGetFramebufferSize(ventana_, &a, &b);
}
void Ventana::titulo(const std::string &t) {
    glfwSetWindowTitle(ventana_, t.c_str());
}
