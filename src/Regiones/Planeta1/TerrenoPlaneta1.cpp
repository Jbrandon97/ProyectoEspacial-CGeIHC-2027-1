#include "TerrenoPlaneta1.h"
#include "CircuitoCarreras.h"
#include "Distribucion.h"
#include "Graficos/Geometria.h"
#include <algorithm>
#include <cmath>
namespace {
float suave(float a, float b, float x) {
    float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3 - 2 * t);
}
const glm::vec3 lagos[] = {{-125, 0, 40}, {-195, 0, -40}};
const float radios[] = {29, 17};
float radio(int i, float ang) {
    return radios[i] * (1 + .085f * std::sin(3 * ang + .7f * i) + .06f * std::cos(5 * ang));
}
float distanciaOrilla(int i, float x, float z) {
    float dx = x - lagos[i].x, dz = z - lagos[i].z;
    return std::sqrt(dx * dx + dz * dz) - radio(i, std::atan2(dz, dx));
}
} // namespace
bool TerrenoPlaneta1::reservaPista(float x, float z, float margen) {
    auto p = glm::vec3(x, 0, z) - CircuitoCarreras::origen();
    return std::abs(p.x) < 96 + margen && std::abs(p.z) < 65 + margen;
}
bool TerrenoPlaneta1::lago(float x, float z, float margen) {
    for (int i = 0; i < 2; ++i)
        if (distanciaOrilla(i, x, z) < margen)
            return true;
    return false;
}
float TerrenoPlaneta1::altura(float x, float z) {
    float h =
        4 + 2.5f * std::sin(x * .023f) * std::cos(z * .027f) + 1.5f * std::sin((x + z) * .047f);
    // Cordillera continua con pie y cresta irregulares: oculta el borde exterior a 300 m.
    float borde = std::max(std::abs(x), std::abs(z)) + 7 * std::sin(x * .035f + z * .025f);
    float cresta = 66 + 18 * std::sin(x * .043f + z * .019f) + 12 * std::cos(z * .071f - x * .028f);
    h += suave(235, 292, borde) * cresta;
    auto q = glm::vec3(x, 0, z) - CircuitoCarreras::origen();
    h = glm::mix(2.0f, h, suave(0, 15, std::max(std::abs(q.x) - 96, std::abs(q.z) - 65)));
    for (auto &e : Distribucion::edificios) {
        auto d = glm::abs(glm::vec2(x, z) - e.centro) - e.semiejes;
        h = glm::mix(2.0f, h, suave(3, 13, std::max(d.x, d.y)));
    }
    auto reserva = glm::abs(glm::vec2(x - 145, z - 25)) - glm::vec2(22, 17);
    h = glm::mix(2.0f, h, suave(2, 10, std::max(reserva.x, reserva.y)));
    h = glm::mix(2.0f, h, suave(5, 13, Distribucion::camino(x, z)));
    for (int i = 0; i < 2; ++i) {
        float d = distanciaOrilla(i, x, z);
        h = glm::mix(-1.8f, h, suave(-4, 10, d));
    }
    return h;
}
glm::vec3 TerrenoPlaneta1::normal(float x, float z) {
    float d = .3f;
    return glm::normalize(
        glm::vec3(altura(x - d, z) - altura(x + d, z), 2 * d, altura(x, z - d) - altura(x, z + d)));
}
glm::vec3 TerrenoPlaneta1::permitir(glm::vec3 previo, glm::vec3 p) {
    p.x = std::clamp(p.x, -296.0f, 296.0f);
    p.z = std::clamp(p.z, -296.0f, 296.0f);
    if (lago(p.x, p.z, 2) || normal(p.x, p.z).y < .64f)
        return previo;
    return p;
}
std::vector<Vertice> TerrenoPlaneta1::construir() {
    std::vector<Vertice> v;
    v.reserve(200 * 200 * 6);
    auto vert = [](float x, float z) {
        float h = altura(x, z);
        auto n = normal(x, z);
        return Vertice{{x, h, z}, n, glm::vec3(1), {(x + 300) / 600, (z + 300) / 600}};
    };
    // Misma cantidad de celdas que antes: 3 m por celda, 600 x 600, 80 000 triangulos.
    for (int i = 0; i < 200; ++i)
        for (int j = 0; j < 200; ++j) {
            float x = -300 + i * 3.0f, z = -300 + j * 3.0f;
            auto a = vert(x, z), b = vert(x, z + 3), c = vert(x + 3, z + 3), d = vert(x + 3, z);
            v.insert(v.end(), {a, b, c, a, c, d});
        }
    return v;
}
std::vector<Vertice> TerrenoPlaneta1::paisaje() {
    std::vector<Vertice> v;
    // Rocas ligeras en grupos junto a bosque, lago y transicion; sin edificios provisionales.
    const glm::vec2 rocas[] = {{-171, 75},  {-153, 78},  {-104, 77},  {-88, 4},    {-156, 1},
                               {-165, -4},  {-218, -18}, {-218, -61}, {-80, -115}, {-30, -85},
                               {-185, 115}, {-125, 126}, {-87, 105},  {-20, 80},   {5, -50},
                               {120, -175}, {201, -145}, {235, 10}};
    // Plataforma auxiliar reservada para transporte entre planetas, sin nave ni interaccion.
    Geometria::caja(v, {145, 2.04f, 25}, {40, .08f, 30}, {.39f, .41f, .4f});
    for (float dx : {-18.0f, 18.0f})
        for (float dz : {-13.0f, 13.0f})
            Geometria::caja(v, {145 + dx, 2.1f, 25 + dz}, {3, .03f, .4f}, {.9f, .68f, .23f});
    int k = 0;
    for (auto q : rocas) {
        Geometria::elipsoide(v, {q.x, altura(q.x, q.y) + .5f, q.y},
                             {2 + float(k++ % 3), 1.8f, 2.6f}, {.48f, .5f, .46f}, 8, 5);
    }
    return v;
}
std::vector<Vertice> TerrenoPlaneta1::agua() {
    std::vector<Vertice> v;
    for (int i = 0; i < 2; ++i) {
        auto c = lagos[i];
        c.y = .3f;
        for (int j = 0; j < 80; ++j) {
            float a = 6.2831853f * j / 80, b = 6.2831853f * (j + 1) / 80;
            Geometria::triangulo(
                v, c, c + glm::vec3(std::cos(b) * radio(i, b), 0, std::sin(b) * radio(i, b)),
                c + glm::vec3(std::cos(a) * radio(i, a), 0, std::sin(a) * radio(i, a)), {1, 1, 1});
        }
    }
    for (auto &vert : v)
        vert.uv = {vert.posicion.x / 9, vert.posicion.z / 9};
    return v;
}
