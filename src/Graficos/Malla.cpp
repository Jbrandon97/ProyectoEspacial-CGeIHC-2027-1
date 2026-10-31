#include "Graficos/Malla.h"
#include <cstddef>
Malla::Malla() {
    std::vector<Vertice> datos;
    for (auto n : std::vector<glm::vec3>{
             {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}) {
        auto u = glm::abs(n.y) > .5f ? glm::vec3(1, 0, 0)
                                     : glm::normalize(glm::cross(glm::vec3(0, 1, 0), n));
        auto v = glm::cross(n, u);
        for (auto t : std::vector<glm::vec2>{{-1, -1}, {1, -1}, {1, 1}, {-1, -1}, {1, 1}, {-1, 1}})
            datos.push_back({.5f * (n + u * t.x + v * t.y), n, {1, 1, 1}});
    }
    actualizar(datos);
}
Malla::Malla(const std::vector<Vertice> &v) {
    actualizar(v);
}
Malla::~Malla() {
    glDeleteBuffers(1, &vbo_);
    glDeleteVertexArrays(1, &vao_);
}
void Malla::actualizar(const std::vector<Vertice> &v, bool dinamica) {
    if (!vao_) {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
    }
    bool mismo = cantidad_ == static_cast<GLsizei>(v.size());
    cantidad_ = static_cast<GLsizei>(v.size());
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    if (dinamica && mismo)
        glBufferSubData(GL_ARRAY_BUFFER, 0, v.size() * sizeof(Vertice), v.data());
    else
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertice), v.data(),
                     dinamica ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    const size_t offsets[] = {offsetof(Vertice, posicion), offsetof(Vertice, normal),
                              offsetof(Vertice, color)};
    for (GLuint i = 0; i < 3; ++i) {
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(Vertice),
                              reinterpret_cast<void *>(offsets[i]));
        glEnableVertexAttribArray(i);
    }
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertice),
                          reinterpret_cast<void *>(offsetof(Vertice, uv)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}
void Malla::dibujar() const {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, cantidad_);
}
