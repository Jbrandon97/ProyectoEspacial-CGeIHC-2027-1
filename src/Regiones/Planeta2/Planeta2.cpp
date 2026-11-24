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

glm::vec2 plano(glm::vec3 p) { return {p.x, p.z}; }

bool bloquea(const SectorRatchet::Contacto &c, glm::vec2 anterior, glm::vec2 avance) {
    const auto desde = anterior - c.centro;
    const float radio = c.radio + Planeta2::RadioAvatar;
    const float radio2 = radio * radio;
    const float inicio2 = glm::dot(desde, desde);
    if (inicio2 < radio2)
        return glm::dot(desde, avance) < -1e-6f;
    const float longitud2 = glm::dot(avance, avance);
    const float t = glm::clamp(-glm::dot(desde, avance) / longitud2, 0.f, 1.f);
    const auto cerca = desde + t * avance;
    return glm::dot(cerca, cerca) + 1e-5f < radio2;
}
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
    galaxia_.cargar(c.recursos);
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
    galaxia_.actualizar(dt);
}
void Planeta2::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    const auto sector = glm::translate(m,PosSectorRatchet);
    const auto noche = ambiente().iluminacion.noche;
    ratchet_.dibujarPavimento(c.renderizador,sector,noche);
    if (terreno_)
        terreno_->dibujar(c.renderizador, glm::scale(m, glm::vec3(EscalaTerreno)));
    warframe_.dibujar(c, glm::translate(m, PosSectorWarframe));
    ratchet_.dibujar(c, sector, noche, reflectoresRailjack_);
    galaxia_.dibujar(c.renderizador, m, noche);
}
void Planeta2::liberar() noexcept {
    galaxia_.liberar();
    warframe_.liberar();
    ratchet_.liberar();
    terreno_.reset();
    alturas_.reset();
}
glm::vec3 Planeta2::ajustarMovimiento(glm::vec3 anterior, glm::vec3 p) const {
    p.x = glm::clamp(p.x, -LimiteX, LimiteX);
    p.z = glm::clamp(p.z, -LimiteZ, LimiteZ);
    const auto avance = plano(p - anterior);
    const float distancia = glm::length(avance);
    if (distancia < 1e-5f)
        return p;

    const int pasos = std::max(1, int(std::ceil(distancia / .35f)));
    const auto delta = glm::vec3(avance.x, 0, avance.y) / float(pasos);
    auto permitido = [&](glm::vec3 q) {
        const auto movimiento = plano(q - anterior);
        const float d2 = glm::dot(movimiento, movimiento);
        if (d2 < 1e-10f || ratchet_.obstaculo(q - PosSectorRatchet) ||
            altura(q.x, q.z) - altura(anterior.x, anterior.z) > PendienteMax * std::sqrt(d2))
            return false;
        const auto desde = plano(anterior - PosSectorRatchet);
        for (const auto &c : ratchet_.contactos())
            if (bloquea(c, desde, movimiento))
                return false;
        return true;
    };
    // Se intenta la diagonal y luego el deslizamiento por un eje.
    for (int i = 0; i < pasos; ++i) {
        const auto q = anterior + delta;
        const glm::vec3 x{q.x, q.y, anterior.z}, z{anterior.x, q.y, q.z};
        if (permitido(q))
            anterior = q;
        else if (permitido(x))
            anterior = x;
        else if (permitido(z))
            anterior = z;
        else
            break;
    }
    anterior.y = p.y;
    return anterior;
}

glm::vec3 Planeta2::separarAvatar(glm::vec3 posicion, float dt) const {
    if (dt <= 0)
        return posicion;
    const auto desde = plano(posicion - PosSectorRatchet);
    const auto &contactos = ratchet_.contactos();
    for (size_t i = 0; i < contactos.size(); ++i) {
        const auto &c = contactos[i];
        const auto diferencia = desde - c.centro;
        const float radio = c.radio + RadioAvatar;
        const float distancia2 = glm::dot(diferencia, diferencia);
        if (distancia2 >= radio * radio)
            continue;
        const float distancia = std::sqrt(distancia2);
        const auto normal = distancia > .001f ? diferencia / distancia
            : (glm::dot(c.avance, c.avance) > 1e-6f ? glm::normalize(c.avance) : glm::vec2(1, 0));
        const float penetracion = radio - distancia;
        const float avanceActor = std::max(0.f, glm::dot(c.avance, normal));
        const float limite = i < SectorRatchet::CantidadNPC ? 5.f : 10.f;
        const float alcance = std::min({penetracion, limite * dt,
                                        avanceActor + penetracion * (1.f - std::exp(-12.f * dt))});
        auto intentar = [&](glm::vec2 direccion) {
            return ajustarMovimiento(posicion, posicion + alcance * glm::vec3(direccion.x, 0, direccion.y));
        };
        const auto directa = intentar(normal);
        if (glm::length(plano(directa - posicion)) >= .75f * alcance)
            return directa;
        const glm::vec2 tangente{-normal.y, normal.x};
        const auto izquierda = intentar(tangente), derecha = intentar(-tangente);
        const float a = glm::length(plano(izquierda - posicion));
        const float b = glm::length(plano(derecha - posicion));
        if (std::max(a, b) > glm::length(plano(directa - posicion)))
            return a >= b ? izquierda : derecha;
        if (directa != posicion)
            return directa;
    }
    return posicion;
}

void Planeta2::ajustarLuzEngineer(LuzPuntual &luz) {
    luz.intensidad = 1.8f;
    luz.atenuacion = {1, .047f, .010f};
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
