#include "SistemasPlaneta1.h"
#include "TerrenoPlaneta1.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
void SistemasPlaneta1::configurar(const std::filesystem::path &p) {
    std::ifstream f(p);
    std::string linea;
    while (std::getline(f, linea)) {
        auto k = linea.find('=');
        if (k == std::string::npos || linea[0] == '#')
            continue;
        try {
            float v = std::stof(linea.substr(k + 1));
            if (!std::isfinite(v))
                continue;
            if (linea.substr(0, k) == "ciclo_segundos")
                ciclo = std::clamp(v, 12.f, 120.f);
            if (linea.substr(0, k) == "fase_inicial")
                faseInicial = v - std::floor(v);
        } catch (...) {
        }
    }
}
void SistemasPlaneta1::actualizar(float dt) {
    tiempo += dt;
    if (lanzamiento >= 0)
        lanzamiento = std::min(22.f, lanzamiento + dt);
    if (reaccion >= 0) {
        reaccion += dt;
        if (reaccion >= 5)
            reaccion = -1;
    }
}
float SistemasPlaneta1::fase() const {
    return std::fmod(faseInicial + tiempo / ciclo, 1.f);
}
float SistemasPlaneta1::faseSolar() const {
    // 2/3 del tiempo sobre el horizonte, 1/3 bajo el: 80 s / 40 s.
    // Hermite mantiene velocidad angular continua en amanecer y atardecer.
    float f = fase(), limite = 2.f / 3.f;
    bool dia = f < limite;
    float u = dia ? f / limite : (f - limite) / (1 - limite);
    float pendiente = dia ? 4.f / 3.f : 2.f / 3.f;
    float avance = u + (pendiente - 1) * u * (1 - u) * (1 - 2 * u);
    return (dia ? 0.f : .5f) + .5f * avance;
}
Iluminacion SistemasPlaneta1::luces() const {
    Iluminacion l;
    l.activa = true;
    float a = faseSolar() * 6.2831853f;
    l.sol = glm::normalize(glm::vec3(std::cos(a), std::sin(a), .2f * std::cos(a)));
    float dia = glm::smoothstep(-.15f, .2f, l.sol.y);
    l.noche = 1 - dia;
    l.fuerzaSol = glm::smoothstep(-.04f, .3f, l.sol.y) * .95f;
    l.ambiente = .15f + .115f * dia;
    l.colorSol = glm::mix(glm::vec3(1, .48f, .23f), glm::vec3(1, .98f, .9f),
                          glm::smoothstep(0.f, .55f, l.sol.y));
    const glm::vec3 pos[] = {{165, 11, -72}, {40, 9, -95},  {90, 9, -66},
                             {85, 9, -122},  {205, 11, -7}, {-145, 12, -63}};
    for (int i = 0; i < 6; ++i) {
        auto &p = l.puntuales[i];
        p.posicion = pos[i];
        p.color = i == 5   ? glm::vec3(.35f, .6f, 1)
                  : i == 4 ? glm::vec3(.4f, .85f, 1)
                           : glm::vec3(1, .68f, .38f);
        p.intensidad = 5 * l.noche;
        p.encendida = l.noche > .001f;
    }
    // Reutilizar las dos plazas libres: luz corta solidaria a cada patrulla.
    for (int i = 0; i < 2; ++i) {
        auto &p = l.puntuales[6 + i];
        p.posicion = sentinel(i) + glm::vec3(0, -.3f, 0);
        p.color = {.35f, .65f, 1.f};
        p.intensidad = 3.f * l.noche;
        p.atenuacion = {1.f, .35f, .22f};
        p.encendida = l.noche > .001f;
    }
    return l;
}
glm::vec3 SistemasPlaneta1::cohete() const {
    float t = std::max(0.f, lanzamiento - 3);
    return {205 + std::max(0.f, t - 7) * 1.8f, 5.22f + .85f * t * t, 21};
}
glm::vec3 SistemasPlaneta1::sentinel(int i) const {
    glm::vec2 c = i == 0 ? glm::vec2(-110, -45) : glm::vec2(123, -38);
    float a = tiempo * (i == 0 ? .22f : -.18f) + i * 2;
    float x = c.x + 12 * std::cos(a), z = c.y + 8 * std::sin(a);
    return {x, TerrenoPlaneta1::altura(x, z) + 9 + .4f * std::sin(tiempo * 1.3f + i), z};
}
float SistemasPlaneta1::giroSentinel(int i) const {
    float v = i == 0 ? .22f : -.18f, a = tiempo * v + i * 2;
    return std::atan2(-12 * std::sin(a) * v, 8 * std::cos(a) * v);
}
glm::mat4 SistemasPlaneta1::animacionNPC(int i) const {
    glm::mat4 m(1);
    if (i == 0 && reaccion >= 0) {
        float u = reaccion / 5;
        float caminar = std::sin(u * 3.14159265f);
        m = glm::translate(m, {0, 0, 2.8f * caminar});
        return glm::rotate(m, .4f * std::sin(u * 6.2831853f), {0, 1, 0});
    }
    if (i == 2)
        return glm::rotate(m, .22f * std::sin(tiempo * .8f), glm::vec3(0, 1, 0));
    if (i == 1 || i == 3) {
        // Dos objetos distintos: Grunt 2 inspecciona; Hunter gira en vigilancia.
        const float t[] = {0, 2, 4, 6, 8};
        const float giro[] = {0, .5f, -.35f, .25f, 0};
        const float incl[] = {0, .12f, .05f, -.07f, 0};
        float u = std::fmod(tiempo + (i == 3 ? 1.f : 0.f), 8.f);
        int k = 0;
        while (k < 3 && u > t[k + 1])
            ++k;
        float s = glm::smoothstep(0.f, 1.f, (u - t[k]) / (t[k + 1] - t[k]));
        m = glm::rotate(m, glm::mix(giro[k], giro[k + 1], s) * (i == 3 ? .65f : 1.f), {0, 1, 0});
        if (i == 1)
            m = glm::rotate(m, glm::mix(incl[k], incl[k + 1], s), {1, 0, 0});
    }
    return m;
}
