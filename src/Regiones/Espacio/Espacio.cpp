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
    estacion_ =
        std::make_unique<ModeloGltf>(c.recursos / "Modelos/EspacioBloque1/Freeport.gltf", true);
    naveAlien_ =
        std::make_unique<ModeloGltf>(c.recursos / "Modelos/EspacioBloque1/NaveAlien.gltf", true);
    holocron_ =
        std::make_unique<ModeloGltf>(c.recursos / "Modelos/EspacioBloque1/Holocron.gltf", true);
    for (int i = 0; i < 6; ++i)
        asteroides_[i] = std::make_unique<ModeloGltf>(
            c.recursos / ("Modelos/EspacioBloque1/Asteroide" + std::to_string(i) + ".gltf"), true);
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
    v.clear();
    Geometria::elipsoide(v, {0, 0, 0}, {1, 1, 1}, {.4f, .7f, 1.f}, 10, 6);
    luzNave_ = std::make_unique<Malla>(v);
    v.clear();
    // Dos muelles exteriores conectados al anillo; no requieren un interior ficticio.
    Geometria::caja(v, {0, -4.2f, 50}, {18, 1.4f, 26}, {.32f, .37f, .42f});
    Geometria::caja(v, {-50, -4.2f, 0}, {26, 1.4f, 18}, {.32f, .37f, .42f});
    for (int i = 0; i < 5; ++i) {
        Geometria::caja(v, {-7, -3.45f, 42.f + i * 4}, {.5f, .12f, 1.8f}, {.2f, .75f, .8f});
        Geometria::caja(v, {7, -3.45f, 42.f + i * 4}, {.5f, .12f, 1.8f}, {.2f, .75f, .8f});
        Geometria::caja(v, {-42.f - i * 4, -3.45f, 7}, {1.8f, .12f, .5f}, {.8f, .6f, .2f});
        Geometria::caja(v, {-42.f - i * 4, -3.45f, -7}, {1.8f, .12f, .5f}, {.8f, .6f, .2f});
    }
    Geometria::caja(v, {0, 8, 40}, {.35f, 20, .35f}, {.3f, .34f, .38f});
    Geometria::caja(v, {-40, 8, 0}, {.35f, 20, .35f}, {.3f, .34f, .38f});
    puertos_ = std::make_unique<Malla>(v);
}
glm::vec3 Espacio::direccion() const {
    return {std::sin(rumbo_) * std::cos(cabeceo_), std::sin(cabeceo_),
            -std::cos(rumbo_) * std::cos(cabeceo_)};
}
void Espacio::controlar(const OrdenActividad &o) {
    orden_ = o;
    if (o.luzAcople)
        luzAcople_ = !luzAcople_;
    if (o.luzServicio)
        luzServicio_ = !luzServicio_;
    rumboObjetivo_ += glm::clamp(o.mirada.x, -100.f, 100.f) * .0022f;
    cabeceoObjetivo_ =
        glm::clamp(cabeceoObjetivo_ - glm::clamp(o.mirada.y, -100.f, 100.f) * .0022f, -1.1f, 1.1f);
    if (o.confirmar)
        for (int i = 0; i < 2; ++i)
            if (glm::length(posicion_ - glm::vec3(i ? 210.f : -210.f, 0, 0)) < 28)
                destino_ = i ? Region::Planeta2 : Region::Planeta1;
    if (o.recuperar) {
        posicion_ = {-210, 0, 0};
        velocidad_ = 0;
        rumbo_ = rumboObjetivo_ = pi / 2;
        cabeceo_ = cabeceoObjetivo_ = ascenso_ = 0;
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
    if (glm::length(p - glm::vec3(-65, 30, -65)) < 12)
        return anterior;
    if (colisionFreeport(p + glm::vec3(0, 1.7f, 0), 3.4f))
        return anterior;
    for (int i = 0; i < CantidadAsteroides; ++i)
        if (glm::length(p - posicionAsteroide(i, tiempo_)) < radioAsteroide(i) + 2.2f)
            return anterior;
    return p;
}
glm::mat4 Espacio::matrizNave() const {
    auto frente = direccion();
    auto derecha = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));
    auto arriba = glm::cross(derecha, frente);
    glm::mat4 m(1);
    m[0] = glm::vec4(derecha, 0);
    m[1] = glm::vec4(arriba, 0);
    m[2] = glm::vec4(-frente, 0);
    m[3] = glm::vec4(posicion_, 1);
    return m;
}
void Espacio::actualizar(float dt) {
    if (dt <= 0)
        return;
    tiempo_ += dt;
    // Subpasos cortos mantienen la respuesta y las colisiones al variar los FPS.
    int pasos = std::max(1, int(std::ceil(dt * 120)));
    float h = dt / pasos;
    for (int i = 0; i < pasos; ++i) {
        rumboObjetivo_ += orden_.giro * h * .85f;
        float diferencia =
            std::atan2(std::sin(rumboObjetivo_ - rumbo_), std::cos(rumboObjetivo_ - rumbo_));
        float suavidad = 1 - std::exp(-8 * h);
        rumbo_ += diferencia * suavidad;
        cabeceo_ += (cabeceoObjetivo_ - cabeceo_) * suavidad;
        ascenso_ += (orden_.vertical * 32 - ascenso_) * suavidad;
        float limite = orden_.boost ? 70.f : 40.f;
        velocidad_ += (orden_.acelerador * (orden_.boost ? 28.f : 14.f) - orden_.freno * 34.f) * h;
        velocidad_ = std::clamp(velocidad_, 0.f, limite);
        if (!orden_.acelerador && !orden_.freno)
            velocidad_ *= std::exp(-.7f * h);
        auto propuesta = posicion_ + (direccion() * velocidad_ + glm::vec3(0, ascenso_, 0)) * h;
        auto nueva = ajustarMovimiento(posicion_, propuesta);
        if (glm::length(nueva - propuesta) > .001f)
            velocidad_ = ascenso_ = 0;
        posicion_ = nueva;
    }
}

