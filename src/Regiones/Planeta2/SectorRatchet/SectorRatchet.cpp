#include "SectorRatchet.h"
#include "Graficos/Geometria.h"
#include "Graficos/Renderizador.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace {
constexpr float Pi = 3.14159265358979323846f;
constexpr float EscalaEdificios = 2.f;
constexpr float EscalaRailjack = .16f;
constexpr glm::vec2 Corral{-85.f, 35.f};
constexpr glm::vec2 MedioCorral{24.f, 19.f};
constexpr glm::vec2 PatioRailjack{-94.f, -67.f};
struct Edificio {
    int tipo;
    glm::vec2 posicion;
    glm::vec2 medio;
};
// Tres edificios originales + una copia de cada uno. Se mantiene una plaza central.
constexpr std::array<Edificio, 6> Edificios{{
    {0, {8, -15}, {2.4f, 1.9f}},
    {1, {51, -27}, {3.5f, 4.8f}},
    {2, {38, 45}, {1.6f, 1.7f}},
    {0, {-25, -52}, {2.4f, 1.9f}},
    {1, {94, 63}, {3.5f, 4.8f}},
    {2, {-32, 85}, {1.6f, 1.7f}},
}};
// Posiciones fijas, no calculos aleatorios por fotograma.
constexpr std::array<glm::vec2, 29> Arboles{{
    {-115, -5}, {-125, -40}, {-130, 74}, {-116, 105}, {-95, 111},
    {-55, 117}, {-12, 115}, {21, 117}, {69, 105}, {111, 100},
    {125, 34}, {119, -9}, {125, -48}, {112, -99}, {76, -111},
    {27, -107}, {-21, -108}, {-61, -114}, {-123, -106},
    {-59, -18}, {-35, -4}, {-14, 48}, {10, 82}, {70, 12},
    {88, -53}, {15, -75}, {-13, -18}, {-17, 71}, {-128, 15},
}};
// Un circuito amplio alrededor del escenario. Los puntos son locales a Ratchet.
constexpr std::array<glm::vec3, 10> RutaAphelion{{
    {-145, 88, -90}, {-82, 102, -130}, {4, 104, -127},
    {95, 100, -95}, {135, 112, -19}, {109, 94, 81},
    {34, 103, 125}, {-49, 98, 110}, {-122, 105, 55}, {-145, 92, -20},
}};
constexpr float CicloAphelion = 78.f;

float suave(float a, float b, float x) {
    return glm::smoothstep(a, b, glm::clamp(x, a, b));
}

