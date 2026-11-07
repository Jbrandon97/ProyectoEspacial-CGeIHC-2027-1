#include "Espacio.h"
#include "Graficos/Renderizador.h"
#include "Graficos/Geometria.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
namespace {
constexpr float pi = 3.14159265f;
const glm::vec3 planetas[2] = {{-310, -60, 0}, {310, -60, 0}};
glm::mat4 modelo(glm::vec3 p, glm::vec3 escala, float giro = 0) {
    return glm::scale(glm::rotate(glm::translate(glm::mat4(1), p), giro, {0, 1, 0}), escala);
}
} // namespace
void Espacio::cargar(ContextoEscena &c) {
    liberar();
    gateway_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Gateway/Gateway.gltf");
    nave_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Banshee.gltf");
    halo_ = std::make_unique<Textura>(c.recursos / "Modelos/Espacio/PlanetaHalo.png");
    orokin_ = std::make_unique<Textura>(c.recursos / "Modelos/Espacio/PlanetaOrokin.png");
    luna_ = std::make_unique<Textura>(c.recursos / "Modelos/Espacio/Luna.png");
    std::vector<Vertice> v;
    auto punto = [](int x, int y) {
        float u = float(x) / 48, w = float(y) / 24;
        float a = 2 * pi * u, b = pi * w;
        glm::vec3 n{std::sin(b) * std::cos(a), std::cos(b), std::sin(b) * std::sin(a)};
        return Vertice{n, n, {1, 1, 1}, {u, w}};
    };
    for (int y = 0; y < 24; ++y)
        for (int x = 0; x < 48; ++x) {
            auto a = punto(x, y), b = punto(x + 1, y), c0 = punto(x, y + 1),
                 d = punto(x + 1, y + 1);
            v.insert(v.end(), {a, b, c0, b, d, c0});
        }
    esfera_ = std::make_unique<Malla>(v);
    v.clear();
    Geometria::elipsoide(v, {0, 0, 0}, {2, 2.5f, 2}, {.75f, .72f, .55f}, 16, 10);
    Geometria::caja(v, {0, 0, 0}, {17, .2f, .3f}, {.65f, .67f, .7f});
    for (int lado : {-1, 1})
        for (int i = 0; i < 5; ++i)
            Geometria::caja(v, {lado * (4.f + i * 1.15f), 0, 0}, {1, .13f, 5}, {.08f, .22f, .45f});
    Geometria::cono(v, {0, 2.5f, 0}, 1.8f, 1.2f, {.8f, .8f, .85f}, 20);
    Geometria::caja(v, {0, 4, 0}, {.12f, 2, .12f}, {.8f, .8f, .8f});
    satelite_ = std::make_unique<Malla>(v);
}
glm::vec3 Espacio::direccion() const {
    return {std::sin(rumbo_), 0, -std::cos(rumbo_)};
}
void Espacio::controlar(const OrdenActividad &o) {
    orden_ = o;
    if (o.confirmar)
        for (int i = 0; i < 2; ++i)
            if (glm::length(posicion_ - glm::vec3(i ? 210.f : -210.f, 0, 0)) < 28)
                destino_ = i ? Region::Planeta2 : Region::Planeta1;
    if (o.recuperar) {
        posicion_ = {-210, 0, 0};
        velocidad_ = 0;
        rumbo_ = pi / 2;
    }
}
std::optional<Region> Espacio::consumirDestino() {
    auto d = destino_;
    destino_.reset();
    return d;
}
glm::vec3 Espacio::ajustarMovimiento(glm::vec3 anterior, glm::vec3 p) const {
    if (glm::length(p) > 650)
        return anterior;
    for (auto centro : planetas)
        if (glm::length(p - centro) < 78)
            return anterior;
    if (glm::length(p - glm::vec3(70, 90, -180)) < 39)
        return anterior;
    // Volumen conservador de Gateway: protege sus paneles sin fingir un interior navegable.
    if (std::abs(p.x) < 24 && std::abs(p.y) < 46 && std::abs(p.z) < 37)
        return anterior;
    for (int i = 0; i < 5; ++i)
        if (glm::length(p - glm::vec3(95 + i * 19.f, -25 + i * 7.f, -80 - i * 14.f)) < 10)
            return anterior;
    return p;
}
void Espacio::actualizar(float dt) {
    tiempo_ += dt;
    rumbo_ += orden_.giro * dt * .8f;
    velocidad_ =
        std::clamp(velocidad_ + (orden_.acelerador * 18 - orden_.freno * 28) * dt, 0.f, 55.f);
    if (!orden_.acelerador && !orden_.freno)
        velocidad_ *= std::exp(-.6f * dt);
    auto propuesta =
        posicion_ + (direccion() * velocidad_ + glm::vec3(0, orden_.vertical * 22, 0)) * dt;
    auto nueva = ajustarMovimiento(posicion_, propuesta);
    if (glm::length(nueva - propuesta) > .001f)
        velocidad_ = 0;
    posicion_ = nueva;
}
glm::vec3 Espacio::trafico(float t) const {
    // Aproximacion, espera en puerto exterior y retirada, ciclo suave de 40 segundos.
    const float tiempos[] = {0, 12, 18, 28, 40};
    const glm::vec3 puntos[] = {
        {130, 15, 90}, {30, 0, 15}, {30, 0, 15}, {100, 25, -65}, {130, 15, 90}};
    float f = std::fmod(std::max(0.f, t), 40.f);
    int i = 0;
    while (i < 3 && f > tiempos[i + 1])
        ++i;
    float u = glm::smoothstep(tiempos[i], tiempos[i + 1], f);
    return glm::mix(puntos[i], puntos[i + 1], u);
}
void Espacio::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    auto &r = c.renderizador;
    gateway_->dibujar(r, m);
    r.texturada(*esfera_, m * modelo(planetas[0], {70, 70, 70}, tiempo_ * .008f), halo_->id(),
                {1, 1, 1});
    r.texturada(*esfera_, m * modelo(planetas[1], {70, 70, 70}, -tiempo_ * .006f), orokin_->id(),
                {1, 1, 1});
    r.texturada(*esfera_, m * modelo({70, 90, -180}, {35, 35, 35}, tiempo_ * .015f), luna_->id(),
                {1, 1, 1});
    for (int i = 0; i < 5; ++i)
        r.texturada(*esfera_,
                    m * modelo({95 + i * 19.f, -25 + i * 7.f, -80 - i * 14.f}, {4 + i * .4f, 6, 3},
                               tiempo_ * (.07f + i * .015f)),
                    luna_->id(), {.6f, .54f, .48f});
    r.material(.45f, .6f);
    r.dibujar(*satelite_, m * modelo({-65, 30, -65}, {1, 1, 1}, tiempo_ * .2f));
    nave_->dibujar(r, m * modelo(posicion_, {1, 1, 1}, -rumbo_));
    auto p = trafico(tiempo_), d = trafico(tiempo_ + .05f) - p;
    float giro = glm::length(d) > .0001f ? std::atan2(d.x, -d.z) : -pi / 2;
    nave_->dibujar(r, m * modelo(p, {.8f, .8f, .8f}, -giro));
}
void Espacio::liberar() noexcept {
    gateway_.reset();
    nave_.reset();
    esfera_.reset();
    satelite_.reset();
    halo_.reset();
    orokin_.reset();
    luna_.reset();
}
Ambiente Espacio::ambiente() const noexcept {
    Ambiente a{{.001f, .002f, .008f}, {.4f, .8f, .3f}};
    a.cielo = "Espacio";
    a.iluminacion.activa = true;
    a.iluminacion.sol = glm::normalize(glm::vec3(-.6f, .5f, .8f));
    a.iluminacion.fuerzaSol = 1.15f;
    a.iluminacion.ambiente = .22f;
    return a;
}
EstadoActividad Espacio::actividad() const {
    EstadoActividad e;
    e.ocupado = true;
    e.objetivo = posicion_;
    e.direccion = direccion();
    e.mensajes = {"ESPACIO / F3 NAVE / F4 OBSERVACION / 1-3 REGIONES",
                  "W ACELERA / S FRENA / A-D GIRA / Q-Z ALTURA / C RECUPERA"};
    for (int i = 0; i < 2; ++i)
        if (glm::length(posicion_ - glm::vec3(i ? 210.f : -210.f, 0, 0)) < 28)
            e.mensajes.push_back(i ? "ENTER / VISITAR PLANETA 2" : "ENTER / REGRESAR A PLANETA 1");
    return e;
}