// Volumen central, anillo hueco, cuatro brazos y ocho torres interiores.
// Son pruebas analiticas constantes; los huecos no forman una caja envolvente.
bool Espacio::colisionFreeport(glm::vec3 p, float m) {
    float r = glm::length(glm::vec2(p.x, p.z));
    float radio = p.y < -50 ? 8.f : p.y < -14 ? 3.2f : p.y < 14 ? 8.f : p.y < 43 ? 5.5f : 2.f;
    if (p.y >= -65 - m && p.y <= 65 + m && r < radio + m)
        return true;
    if (p.y > -5 - m && p.y < 1 + m && r > 34 - m && r < 41 + m)
        return true;
    for (int sx : {-1, 1})
        for (int sz : {-1, 1}) {
            glm::vec2 d(p.x - sx * 42.f, p.z - sz * 42.f);
            if (p.y > -34 - m && p.y < 31 + m && glm::length(d) < 10 + m)
                return true;
            float radial = (p.x * sx + p.z * sz) * .70710678f;
            float lateral = (p.x * sx - p.z * sz) * .70710678f;
            if (std::abs(radial - 51) < 20 + m && std::abs(lateral) < 12 + m &&
                std::abs(p.y + 3) < 2 + m)
                return true;
        }
    for (int k = 0; k < 8; ++k) {
        float angulo = (30.f + (k / 2) * 90.f + (k % 2) * 30.f) * pi / 180;
        if (p.y > -1 - m && p.y < 26 + m &&
            glm::length(glm::vec2(p.x, p.z) -
                        glm::vec2(std::cos(angulo), std::sin(angulo)) * 26.f) < 4.6f + m)
            return true;
    }
    if (std::abs(p.x) < 9 + m && std::abs(p.z - 50) < 13 + m && std::abs(p.y + 4.2f) < .7f + m)
        return true;
    if (std::abs(p.x + 50) < 13 + m && std::abs(p.z) < 9 + m && std::abs(p.y + 4.2f) < .7f + m)
        return true;
    return false;
}
namespace {
struct Tramo {
    float inicio, fin;
    glm::vec3 a, b, c, d;
};
// Los extremos comparten velocidad de 8 u/s; solo se frena al entrar al muelle.
const Tramo rutas[] = {{0, 8, {0, 18, 155}, {0, 18, 133.666667f}, {0, 6, 106.333333f}, {0, 6, 85}},
                       {8, 12, {0, 6, 85}, {0, 6, 74.333333f}, {0, .5f, 60}, {0, .5f, 52}},
                       {18, 24, {0, .5f, 52}, {0, .5f, 68}, {0, 24, 134}, {0, 24, 150}},
                       {24, 30, {0, 24, 150}, {0, 24, 166}, {65, 30, 226}, {65, 30, 210}},
                       {30, 40, {65, 30, 210}, {65, 30, 183.333333f}, {0, 18, 181.666667f}, {0, 18, 155}}};
glm::vec3 trayectoria(float t, bool tangente) {
    float f = std::fmod(std::max(0.f, t), 40.f);
    if (f >= 12 && f <= 18) {
        float yaw = 3.14159265f * glm::smoothstep(12.f, 18.f, f);
        return tangente ? glm::vec3(std::sin(yaw), 0, -std::cos(yaw)) : glm::vec3(0, .5f, 52);
    }
    const Tramo *r = &rutas[4];
    for (auto &tr : rutas)
        if (f >= tr.inicio && f <= tr.fin) {
            r = &tr;
            break;
        }
    float u = glm::clamp((f-r->inicio)/(r->fin-r->inicio),0.f,1.f);
    if(r->inicio==8) u=u+u*u-u*u*u;
    if(r->inicio==18) u=2*u*u-u*u*u;
    float v = 1 - u;
    if (tangente)
        return glm::normalize(3.f * v * v * (r->b - r->a) + 6.f * v * u * (r->c - r->b) +
                              3.f * u * u * (r->d - r->c));
    return v * v * v * r->a + 3.f * v * v * u * r->b + 3.f * v * u * u * r->c + u * u * u * r->d;
}
} // namespace
glm::vec3 Espacio::trafico(float t) const {
    return trayectoria(t, false);
}
glm::vec3 Espacio::direccionTrafico(int i, float t) const {
    auto d = trayectoria(t + i * 20.f, true);
    return i ? glm::vec3(-d.z, d.y, d.x) : d;
}
glm::mat4 Espacio::matrizTrafico(int i, float t) const {
    auto d = direccionTrafico(i, t);
    auto derecha = glm::normalize(glm::cross(d, glm::vec3(0, 1, 0)));
    glm::mat4 m(1);
    m[0] = glm::vec4(derecha, 0);
    m[1] = glm::vec4(glm::cross(derecha, d), 0);
    m[2] = glm::vec4(-d, 0);
    m[3] = glm::vec4(posicionTrafico(i, t), 1);
    // Banshee mira a -Z; el modelo Alien mira a +Z.
    return i ? glm::scale(m, glm::vec3(.76f)) : glm::rotate(m, pi, glm::vec3(0, 1, 0));
}