glm::vec3 spline(const glm::vec3 *p, int n, float reloj, float ciclo) {
    float fase = std::fmod(std::max(0.f, reloj), ciclo) * (float(n) / ciclo);
    int i = int(fase);
    float t = fase - float(i), t2 = t * t, t3 = t2 * t;
    const glm::vec3 &a = p[(i + n - 1) % n], &b = p[i % n],
                    &c = p[(i + 1) % n], &d = p[(i + 2) % n];
    return .5f * (2.f * b + (-a + c) * t +
                  (2.f*a - 5.f*b + 4.f*c - d) * t2 +
                  (-a + 3.f*b - 3.f*c + d) * t3);
}
glm::vec3 ruta(float t) {
    return spline(RutaAphelion.data(), int(RutaAphelion.size()), t, CicloAphelion);
}
glm::vec3 rumbo(float t) {
    glm::vec3 v = ruta(t + .2f) - ruta(t - .2f + CicloAphelion);
    return glm::length(v) < .0001f ? glm::vec3(0, 0, -1) : glm::normalize(v);
}
float giroCompleto(float t) {
    // Dos vueltas suaves durante la patrulla. Al cerrar cada ventana vuelve a 2*pi.
    float fase = std::fmod(t, CicloAphelion);
    return 2.f * Pi * (suave(18.f, 22.f, fase) + suave(51.f, 55.f, fase));
}
glm::mat4 matrizAphelion(float t) {
    glm::vec3 d = rumbo(t);
    float yaw = std::atan2(d.x, -d.z);
    float pitch = -std::asin(glm::clamp(d.y, -.85f, .85f));
    float curva = glm::cross(rumbo(t - 1.f + CicloAphelion), rumbo(t + 1.f)).y;
    auto m = glm::translate(glm::mat4(1), ruta(t));
    m = glm::rotate(m, yaw, {0, 1, 0});
    m = glm::rotate(m, pitch, {1, 0, 0});
    m = glm::rotate(m, glm::clamp(curva * 2.6f, -.32f, .32f) + giroCompleto(t), {0, 0, 1});
    return glm::scale(m, glm::vec3(1.65f));
}
float aperturaRailjack(float t) {
    float f = std::fmod(t, 24.f);
    if (f < 4.f) return 0;
    if (f < 8.f) return suave(4.f, 8.f, f);
    if (f < 20.f) return 1;
    return 1.f - suave(20.f, 24.f, f);
}
float ascensoRailjack(float t) {
    float f = std::fmod(t, 24.f);
    if (f < 8.f) return 0;
    if (f < 12.f) return 19.f * suave(8.f, 12.f, f);
    if (f < 16.f) return 19.f;
    if (f < 20.f) return 19.f * (1.f - suave(16.f, 20.f, f));
    return 0;
}
glm::mat4 colocar(glm::mat4 m, glm::vec3 pos, float scale) {
    return glm::scale(glm::translate(m, pos), glm::vec3(scale));
}
} // namespace

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    const auto modelo = [&c](const char *ruta) {
        return std::make_unique<ModeloGltf>(c.recursos / ruta);
    };
    edificio1_ = modelo("Modelos/Kerwan_Edificio1.gltf");
    edificio2_ = modelo("Modelos/Kerwan_Edificio2.gltf");
    edificio3_ = modelo("Modelos/Kerwan_Edificio3.gltf");
    railjack_ = modelo("Modelos/Railjack/RailjackSeparadoVibraAnimado.gltf");
    orokintower_ = modelo("Originales/OrokinTower/TorreOrokin1.gltf");
    warframe_ = modelo("Modelos/Warframe/Warframe.gltf");
    aphelion_ = modelo("Modelos/Aphelion/aphelion.gltf");
    clank_ = modelo("Modelos/Clank/Clank.gltf");
    blarg_ = modelo("Modelos/Blarg/Blarg.gltf");
    const auto alturaSuelo = [this](float x, float z) {
        return suelo_ ? suelo_(x, z) : 0.f;
    };
    for (size_t i = 0; i < Edificios.size(); ++i) {
        auto e = Edificios[i];
        float apoyo = alturaSuelo(e.posicion.x, e.posicion.y);
        // La base se hunde un poco en el terreno; no se deja suspendida.
        for (float x : {-e.medio.x * EscalaEdificios, e.medio.x * EscalaEdificios})
            for (float z : {-e.medio.y * EscalaEdificios, e.medio.y * EscalaEdificios})
                apoyo = std::min(apoyo, alturaSuelo(e.posicion.x + x, e.posicion.y + z));
        apoyos_[i] = apoyo - .15f;
    }
    std::vector<Vertice> arbol;
    Geometria::caja(arbol, {0, 1.05f, 0}, {.52f, 2.1f, .52f}, {.38f, .27f, .49f});
    Geometria::elipsoide(arbol, {0, 3.5f, 0}, {2.35f, 2.30f, 2.35f},
                        {.62f, .35f, .85f}, 10, 6);
    arbol_ = std::make_unique<Malla>(arbol);
    std::vector<Vertice> vallas;
    // Dos travesanos por pared y una entrada de 8 u al frente. Se ubican sobre el suelo.
    auto tramo = [&](float xa, float za, float xb, float zb) {
        float dx = xb - xa, dz = zb - za;
        float longitud = std::sqrt(dx*dx + dz*dz);
        int pasos = std::max(1, int(std::ceil(longitud / 5.f)));
        for (int i = 0; i <= pasos; ++i) {
            float t = float(i) / pasos;
            float x = glm::mix(xa, xb, t), z = glm::mix(za, zb, t);
            float y = alturaSuelo(x, z);
            Geometria::caja(vallas, {x, y + 1.3f, z}, {.28f, 2.6f, .28f}, {.52f, .64f, .82f});
        }
        for (int i = 0; i < pasos; ++i) {
            float t = (float(i) + .5f) / pasos;
            float x = glm::mix(xa, xb, t), z = glm::mix(za, zb, t);
            float y = alturaSuelo(x, z);
            glm::vec3 d = std::abs(dx) > std::abs(dz)
                ? glm::vec3(longitud / pasos + .25f, .23f, .28f)
                : glm::vec3(.28f, .23f, longitud / pasos + .25f);
            Geometria::caja(vallas, {x, y + 1.0f, z}, d, {.33f, .55f, .74f});
            Geometria::caja(vallas, {x, y + 2.4f, z}, d, {.47f, .34f, .75f});
        }
    };
    const float l = Corral.x - MedioCorral.x, r = Corral.x + MedioCorral.x;
    const float f = Corral.y - MedioCorral.y, b = Corral.y + MedioCorral.y;
    tramo(l, f, Corral.x - 4, f); tramo(Corral.x + 4, f, r, f); // entrada
    tramo(l, b, r, b); tramo(l, f, l, b); tramo(r, f, r, b);
    vallas_ = std::make_unique<Malla>(vallas);
    std::vector<Vertice> nube, luz;
    Geometria::elipsoide(nube, {0, 0, 0}, {1, .65f, 1}, {.54f, .60f, .67f}, 8, 4);
    Geometria::elipsoide(luz, {0, 0, 0}, {1, 1, 1}, {1, 1, 1}, 8, 5);
    humo_ = std::make_unique<Malla>(nube);
    propulsor_ = std::make_unique<Malla>(luz);
}

