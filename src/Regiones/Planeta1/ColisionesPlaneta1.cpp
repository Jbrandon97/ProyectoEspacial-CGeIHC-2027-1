#include "ColisionesPlaneta1.h"
#include "TerrenoPlaneta1.h"
#include <algorithm>
#include <cmath>
namespace {
// Punto mas cercano por regiones de Voronoi del triangulo; no depende de su orientacion.
glm::vec3 cercano(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    auto ab = b - a, ac = c - a, ap = p - a;
    float d1 = glm::dot(ab, ap), d2 = glm::dot(ac, ap);
    if (d1 <= 0 && d2 <= 0)
        return a;
    auto bp = p - b;
    float d3 = glm::dot(ab, bp), d4 = glm::dot(ac, bp);
    if (d3 >= 0 && d4 <= d3)
        return b;
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0 && d1 >= 0 && d3 <= 0)
        return a + ab * (d1 / (d1 - d3));
    auto cp = p - c;
    float d5 = glm::dot(ab, cp), d6 = glm::dot(ac, cp);
    if (d6 >= 0 && d5 <= d6)
        return c;
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0 && d2 >= 0 && d6 <= 0)
        return a + ac * (d2 / (d2 - d6));
    float va = d3 * d6 - d5 * d4;
    if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0)
        return b + (c - b) * ((d4 - d3) / ((d4 - d3) + (d5 - d6)));
    float inv = 1 / (va + vb + vc);
    return a + ab * (vb * inv) + ac * (vc * inv);
}
int indice(float x) {
    return int(std::floor(x / 8));
}
} // namespace
const std::vector<size_t> &ColisionesPlaneta1::celda(float x, float z) const {
    static const std::vector<size_t> vacia;
    auto it = celdas_.find({indice(x), indice(z)});
    return it == celdas_.end() ? vacia : it->second;
}
void ColisionesPlaneta1::limpiar() {
    triangulos_.clear();
    celdas_.clear();
}
void ColisionesPlaneta1::agregar(const std::vector<Vertice> &v, glm::vec3 d) {
    std::vector<glm::vec3> puntos;
    for (auto &a : v)
        puntos.push_back(a.posicion);
    agregar(puntos, d);
}
void ColisionesPlaneta1::agregar(const std::vector<glm::vec3> &v, glm::vec3 d) {
    for (size_t i = 0; i + 2 < v.size(); i += 3) {
        auto a = v[i] + d, b = v[i + 1] + d, c = v[i + 2] + d;
        auto cruz = glm::cross(b - a, c - a);
        if (glm::length(cruz) < .00001f)
            continue;
        size_t id = triangulos_.size();
        triangulos_.push_back({a, b, c, glm::normalize(cruz)});
        auto lo = glm::min(a, glm::min(b, c)) - glm::vec3(.7f),
             hi = glm::max(a, glm::max(b, c)) + glm::vec3(.7f);
        for (int x = indice(lo.x); x <= indice(hi.x); ++x)
            for (int z = indice(lo.z); z <= indice(hi.z); ++z)
                celdas_[{x, z}].push_back(id);
    }
}
float ColisionesPlaneta1::apoyo(float x, float z, float base) const {
    float h = base;
    for (auto id : celda(x, z)) {
        auto &t = triangulos_[id];
        if (t.n.y < .64f)
            continue;
        glm::vec2 a(t.a.x, t.a.z), b(t.b.x, t.b.z), c(t.c.x, t.c.z), q(x, z);
        auto u = b - a, v = c - a, w = q - a;
        float den = u.x * v.y - u.y * v.x;
        if (std::abs(den) < .000001f)
            continue;
        float beta = (w.x * v.y - w.y * v.x) / den, gamma = (u.x * w.y - u.y * w.x) / den;
        if (beta < -.0001f || gamma < -.0001f || beta + gamma > 1.0001f)
            continue;
        float y = t.a.y + beta * (t.b.y - t.a.y) + gamma * (t.c.y - t.a.y);
        // Solo niveles bajos transitables: excluye techos y estructuras altas.
        if (y <= base + 4.0f)
            h = std::max(h, y);
    }
    return h;
}
bool ColisionesPlaneta1::obstaculo(glm::vec3 pies) const {
    for (auto id : celda(pies.x, pies.z)) {
        auto &t = triangulos_[id];
        if (t.n.y >= .64f || std::max(t.a.y, std::max(t.b.y, t.c.y)) <= pies.y + .25f)
            continue;
        for (float altura : {.65f, 1.0f, 1.35f}) {
            auto centro = pies + glm::vec3(0, altura, 0);
            auto delta = centro - cercano(centro, t.a, t.b, t.c);
            if (glm::dot(delta, delta) < .64f * .64f)
                return true;
        }
    }
    return false;
}
glm::vec3 ColisionesPlaneta1::mover(glm::vec3 previo, glm::vec3 destino,
                                    const std::function<float(float, float)> &altura) const {
    auto delta = destino - previo;
    delta.y = 0;
    int pasos = std::max(1, int(std::ceil(glm::length(delta) / .18f)));
    auto paso = delta / float(pasos);
    auto actual = previo;
    auto valido = [&](glm::vec3 desde, glm::vec3 p) {
        auto permitido = TerrenoPlaneta1::permitir(desde, p);
        if (glm::length(glm::vec2(permitido.x - p.x, permitido.z - p.z)) > .001f)
            return false;
        float suelo = altura(p.x, p.z), anterior = altura(desde.x, desde.z);
        // Subpasos evitan atravesar paredes. Descenso de un bordillo de pista permitido, sin salto.
        if (suelo - anterior > .65f || anterior - suelo > 1.15f)
            return false;
        return !obstaculo({p.x, suelo + .015f, p.z});
    };
    for (int i = 0; i < pasos; ++i) {
        auto q = actual + paso;
        if (valido(actual, q))
            actual = q;
        else {
            auto x = actual + glm::vec3(paso.x, 0, 0), z = actual + glm::vec3(0, 0, paso.z);
            bool vx = valido(actual, x), vz = valido(actual, z);
            if (vx && (!vz || std::abs(paso.x) >= std::abs(paso.z)))
                actual = x;
            else if (vz)
                actual = z;
        }
    }
    return actual;
}
