#pragma once
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
// Programa GLSL propietario, no copiable. Informa errores con el archivo y log del driver.
class Shader {
    GLuint programa_ = 0;
    mutable std::unordered_map<std::string, GLint> uniformes_;
    GLint uniforme(const char *nombre) const;

  public:
    Shader(const std::filesystem::path &vertice, const std::filesystem::path &fragmento);
    ~Shader();
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    void usar() const;
    void entero(const char *, int) const;
    void matriz(const char *nombre, const glm::mat4 &valor) const;
    void vector(const char *nombre, const glm::vec3 &valor) const;
};
