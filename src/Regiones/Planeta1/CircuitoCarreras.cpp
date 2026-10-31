#include "CircuitoCarreras.h"
#include "Graficos/Geometria.h"
#include <algorithm>
#include <cmath>
#include <limits>
float CircuitoCarreras::envolverReferencia(float s) {
    s = std::fmod(s, LongitudReferencia);
    return s < 0 ? s + LongitudReferencia : s;
}
float CircuitoCarreras::rampa(float x, float z) {
    if (std::abs(x) >= 30 || std::abs(z) < 54.9f)
        return 0;
    float u = (x + 30) / 60;
    float seno = std::sin(Pi * u);
    return (z < 0 ? 6.0f : 4.0f) * seno * seno;
}
float CircuitoCarreras::pendiente(float x, float z) {
    if (std::abs(x) >= 30 || std::abs(z) < 54.9f)
        return 0;
    return (z < 0 ? 6.0f : 4.0f) * Pi / 60 * std::sin(2 * Pi * (x + 30) / 60);
}
MuestraCircuito CircuitoCarreras::muestraReferencia(float s, float lateral) {
    s = envolverReferencia(s);
    glm::vec3 p(0), t(0);
    const float arco = Pi * 12.5f;
    // Segmentos rectos y arcos exactos; tangentes coinciden en todas las uniones.
    if (s < 30) {
        p = {90, 0, -s};
        t = {0, 0, -1};
    } else if ((s -= 30) < arco) {
        float a = -s / 25;
        p = {65 + 25 * std::cos(a), 0, -30 + 25 * std::sin(a)};
        t = {std::sin(a), 0, -std::cos(a)};
    } else if ((s -= arco) < 130) {
        p = {65 - s, 0, -55};
        t = {-1, 0, 0};
    } else if ((s -= 130) < arco) {
        float a = -Pi / 2 - s / 25;
        p = {-65 + 25 * std::cos(a), 0, -30 + 25 * std::sin(a)};
        t = {std::sin(a), 0, -std::cos(a)};
    } else if ((s -= arco) < 60) {
        p = {-90, 0, -30 + s};
        t = {0, 0, 1};
    } else if ((s -= 60) < arco) {
        float a = -Pi - s / 25;
        p = {-65 + 25 * std::cos(a), 0, 30 + 25 * std::sin(a)};
        t = {std::sin(a), 0, -std::cos(a)};
    } else if ((s -= arco) < 130) {
        p = {-65 + s, 0, 55};
        t = {1, 0, 0};
    } else if ((s -= 130) < arco) {
        float a = Pi / 2 - s / 25;
        p = {65 + 25 * std::cos(a), 0, 30 + 25 * std::sin(a)};
        t = {std::sin(a), 0, -std::cos(a)};
    } else {
        s -= arco;
        p = {90, 0, 30 - s};
        t = {0, 0, -1};
    }
    float h = rampa(p.x, p.z), dy = pendiente(p.x, p.z) * t.x;
    auto derecha = glm::vec3(-t.z, 0, t.x);
    p += derecha * lateral;
    p.y = 3 + h;
    t = glm::normalize(glm::vec3(t.x, dy, t.z));
    return {p, t, glm::normalize(glm::cross(derecha, t))};
}
ProyeccionCircuito CircuitoCarreras::proyectarReferencia(glm::vec3 p) {
    // Proyeccion exacta contra los nueve tramos, no una busqueda por vertices de la malla.
    float mejor = std::numeric_limits<float>::max(), sMejor = 0;
    glm::vec2 q(p.x, p.z);
    auto probar = [&](float s) {
        auto m = muestraReferencia(s);
        float d = glm::length(q - glm::vec2(m.posicion.x, m.posicion.z));
        if (d < mejor) {
            mejor = d;
            sMejor = envolverReferencia(s);
        }
    };
    const float a = Pi * 12.5f;
    probar(std::clamp(-p.z, 0.0f, 30.0f));
    probar(30 + a + std::clamp(65 - p.x, 0.0f, 130.0f));
    probar(30 + 2 * a + 130 + std::clamp(p.z + 30, 0.0f, 60.0f));
    probar(90 + 3 * a + 130 + std::clamp(p.x + 65, 0.0f, 130.0f));
    probar(90 + 4 * a + 260 + std::clamp(30 - p.z, 0.0f, 30.0f));
    const glm::vec2 centros[] = {{65, -30}, {-65, -30}, {-65, 30}, {65, 30}};
    const float inicio[] = {0, -Pi / 2, -Pi, Pi / 2},
                base[] = {30, 30 + a + 130, 90 + 2 * a + 130, 90 + 3 * a + 260};
    for (int i = 0; i < 4; ++i) {
        auto v = q - centros[i];
        float ang = std::atan2(v.y, v.x);
        if (i == 2 && ang > 0)
            ang -= 2 * Pi;
        if (i == 1 && ang > 0)
            ang -= 2 * Pi;
        ang = std::clamp(ang, inicio[i] - Pi / 2, inicio[i]);
        probar(base[i] + (inicio[i] - ang) * 25);
    }
    auto m = muestraReferencia(sMejor);
    auto derecha = glm::normalize(glm::vec3(-m.tangente.z, 0, m.tangente.x));
    return {sMejor, mejor, glm::dot(p - m.posicion, derecha)};
}
// Toda la simulacion publica usa metros fisicos: s, lateral y proyecciones escalados.
float CircuitoCarreras::envolver(float s) {
    s = std::fmod(s, Longitud);
    return s < 0 ? s + Longitud : s;
}
MuestraCircuito CircuitoCarreras::muestra(float s, float lateral) {
    auto m = muestraReferencia(s / Escala, lateral / Escala);
    m.posicion.x *= Escala;
    m.posicion.z *= Escala;
    m.tangente =
        glm::normalize(glm::vec3(m.tangente.x * Escala, m.tangente.y, m.tangente.z * Escala));
    auto derecha = glm::normalize(glm::vec3(-m.tangente.z, 0, m.tangente.x));
    m.normal = glm::normalize(glm::cross(derecha, m.tangente));
    return m;
}
ProyeccionCircuito CircuitoCarreras::proyectar(glm::vec3 p) {
    p.x /= Escala;
    p.z /= Escala;
    auto r = proyectarReferencia(p);
    r.s *= Escala;
    r.distancia *= Escala;
    r.lateral *= Escala;
    return r;
}
std::vector<Vertice> CircuitoCarreras::construir() {
    std::vector<Vertice> v;
    const int pasos = 1080;
    for (int i = 0; i < pasos; ++i) {
        float s0 = Longitud * i / pasos, s1 = Longitud * (i + 1) / pasos;
        auto a = muestra(s0, -Ancho / 2), b = muestra(s1, -Ancho / 2), c = muestra(s1, Ancho / 2),
             d = muestra(s0, Ancho / 2);
        glm::vec3 color(.12f, .17f, .20f);
        // Caras superiores con normales analiticas continuas incluso sobre las rampas.
        for (auto m : {a, c, b, a, d, c})
            v.push_back({m.posicion, m.normal, color});
        for (float lado : {-Ancho / 2, Ancho / 2}) {
            auto x = muestra(s0, lado).posicion, y = muestra(s1, lado).posicion;
            auto z = y, w = x;
            z.y = 1.8f;
            w.y = 1.8f;
            if (lado < 0) {
                Geometria::triangulo(v, x, w, z, {.16f, .23f, .26f});
                Geometria::triangulo(v, x, z, y, {.16f, .23f, .26f});
            } else {
                Geometria::triangulo(v, x, y, z, {.16f, .23f, .26f});
                Geometria::triangulo(v, x, z, w, {.16f, .23f, .26f});
            }
        }
    }
    return v;
}
std::vector<Vertice> CircuitoCarreras::detalles() {
    std::vector<Vertice> v;
    const int pasos = 720;
    // Bandas finas elevadas 2 cm para evitar z fighting, ancho fisico de asfalto 12.6.
    for (int i = 0; i < pasos; ++i)
        for (float lado : {-6.65f * Escala, 6.65f * Escala}) {
            float a = Longitud * i / pasos, b = Longitud * (i + 1) / pasos;
            glm::vec3 color = i % 12 < 6 ? glm::vec3(.12f, .85f, .87f) : glm::vec3(.9f, .56f, .14f);
            auto p = muestra(a, lado - .16f * Escala).posicion,
                 q = muestra(b, lado - .16f * Escala).posicion,
                 r = muestra(b, lado + .16f * Escala).posicion,
                 s = muestra(a, lado + .16f * Escala).posicion;
            p.y += .025f;
            q.y += .025f;
            r.y += .025f;
            s.y += .025f;
            Geometria::triangulo(v, p, r, q, color);
            Geometria::triangulo(v, p, s, r, color);
        }
    const size_t inicioArco = v.size();
    for (int i = 0; i < 14; ++i)
        Geometria::caja(v, {83.5f + i, 3.04f, 0}, {1, .04f, 1.8f},
                        i % 2 ? glm::vec3(.94f) : glm::vec3(.04f));
    Geometria::caja(v, {81.6f, 7.5f, 0}, {1, 9, 1.7f}, {.16f, .3f, .34f});
    Geometria::caja(v, {98.4f, 7.5f, 0}, {1, 9, 1.7f}, {.16f, .3f, .34f});
    Geometria::caja(v, {90, 12, 0}, {18, 1.2f, 2}, {.14f, .25f, .29f});
    Geometria::caja(v, {90, 11.8f, 1.03f}, {13, .3f, .1f}, {.15f, .95f, .85f});
    for (size_t i = inicioArco; i < v.size(); ++i) {
        v[i].posicion.x *= Escala;
        v[i].posicion.z *= Escala;
    }
    return v;
}
