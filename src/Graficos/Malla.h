#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
// Formato compartido con UV opcionales; los vertices anteriores conservan sus valores por defecto.
struct Vertice {
    glm::vec3 posicion, normal, color{1};
    glm::vec2 uv{0};
};
// Malla GPU RAII no copiable. El constructor vacio mantiene el cubo de Etapa 0.
class Malla {
    GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
    bool indexada_ = false;
    GLsizei cantidad_ = 0;

  public:
    Malla();
    explicit Malla(const std::vector<Vertice> &vertices);
    Malla(const std::vector<Vertice> &, const std::vector<unsigned> &);
    ~Malla();
    Malla(const Malla &) = delete;
    Malla &operator=(const Malla &) = delete;
    void actualizar(const std::vector<Vertice> &vertices, bool dinamica = false);
    void dibujar(GLenum modo = GL_TRIANGLES) const;
};
