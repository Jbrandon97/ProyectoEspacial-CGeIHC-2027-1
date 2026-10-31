#include "Graficos/Shader.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>
namespace {
// Compila una etapa y elimina el objeto si el driver la rechaza.
GLuint compilar(GLenum tipo, const std::filesystem::path &ruta) {
    std::ifstream f(ruta);
    if (!f)
        throw std::runtime_error("No se encuentra shader: " + ruta.string());
    std::stringstream s;
    s << f.rdbuf();
    const auto texto = s.str();
    const char *p = texto.c_str();
    GLuint id = glCreateShader(tipo);
    glShaderSource(id, 1, &p, nullptr);
    glCompileShader(id);
    GLint ok = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetShaderInfoLog(id, 4096, nullptr, log);
        glDeleteShader(id);
        throw std::runtime_error(ruta.string() + ": " + log);
    }
    return id;
}
} // namespace
Shader::Shader(const std::filesystem::path &v, const std::filesystem::path &f) {
    GLuint a = compilar(GL_VERTEX_SHADER, v), b = 0;
    try {
        b = compilar(GL_FRAGMENT_SHADER, f);
        programa_ = glCreateProgram();
        glAttachShader(programa_, a);
        glAttachShader(programa_, b);
        glLinkProgram(programa_);
        GLint ok = 0;
        glGetProgramiv(programa_, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[4096];
            glGetProgramInfoLog(programa_, 4096, nullptr, log);
            throw std::runtime_error(log);
        }
    } catch (...) {
        glDeleteShader(a);
        if (b)
            glDeleteShader(b);
        if (programa_)
            glDeleteProgram(programa_);
        throw;
    }
    glDeleteShader(a);
    glDeleteShader(b);
}
Shader::~Shader() {
    if (programa_)
        glDeleteProgram(programa_);
}
void Shader::usar() const {
    glUseProgram(programa_);
}
void Shader::matriz(const char *n, const glm::mat4 &v) const {
    glUniformMatrix4fv(glGetUniformLocation(programa_, n), 1, GL_FALSE, glm::value_ptr(v));
}
void Shader::vector(const char *n, const glm::vec3 &v) const {
    glUniform3fv(glGetUniformLocation(programa_, n), 1, glm::value_ptr(v));
}

void Shader::entero(const char *n, int v) const {
    glUniform1i(glGetUniformLocation(programa_, n), v);
}