void SectorRatchet::actualizar(float dt) {
    tiempo_ = std::fmod(tiempo_ + std::max(0.f, dt), 780.f);
}
glm::vec3 SectorRatchet::aphelionPosicion() const { return ruta(tiempo_); }
glm::vec3 SectorRatchet::railjackPosicion() const {
    return {PatioRailjack.x, (suelo_ ? suelo_(PatioRailjack.x, PatioRailjack.y) : 0.f) +
                ascensoRailjack(tiempo_) + 9.f, PatioRailjack.y};
}
void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    auto &r = c.renderizador;
    auto alturaSuelo = [this](float x, float z) { return suelo_ ? suelo_(x, z) : 0.f; };
    const ModeloGltf *tipos[] = {edificio1_.get(), edificio2_.get(), edificio3_.get()};
    for (size_t i = 0; i < Edificios.size(); ++i) {
        const auto e = Edificios[i];
        if (tipos[e.tipo])
            tipos[e.tipo]->dibujar(r, colocar(m, {e.posicion.x, apoyos_[i], e.posicion.y}, EscalaEdificios));
    }
    if (orokintower_) {
        const glm::vec2 p{-35, -16};
        orokintower_->dibujar(r, colocar(m, {p.x, alturaSuelo(p.x, p.y), p.y}, .85f));
    }
    if (arbol_)
        for (const auto &p : Arboles)
            r.dibujar(*arbol_, glm::translate(m, {p.x, alturaSuelo(p.x, p.y), p.y}));
    if (vallas_) r.dibujar(*vallas_, m);

    // Tres NPC diferentes dentro del corral, cada uno con un ciclo independiente.
    if (clank_) {
        glm::vec3 q{-97.f + 3.4f * std::sin(tiempo_ * .44f), 0,
                    35.f + 4.5f * std::cos(tiempo_ * .44f)};
        q.y = alturaSuelo(q.x, q.z) + .12f * std::max(0.f, std::sin(tiempo_*3.f));
        auto t = glm::translate(m, q);
        t = glm::rotate(t, tiempo_* .44f + .42f * std::sin(tiempo_*1.5f), {0, 1, 0});
        clank_->dibujar(r, glm::scale(t, glm::vec3(.27f)));
    }
    if (blarg_) {
        glm::vec3 q{-74.f + 4.4f * std::sin(tiempo_ * .29f + 1.f), 0,
                    30.f + 5.f * std::cos(tiempo_*.29f + 1.f)};
        q.y = alturaSuelo(q.x, q.z) + .1f * std::abs(std::sin(tiempo_ * 2.f));
        auto t = glm::translate(m, q);
        glm::vec2 d = {avatarLocal_.x - q.x, avatarLocal_.z - q.z};
        float ang = glm::length(d) < 20.f ? std::atan2(d.x, d.y)
                                                : .29f * tiempo_ + 1.f;
        t = glm::rotate(t, ang, {0, 1, 0});
        t = glm::rotate(t, .08f*std::sin(tiempo_*2.f), {0, 0, 1});
        blarg_->dibujar(r, glm::scale(t, glm::vec3(.48f)));
    }
    if (warframe_) {
        const float f = std::fmod(tiempo_, 11.f);
        const float saludo = suave(2.f, 4.f, f) * (1.f - suave(7.f, 10.f, f));
        glm::vec3 q{-84, 0, 47};
        q.y = alturaSuelo(q.x, q.z) + .16f * saludo;
        auto t = glm::translate(m, q);
        t = glm::rotate(t, .46f * saludo, {0, 1, 0});
        t = glm::rotate(t, .16f * saludo, {0, 0, 1});
        warframe_->dibujar(r, glm::scale(t, glm::vec3(4.f)));
    }

    // Railjack: detenido en suelo, abre piezas, asciende, espera, baja y se cierra.
    if (railjack_) {
        float suelo = alturaSuelo(PatioRailjack.x, PatioRailjack.y);
        auto base = glm::translate(m, {PatioRailjack.x, suelo + 2.85f + ascensoRailjack(tiempo_), PatioRailjack.y});
        base = glm::rotate(base, -.18f, {0, 1, 0});
        base = glm::scale(base, glm::vec3(EscalaRailjack));
        std::vector<glm::mat4> partes(20, glm::mat4(1));
        const float a = aperturaRailjack(tiempo_);
        for (int i = 2; i <= 18; ++i) {
            float signo = (i % 2) ? -1.f : 1.f;
            // No alterar el fuselaje: separar solamente partes laterales exportadas.
            partes[size_t(i)] = glm::translate(glm::mat4(1), {signo * 15.f * a, 5.f*a, 0});
        }
        railjack_->dibujar(r, base, 0, nullptr, &partes);
        if (propulsor_ && ascensoRailjack(tiempo_) > .5f)
            r.texturada(*propulsor_, colocar(m, {PatioRailjack.x, suelo + .9f, PatioRailjack.y}, 1.7f),
                        0, {.32f, .68f, 1.f}, {.09f, .18f, .28f});
    }

    if (aphelion_) {
        aphelion_->dibujar(r, m * matrizAphelion(tiempo_));
        // Una pequena luz en la parte trasera y cuatro bocanadas reutilizando una malla.
        glm::vec3 p = ruta(tiempo_), d = rumbo(tiempo_);
        if (propulsor_)
            r.texturada(*propulsor_, colocar(m, p - d*5.6f, .60f), 0,
                        {.36f, .74f, 1.f}, {.10f, .24f, .42f});
        if (humo_)
            for (int i = 1; i <= 4; ++i) {
                float edad = float(i)* .28f;
                glm::vec3 q = ruta(tiempo_ - edad + CicloAphelion) - rumbo(tiempo_ - edad + CicloAphelion)*5.6f;
                q += glm::vec3(0, edad * .25f, 0);
                r.dibujar(*humo_, colocar(m, q, .18f + .16f * float(i)));
            }
    }
}
void SectorRatchet::liberar() noexcept {
    edificio1_.reset(); edificio2_.reset(); edificio3_.reset();
    railjack_.reset(); orokintower_.reset(); warframe_.reset(); aphelion_.reset();
    clank_.reset(); blarg_.reset();
    arbol_.reset(); vallas_.reset(); humo_.reset(); propulsor_.reset();
}
bool SectorRatchet::obstaculo(glm::vec3 p) const {
    for (const auto &e : Edificios) {
        if (std::abs(p.x-e.posicion.x) < e.medio.x*EscalaEdificios + 1.f &&
            std::abs(p.z-e.posicion.y) < e.medio.y*EscalaEdificios + 1.f)
            return true;
    }
    // Colision con la plataforma Railjack incluso durante el vuelo.
    if (glm::length(glm::vec2(p.x - PatioRailjack.x, p.z - PatioRailjack.y)) < 13.f)
        return true;
    // Valla cerrada excepto el acceso frontal de 8 unidades.
    float x = p.x - Corral.x, z = p.z - Corral.y;
    if (std::abs(x) < MedioCorral.x + 1.f && std::abs(z) < MedioCorral.y + 1.f) {
        if (std::abs(std::abs(x) - MedioCorral.x) < 1.2f)
            return true;
        if (std::abs(z - MedioCorral.y) < 1.2f)
            return true;
        if (std::abs(z + MedioCorral.y) < 1.2f && std::abs(x) > 4.1f)
            return true;
    }
    return false;
}
