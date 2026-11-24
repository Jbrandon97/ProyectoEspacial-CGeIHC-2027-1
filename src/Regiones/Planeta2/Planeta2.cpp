#include "Planeta2.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float Pi = 3.14159265f;
constexpr float EscalaTerreno = .60f; // 600 originales -> 360 u, croquis compacto.
constexpr float LimiteX = 171.f;      // Dejar montanas en el borde, fuera del recorrido.
constexpr float LimiteZ = 151.f;
constexpr float PendienteMax = 1.2f;
const glm::vec3 PosSectorWarframe{-18, 0, 0};
const glm::vec3 PosSectorRatchet{18, 0, 0};
} // namespace

void Planeta2::cargar(ContextoEscena &c) {
    liberar();
    terreno_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Terreno2.gltf",true);
    alturas_ = std::make_unique<AlturasTerreno>(c.recursos / "Modelos/Alturas_201x201.f32");
    warframe_.cargar(c);
    ratchet_.usarSuelo([this](float x, float z) {
        return alturaTerreno(x + PosSectorRatchet.x, z + PosSectorRatchet.z);
    });
    ratchet_.cargar(c);
}
float Planeta2::alturaTerreno(float x, float z) const noexcept {
    return alturas_ ? EscalaTerreno * alturas_->altura(x/EscalaTerreno,z/EscalaTerreno) : 0.f;
}
float Planeta2::altura(float x, float z) const noexcept {
    return alturas_ ? ratchet_.superficie(x - PosSectorRatchet.x, z - PosSectorRatchet.z) : 0.f;
}
glm::vec3 Planeta2::centroPlaza() const {
    return ratchet_.centroPlaza() + PosSectorRatchet;
}
void Planeta2::controlar(const OrdenActividad &orden) {
    // Reaccion de NPC solo cerca del corral, sin interferir con controles de P1.
    ratchet_.registrarAvatar(orden.avatar - PosSectorRatchet);
    if(orden.luzServicio) reflectorOrokin_=!reflectorOrokin_;
    if (orden.luzAcople)
        reflectoresRailjack_ = !reflectoresRailjack_;
    if (orden.reflector)
        reflector_ = !reflector_;
}
void Planeta2::actualizar(float dt) {
    tiempo_ = std::fmod(tiempo_ + std::max(0.f, dt), DuracionCiclo);
    warframe_.actualizar(dt);
    ratchet_.actualizar(dt);
}
void Planeta2::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    const auto sector = glm::translate(m,PosSectorRatchet);
    const auto noche = ambiente().iluminacion.noche;
    ratchet_.dibujarPavimento(c.renderizador,sector,noche);
    if (terreno_)
        terreno_->dibujar(c.renderizador, glm::scale(m, glm::vec3(EscalaTerreno)));
    warframe_.dibujar(c, glm::translate(m, PosSectorWarframe));
    ratchet_.dibujar(c, sector, noche, reflectoresRailjack_);
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
    if (!choca(p))
        return p;
    glm::vec3 soloX{p.x, p.y, anterior.z}, soloZ{anterior.x, p.y, p.z};
    if (!choca(soloX))
        return soloX;
    if (!choca(soloZ))
        return soloZ;
    return anterior;
}
Ambiente Planeta2::ambiente() const noexcept {
    Ambiente a{{.13f, .18f, .28f}, {.55f, .63f, .82f}};
    a.cielo = "Planeta2";
    auto &l = a.iluminacion;
    l.activa = true;
    const float angulo = tiempo_ < SegundosDia ? Pi * (tiempo_ / SegundosDia)
        : Pi + Pi * ((tiempo_ - SegundosDia) / (DuracionCiclo - SegundosDia));
    l.sol = glm::normalize(glm::vec3(std::cos(angulo), std::sin(angulo), .21f));
    float dia = glm::smoothstep(-.15f, .2f, l.sol.y);
    l.noche = 1 - dia;
    l.fuerzaSol = .93f * glm::smoothstep(-.04f, .3f, l.sol.y);
    l.ambiente = .17f + .10f * dia;
    l.colorSol = glm::mix(glm::vec3(1, .56f, .4f), glm::vec3(.95f, .96f, 1),
                          glm::smoothstep(0.f, .65f, l.sol.y));
    // Seis luminarias de fachada sincronizadas con la noche.
    for (int i = 0; i < 6; ++i) {
        auto &p = l.puntuales[i];
        p.posicion = ratchet_.luzEdificio(size_t(i)) + PosSectorRatchet;
        p.color = i % 2 ? glm::vec3(.48f, .61f, 1.f) : glm::vec3(.75f, .42f, 1.f);
        p.intensidad = 3.5f * l.noche;
        p.atenuacion = {1, .09f, .018f};
        p.encendida = l.noche > .001f;
    }
    const auto nave = ratchet_.aphelionPosicion() + PosSectorRatchet;
    auto &aphelion = l.puntuales[7];
    aphelion.posicion = nave + glm::vec3(0, 0, 1);
    aphelion.color = {.25f, .64f, 1.f};
    aphelion.intensidad = 1.1f + 1.1f * l.noche;
    aphelion.atenuacion = {1, .11f, .045f};
    aphelion.encendida = true;
    // El indice 8 es EXCLUSIVO para la luz del Engineer (Aplicacion.cpp).
    auto &corral = l.puntuales[9];
    corral.posicion = {-69, altura(-69, 55) + 6.f, 55};
    corral.color = {.60f, .39f, 1.f};
    corral.intensidad = 3.2f * l.noche;
    corral.atenuacion = {1, .07f, .018f};
    corral.encendida = l.noche > .001f;
    auto &railjack = l.puntuales[10];
    railjack.posicion = ratchet_.railjackPosicion() + PosSectorRatchet;
    railjack.color = {.36f, .68f, 1.f};
    railjack.intensidad = 1.7f + 1.8f * l.noche;
    railjack.atenuacion = {1, .12f, .036f};
    railjack.encendida = true;

    auto &f = l.focales[0];
    const auto plaza = centroPlaza();
    f.posicion = plaza + glm::vec3(9,12,11);
    f.direccion = glm::normalize(glm::vec3(-9,-8,-11));
    f.color = {1.f,.85f,.62f};
    f.intensidad = 2.2f;
    f.interno = 22;
    f.externo = 38;
    f.atenuacion = {1, .035f, .003f};
    f.encendida = reflector_;
    auto &f2=l.focales[1];f2=f;f2.posicion={104,altura(83,-15)+20,6};
    f2.direccion=glm::normalize(glm::vec3(-21,-15,-21));f2.color={1,.79f,.4f};f2.intensidad=2.5f;
    f2.encendida=reflectorOrokin_;
    const glm::vec2 puntos[]={{plaza.x-8,plaza.z+9},{plaza.x+8,plaza.z-8}};
    for (int i = 0; i < 2; ++i) {
        auto &p = l.puntuales[11+i];
        const auto q = puntos[i];
        p.posicion = {q.x,altura(q.x,q.y)+5.f,q.y};
        p.color = {1,.75f,.45f};
        p.intensidad = 2.8f*l.noche;
        p.encendida = l.noche > .001f;
        p.atenuacion = {1,.045f,.010f};
    }
    const auto puerto = ratchet_.reflectoresRailjack(reflectoresRailjack_ ? l.noche : 0.f);
    for (size_t i = 0; i < puerto.size(); ++i) {
        l.focales[2+i] = puerto[i];
        l.focales[2+i].posicion += PosSectorRatchet;
    }
    auto &reflectorAphelion = l.focales.back();
    reflectorAphelion = ratchet_.reflectorAphelion(l.noche);
    reflectorAphelion.posicion += PosSectorRatchet;

    return a;
}
EstadoActividad Planeta2::actividad() const {
    EstadoActividad e;
    e.mensajes = {"PLANETA 2",
                  "WASD MOVER / SHIFT CORRER / MOUSE CAMARA / F1 EXPLORAR / F3 PLAZA / F4 WARFRAME",
                  "F ENERGIA / P PLAZA / I OROKIN / U RAILJACK DE NOCHE"};
    return e;
}
