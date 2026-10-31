#include "Graficos/Geometria.h"
#include <cmath>
namespace Geometria {
void triangulo(std::vector<Vertice> &m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color) {
    auto n = glm::cross(b - a, c - a);
    if (glm::length(n) < 1e-6f)
        return;
    n = glm::normalize(n);
    m.insert(m.end(), {{a, n, color}, {b, n, color}, {c, n, color}});
}
void caja(std::vector<Vertice> &m, glm::vec3 centro, glm::vec3 d, glm::vec3 color) {
    for (auto n : std::vector<glm::vec3>{
             {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}) {
        auto u = glm::abs(n.y) > .5f ? glm::vec3(1, 0, 0)
                                     : glm::normalize(glm::cross(glm::vec3(0, 1, 0), n));
        auto v = glm::cross(n, u);
        for (auto t : std::vector<glm::vec2>{{-1, -1}, {1, -1}, {1, 1}, {-1, -1}, {1, 1}, {-1, 1}})
            m.push_back({centro + .5f * d * (n + u * t.x + v * t.y), n, color});
    }
}
void cono(std::vector<Vertice> &m, glm::vec3 p, float r, float h, glm::vec3 color, int lados) {
    for (int i = 0; i < lados; ++i) {
        float a = 6.2831853f * i / lados, b = 6.2831853f * (i + 1) / lados;
        glm::vec3 x = p + glm::vec3(r * std::cos(a), 0, r * std::sin(a)),
                  z = p + glm::vec3(r * std::cos(b), 0, r * std::sin(b));
        triangulo(m, x, p + glm::vec3(0, h, 0), z, color);
        triangulo(m, p, x, z, color);
    }
}
void elipsoide(std::vector<Vertice> &m, glm::vec3 p, glm::vec3 r, glm::vec3 color, int lados,
               int anillos) {
    auto punto = [&](int i, int j) {
        float a = 6.2831853f * i / lados, b = 3.14159265f * j / anillos;
        return p + r * glm::vec3(std::sin(b) * std::cos(a), std::cos(b), std::sin(b) * std::sin(a));
    };
    for (int i = 0; i < lados; ++i)
        for (int j = 0; j < anillos; ++j) {
            auto a = punto(i, j), b = punto(i + 1, j), c = punto(i + 1, j + 1), d = punto(i, j + 1);
            triangulo(m, a, b, c, color);
            triangulo(m, a, c, d, color);
        }
}
} // namespace Geometria
