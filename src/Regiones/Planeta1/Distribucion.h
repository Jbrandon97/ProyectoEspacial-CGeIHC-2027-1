#pragma once
#include <array>
#include <glm/glm.hpp>
// Coordenadas del plano numerado: norte = -Z. Derivadas del croquis, no aleatorias.
namespace Distribucion {
struct Edificio {
    const char *modelo;
    glm::vec2 centro;
    glm::vec2 semiejes;
};
inline const std::array<Edificio, 7> edificios = {{{"Covenant", {-145, -100}, {31, 33}},
                                                   {"Operaciones", {40, -115}, {16, 15}},
                                                   {"Habitat", {85, -145}, {15, 14}},
                                                   {"Taller", {90, -85}, {18, 15}},
                                                   {"Hangar", {165, -100}, {36, 27}},
                                                   {"Plataforma", {205, 15}, {28, 34}},
                                                   {"Recinto", {-65, 160}, {52, 28}}}};
struct Tramo {
    glm::vec2 a, b;
};
inline const std::array<Tramo, 15> caminos = {{{{-145, -63}, {-80, -40}},
                                               {{-80, -40}, {-20, 0}},
                                               {{-20, 0}, {60, 0}},
                                               {{60, 0}, {125, -35}},
                                               {{125, -35}, {165, -65}},
                                               {{70, -5}, {70, -60}},
                                               {{70, -60}, {40, -98}},
                                               {{70, -60}, {110, -60}},
                                               {{110, -60}, {120, -110}},
                                               {{120, -110}, {85, -128}},
                                               {{125, -35}, {205, -22}},
                                               {{205, -22}, {230, 65}},
                                               {{230, 65}, {230, 135}},
                                               {{-20, 0}, {-65, 130}},
                                               {{125, -35}, {145, 5}}}};
inline const std::array<glm::vec2, 28> arboles = {
    {{-212, 5},    {-205, 32},   {-196, 65},   {-184, 95},   {-165, 100}, {-140, 98},
     {-108, 89},   {-87, 67},    {-86, 35},    {-98, 8},     {-162, 8},   {-170, 28},
     {-177, 53},   {-180, -10},  {-215, -68},  {-215, -20},  {-184, -72}, {-177, -101},
     {-180, -136}, {-151, -146}, {-113, -141}, {-105, -106}, {-110, -72}, {-65, -85},
     {-62, -120},  {-55, 80},    {-190, 130},  {-158, 145}}};
inline float distancia(glm::vec2 p, Tramo t) {
    auto d = t.b - t.a;
    float u = glm::clamp(glm::dot(p - t.a, d) / glm::dot(d, d), 0.0f, 1.0f);
    return glm::length(p - t.a - u * d);
}
inline float camino(float x, float z) {
    float d = 10000;
    for (auto t : caminos)
        d = glm::min(d, distancia({x, z}, t));
    return d;
}
} // namespace Distribucion