glm::vec3 Espacio::posicionTrafico(int i, float t) const {
    auto p = trafico(t + i * 20.f);
    return i ? glm::vec3(-p.z, p.y, p.x) : p;
}
glm::vec3 Espacio::posicionHolocron(float t) const {
    return posicionTrafico(0, t) + glm::vec3(10.f * std::cos(t * .45f),
                                             3 + 2.f * std::sin(t * .45f),
                                             10.f * std::sin(t * .45f));
}
glm::vec3 Espacio::posicionAsteroide(int i, float t) const {
    float a = i * 2.3999632f;
    glm::vec3 centro(145 + std::cos(a) * (24 + i * 2.f), -12 + (i % 5) * 10.f,
                     -125 + std::sin(a) * (20 + i * 1.6f));
    if (i % 2)
        centro += glm::vec3(9.f * std::cos(t * (.09f + (i % 4) * .025f) + a),
                            5.f * std::sin(t * (.12f + (i % 3) * .02f) + a),
                            7.f * std::sin(t * (.09f + (i % 4) * .025f) + a));
    return centro;
}
void Espacio::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    auto &r = c.renderizador;
    if (r.visible(glm::vec3(m * glm::vec4(0, 0, 0, 1)), 90))
        estacion_->dibujar(r, m);
    r.texturada(*puertos_, m, 0, {1, 1, 1}, {0, 0, 0}, .6f, .3f);
    r.texturada(*luzNave_, m * modelo({0, 18, 40}, {.65f, .4f, .65f}), 0, {.5f, .4f, .2f},
                luzAcople_ ? glm::vec3(.5f, .4f, .2f) : glm::vec3(0));
    r.texturada(*luzNave_, m * modelo({-40, 18, 0}, {.65f, .4f, .65f}), 0, {.2f, .4f, .5f},
                luzServicio_ ? glm::vec3(.2f, .4f, .5f) : glm::vec3(0));
    r.texturada(*esfera_, m * modelo(planetas[0], {70, 70, 70}, tiempo_ * .008f), halo_->id(),
                {1, 1, 1});
    r.texturada(*esfera_, m * modelo(planetas[1], {70, 70, 70}, -tiempo_ * .006f), orokin_->id(),
                {1, 1, 1});
    r.texturada(*esfera_, m * modelo({70, 90, -180}, {35, 35, 35}, tiempo_ * .015f), luna_->id(),
                {1, 1, 1});
    for (int i = 0; i < CantidadAsteroides; ++i) {
        auto pos = posicionAsteroide(i, tiempo_);
        float radio = radioAsteroide(i);
        if (r.visible(glm::vec3(m * glm::vec4(pos, 1)), radio * 1.1f))
            asteroides_[i % 6]->dibujar(
                r, m * modelo(pos, glm::vec3(radio),
                              i * .7f + (i % 2 ? tiempo_ * (.05f + (i % 4) * .025f) : 0)));
    }
    r.material(.45f, .6f);
    r.dibujar(*satelite_, m * modelo({-65, 30, -65}, {1, 1, 1}, tiempo_ * .2f));
    if (exterior_)
        nave_->dibujar(r, m * matrizNave());
    for (int i = 0; i < 2; ++i) {
        if (i)
            nave_->dibujar(r, m * matrizTrafico(i, tiempo_));
        else
            naveAlien_->dibujar(r, m * matrizTrafico(i, tiempo_));
    }
    holocron_->dibujar(r, m * modelo(posicionHolocron(tiempo_), {1, 1, 1}, tiempo_ * .2f));
}
void Espacio::liberar() noexcept {
    estacion_.reset();
    naveAlien_.reset();
    holocron_.reset();
    puertos_.reset();
    for (auto &a : asteroides_)
        a.reset();
    nave_.reset();
    esfera_.reset();
    satelite_.reset();
    luzNave_.reset();
    halo_.reset();
    orokin_.reset();
    luna_.reset();
}
Ambiente Espacio::ambiente() const noexcept {
    Ambiente a{{.001f, .002f, .008f}, {.4f, .8f, .3f}};
    a.cielo = "Espacio";
    a.iluminacion.activa = true;
    a.iluminacion.sol = glm::normalize(glm::vec3(-.6f, .5f, .8f));
    a.iluminacion.fuerzaSol = .85f;
    a.iluminacion.ambiente = .19f;
    for (int i = 0; i < 2; ++i) {
        auto &p = a.iluminacion.puntuales[i];
        p.posicion = posicionTrafico(i, tiempo_);
        p.color = {.35f, .65f, 1.f};
        p.intensidad = 2.2f;
        p.atenuacion = {1.f, .15f, .07f};
        p.encendida = true;
    }
    auto &f = a.iluminacion.focales[0];
    f.posicion = {0, 18, 40};
    f.direccion = glm::normalize(glm::vec3(0, -21, 14));
    f.color = {1, .82f, .57f};
    f.intensidad = 4.5f;
    f.interno = 22;
    f.externo = 36;
    f.encendida = luzAcople_;
    auto &g = a.iluminacion.focales[1];
    g.posicion = {-40, 18, 0};
    g.direccion = glm::normalize(glm::vec3(-14, -21, 0));
    g.color = {.4f, .72f, 1};
    g.intensidad = 4.5f;
    g.interno = 22;
    g.externo = 36;
    g.encendida = luzServicio_;
    return a;
}
EstadoActividad Espacio::actividad() const {
    EstadoActividad e;
    e.ocupado = true;
    e.objetivo = posicion_;
    e.direccion = direccion();
    e.mensajes = {"FREEPORT / U-I FOCOS",
                  "W-S MOTOR / A-D GIRA / MOUSE / Q-Z ALTURA / C RECUPERA / SHIFT BOOST"};
    for (int i = 0; i < 2; ++i)
        if (glm::length(posicion_ - glm::vec3(i ? 210.f : -210.f, 0, 0)) < 28)
            e.mensajes.push_back(i ? "ENTER / VISITAR PLANETA 2" : "ENTER / REGRESAR A PLANETA 1");
    return e;
}
