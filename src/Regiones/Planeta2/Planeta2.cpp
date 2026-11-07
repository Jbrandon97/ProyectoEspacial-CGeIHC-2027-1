#include "Planeta2.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float Pi = 3.14159265f;
constexpr float EscalaTerreno = .60f; // 600 originales -> 360 u, croquis compacto.
constexpr float LimiteX = 171.f; // Dejar montanas en el borde, fuera del recorrido.
constexpr float LimiteZ = 151.f;
constexpr float PendienteMax = 1.2f;
const glm::vec3 PosSectorWarframe{-18, 0, 0};
const glm::vec3 PosSectorRatchet{18, 0, 0};
} // namespace

void Planeta2::cargar(ContextoEscena &c) {
    liberar();
    terreno_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Terreno2.gltf");
    alturas_ = std::make_unique<AlturasTerreno>(c.recursos / "Modelos/Alturas_201x201.f32");
    warframe_.cargar(c);
    ratchet_.usarSuelo([this](float x, float z) {
        return altura(x + PosSectorRatchet.x, z + PosSectorRatchet.z);
    });
    ratchet_.cargar(c);
}
void Planeta2::controlar(const OrdenActividad &orden) {
    // Reaccion de NPC solo cerca del corral, sin interferir con controles de P1.
    ratchet_.registrarAvatar(orden.avatar - PosSectorRatchet);
}
void Planeta2::actualizar(float dt) {
    tiempo_ = std::fmod(tiempo_ + std::max(0.f, dt), 120.f);
    warframe_.actualizar(dt);
    ratchet_.actualizar(dt);
}
void Planeta2::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    if (terreno_) terreno_->dibujar(c.renderizador, glm::scale(m, glm::vec3(EscalaTerreno)));
    warframe_.dibujar(c, glm::translate(m, PosSectorWarframe));
    ratchet_.dibujar(c, glm::translate(m, PosSectorRatchet));
}
void Planeta2::liberar() noexcept {
    warframe_.liberar();
    ratchet_.liberar();
    terreno_.reset();
    alturas_.reset();
}
glm::vec3 Planeta2::ajustarMovimiento(glm::vec3 anterior, glm::vec3 p) const {
    p.x = glm::clamp(p.x, -LimiteX, LimiteX);
    p.z = glm::clamp(p.z, -LimiteZ, LimiteZ);
    auto empinado = [this](glm::vec3 a, glm::vec3 b) {
        float d = glm::length(glm::vec2(b.x - a.x, b.z - a.z));
        return d > 1e-4f && (altura(b.x, b.z) - altura(a.x, a.z)) / d > PendienteMax;
    };
    auto choca = [&](glm::vec3 q) {
        return ratchet_.obstaculo(q - PosSectorRatchet) || empinado(anterior, q);
    };
    if (!choca(p)) return p;
    glm::vec3 soloX{p.x, p.y, anterior.z}, soloZ{anterior.x, p.y, p.z};
    if (!choca(soloX)) return soloX;
    if (!choca(soloZ)) return soloZ;
    return anterior;
}
Ambiente Planeta2::ambiente() const noexcept {
    Ambiente a{{.13f, .18f, .28f}, {.55f, .63f, .82f}};
    a.cielo = "Planeta2";
    auto &l = a.iluminacion;
    l.activa = true;
    // Igual que Planeta 1: 80 s de dia y 40 s de noche.
    float angulo = tiempo_ < 80 ? Pi * (tiempo_ / 80.f)
                                : Pi + Pi * ((tiempo_ - 80.f) / 40.f);
    l.sol = glm::normalize(glm::vec3(std::cos(angulo), std::sin(angulo), .21f));
    float dia = glm::smoothstep(-.15f, .2f, l.sol.y);
    l.noche = 1 - dia;
    l.fuerzaSol = .93f * glm::smoothstep(-.04f, .3f, l.sol.y);
    l.ambiente = .13f + .14f * dia;
    l.colorSol = glm::mix(glm::vec3(1, .56f, .4f), glm::vec3(.95f, .96f, 1),
                          glm::smoothstep(0.f, .65f, l.sol.y));
    // Cinco/seis edificios con luminarias concentradas. Solo de noche.
    const glm::vec3 edificios[] = {{26, 12, -15}, {69, 17, -27}, {56, 19, 45},
                                   {-7, 12, -52}, {112, 17, 63}, {-14, 19, 85}};
    for (int i = 0; i < 6; ++i) {
        auto &p = l.puntuales[i];
        p.posicion = edificios[i];
        p.posicion.y += altura(p.posicion.x, p.posicion.z);
        p.color = i % 2 ? glm::vec3(.48f, .61f, 1.f) : glm::vec3(.75f, .42f, 1.f);
        p.intensidad = 3.0f * l.noche;
        p.atenuacion = {1, .10f, .022f};
        p.encendida = l.noche > .001f;
    }
    const auto nave = ratchet_.aphelionPosicion() + PosSectorRatchet;
    auto &aphelion = l.puntuales[7];
    aphelion.posicion = nave + glm::vec3(0, 0, 1);
    aphelion.color = {.25f, .64f, 1.f};
    aphelion.intensidad = 1.1f + 1.1f*l.noche;
    aphelion.atenuacion = {1, .11f, .045f};
    aphelion.encendida = true;
    // El indice 8 es EXCLUSIVO para la luz del Engineer (Aplicacion.cpp).
    auto &corral = l.puntuales[9];
    corral.posicion = {-67, altura(-67, 35) + 6.f, 35};
    corral.color = {.60f, .39f, 1.f};
    corral.intensidad = 3.2f*l.noche;
    corral.atenuacion = {1, .07f, .018f};
    corral.encendida = l.noche > .001f;
    auto &railjack = l.puntuales[10];
    railjack.posicion = ratchet_.railjackPosicion() + PosSectorRatchet;
    railjack.color = {.36f, .68f, 1.f};
    railjack.intensidad = 1.7f + 1.8f*l.noche;
    railjack.atenuacion = {1, .12f, .036f};
    railjack.encendida = true;

    return a;
}
