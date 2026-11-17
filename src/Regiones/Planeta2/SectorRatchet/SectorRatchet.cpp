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
constexpr glm::vec2 PatioRailjack{80.f, 65.f};
constexpr glm::vec2 Torre{65.f,-15.f};
struct Edificio {
    int tipo;
    glm::vec2 posicion;
    glm::vec2 medio;
};
// Tres edificios originales + una copia de cada uno. Se mantiene una plaza central.
constexpr std::array<Edificio, 6> Edificios{{
    {0, {-95, -20}, {2.4f, 1.9f}},
    {1, {-60, -25}, {3.5f, 4.8f}},
    {2, {-27, -12}, {1.6f, 1.7f}},
    {0, {-100, 32}, {2.4f, 1.9f}},
    {1, {-22, 50}, {3.5f, 4.8f}},
    {2, {-75, 65}, {1.6f, 1.7f}},
}};
// Posiciones fijas, no calculos aleatorios por fotograma.
constexpr std::array<glm::vec2, 29> Arboles{{
    {-115, -5},   {-125, -40}, {-130, 74}, {-116, 105}, {-95, 111},  {-55, 117},
    {-12, 115},   {21, 117},   {69, 105},  {111, 100},  {125, 34},   {119, -9},
    {125, -48},   {112, -99},  {76, -111}, {27, -107},  {-21, -108}, {-61, -114},
    {-123, -106}, {-59, -18},  {-35, -4},  {-14, 48},   {10, 82},    {70, 12},
    {88, -53},    {15, -75},   {-13, -18}, {-17, 71},   {-128, 15},
}};
// Un circuito amplio alrededor del escenario. Los puntos son locales a Ratchet.
constexpr std::array<glm::vec3, 10> RutaAphelion{{
    {-145, 88, -90},
    {-82, 102, -130},
    {4, 104, -127},
    {95, 100, -95},
    {135, 112, -19},
    {109, 94, 81},
    {34, 103, 125},
    {-49, 98, 110},
    {-122, 105, 55},
    {-145, 92, -20},
}};
constexpr float CicloAphelion = 78.f;

float suave(float a, float b, float x) {
    return glm::smoothstep(a, b, glm::clamp(x, a, b));
}

glm::vec3 spline(const glm::vec3 *p, int n, float reloj, float ciclo) {
    float fase = std::fmod(std::max(0.f, reloj), ciclo) * (float(n) / ciclo);
    int i = int(fase);
    float t = fase - float(i), t2 = t * t, t3 = t2 * t;
    const glm::vec3 &a = p[(i + n - 1) % n], &b = p[i % n], &c = p[(i + 1) % n],
                    &d = p[(i + 2) % n];
    return .5f * (2.f * b + (-a + c) * t + (2.f * a - 5.f * b + 4.f * c - d) * t2 +
                  (-a + 3.f * b - 3.f * c + d) * t3);
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
    if (f < 4.f)
        return 0;
    if (f < 8.f)
        return suave(4.f, 8.f, f);
    if (f < 20.f)
        return 1;
    return 1.f - suave(20.f, 24.f, f);
}
float ascensoRailjack(float t) {
    float f = std::fmod(t, 24.f);
    if (f < 8.f)
        return 0;
    if (f < 12.f)
        return 19.f * suave(8.f, 12.f, f);
    if (f < 16.f)
        return 19.f;
    if (f < 20.f)
        return 19.f * (1.f - suave(16.f, 20.f, f));
    return 0;
}
glm::mat4 colocar(glm::mat4 m, glm::vec3 pos, float scale) {
    return glm::scale(glm::translate(m, pos), glm::vec3(scale));
}
} // namespace

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    const auto modelo = [&c](const char *ruta, bool indexada = false) {
        return std::make_unique<ModeloGltf>(c.recursos / ruta, indexada);
    };
    edificio1_ = modelo("Modelos/Kerwan_Edificio1.gltf");
    edificio2_ = modelo("Modelos/Kerwan_Edificio2.gltf");
    edificio3_ = modelo("Modelos/Kerwan_Edificio3.gltf");
    railjack_ = modelo("Modelos/Railjack/Railjack_Optimizado.gltf", true);
    orokintower_ = modelo("Originales/OrokinTower/TorreOrokin1.gltf");
    warframe_ = modelo("Modelos/Warframe/Warframe.gltf",true);
    aphelion_ = modelo("Modelos/Aphelion/aphelion.gltf",true);
    clank_ = modelo("Modelos/Clank/Clank.gltf",true);
    blarg_ = modelo("Modelos/Blarg/Blarg.gltf",true);
    const auto alturaSuelo = [this](float x, float z) { return suelo_ ? suelo_(x, z) : 0.f; };
    const ModeloGltf *tipos[] = {edificio1_.get(), edificio2_.get(), edificio3_.get()};
    for (size_t i = 0; i < Edificios.size(); ++i) {
        const auto e = Edificios[i];
        const auto &vertices = tipos[e.tipo]->triangulos();
        glm::vec3 minimo(1e9f), maximo(-1e9f);
        for (auto v : vertices) {
            minimo = glm::min(minimo, v);
            maximo = glm::max(maximo, v);
        }
        float apoyo = 1e9f;
        // Cada punto inferior se compara con su terreno; se incluye el origen del glTF.
        for (auto v : vertices)
            if (v.y <= minimo.y + .05f) {
                float x = e.posicion.x + v.x * EscalaEdificios;
                float z = e.posicion.y + v.z * EscalaEdificios;
                apoyo = std::min(apoyo, alturaSuelo(x, z) - v.y * EscalaEdificios);
            }
        apoyos_[i] = apoyo - .12f;
        huecosBase_[i] = -1e9f;
        float hundimientoMinimo = 1e9f, hundimientoMaximo = -1e9f;
        for (auto v : vertices)
            if (v.y <= minimo.y + .05f) {
                float suelo = alturaSuelo(e.posicion.x + v.x * EscalaEdificios,
                                          e.posicion.y + v.z * EscalaEdificios);
                float hundimiento = suelo - (apoyos_[i] + v.y * EscalaEdificios);
                hundimientoMinimo = std::min(hundimientoMinimo, hundimiento);
                hundimientoMaximo = std::max(hundimientoMaximo, hundimiento);
                huecosBase_[i] =
                    std::max(huecosBase_[i], apoyos_[i] + v.y * EscalaEdificios - suelo);
            }
        contactoBase_[i] = {hundimientoMinimo, hundimientoMaximo};
        centrosEdificios_[i] = {e.posicion.x, apoyos_[i] + minimo.y * EscalaEdificios,
                                e.posicion.y};
        // La luz queda fuera de la fachada, no dentro del volumen opaco del edificio.
        glm::vec3 luz{e.posicion.x + (minimo.x + maximo.x) * EscalaEdificios * .5f, 0,
                      e.posicion.y + maximo.z * EscalaEdificios + 2.5f};
        luz.y = alturaSuelo(luz.x, luz.z) + 6.f;
        lucesEdificios_[i] = luz;
    }
    std::vector<Vertice> arbol;
    Geometria::caja(arbol, {0, 1.05f, 0}, {.52f, 2.1f, .52f}, {.38f, .27f, .49f});
    Geometria::elipsoide(arbol, {0, 3.5f, 0}, {2.35f, 2.30f, 2.35f}, {.62f, .35f, .85f}, 10, 6);
    arbol_ = std::make_unique<Malla>(arbol);
    std::vector<Vertice> piso;
    auto punto=[&](glm::vec2 v){return glm::vec3(v.x,alturaSuelo(v.x,v.y)+.06f,v.y);};
    auto camino=[&](glm::vec2 a,glm::vec2 b,float ancho){
        auto lado=glm::normalize(glm::vec2(-(b-a).y,(b-a).x))*ancho*.5f;
        int n=int(glm::length(b-a)/3)+1;
        for(int i=0;i<n;++i){
            auto c=glm::mix(a,b,float(i)/n),d=glm::mix(a,b,float(i+1)/n);
            glm::vec3 color=i%5 ? glm::vec3(.38f,.36f,.34f):glm::vec3(.42f,.40f,.37f);
            Geometria::triangulo(piso,punto(c-lado),punto(d+lado),punto(d-lado),color);
            Geometria::triangulo(piso,punto(c-lado),punto(c+lado),punto(d+lado),color);
        }
    };
    auto disco=[&](glm::vec2 centro,glm::vec2 radio){
        for(int i=0;i<64;++i){
            float a=2*Pi*i/64,b=2*Pi*(i+1)/64;
            auto u=centro+radio*glm::vec2(std::cos(a),std::sin(a));
            auto v=centro+radio*glm::vec2(std::cos(b),std::sin(b));
            Geometria::triangulo(piso,punto(centro),punto(v),punto(u),{.42f,.40f,.37f});
            auto medio=centro+radio*.94f*glm::vec2(std::cos(a),std::sin(a));
            Geometria::caja(piso,{medio.x,alturaSuelo(medio.x,medio.y)+.12f,medio.y},{.5f,.12f,.5f},{.75f,.57f,.23f});
        }
    };
    camino({-60,118},{-60,20},12);camino({-60,20},{65,20},12);
    camino({65,20},{65,-15},12);camino({65,20},PatioRailjack,14);
    camino({0,20},{0,-95},8);
    disco({-60,20},{28,28});disco(Torre,{24,24});disco(PatioRailjack,{30,36});disco({0,-95},{15,15});
    float torreMin=1e9f,railMin=1e9f;
    for(auto v:orokintower_->triangulos())torreMin=std::min(torreMin,v.y);
    for(auto v:railjack_->triangulos())railMin=std::min(railMin,v.y);
    apoyoTorre_=alturaSuelo(Torre.x,Torre.y)-torreMin*.85f-.1f;
    apoyoRailjack_=alturaSuelo(PatioRailjack.x,PatioRailjack.y)-railMin*EscalaRailjack+.12f;
    // Un pedestal compacto organiza la plaza sin introducir otro modelo descargado.
    Geometria::caja(piso,{-60,alturaSuelo(-60,20)+.7f,20},{3,1.4f,3},{.38f,.34f,.24f});
    for(auto v:std::array<glm::vec2,4>{{{-60,90},{-33,20},{20,20},{65,38}}}){
        float y=alturaSuelo(v.x,v.y);
        Geometria::caja(piso,{v.x,y+2.2f,v.y},{.4f,4.4f,.4f},{.3f,.32f,.34f});
        Geometria::caja(piso,{v.x,y+4.5f,v.y},{1,.25f,1},{.75f,.67f,.36f});
    }
    vallas_=std::make_unique<Malla>(piso);
    std::vector<Vertice> nube, luz;
    Geometria::elipsoide(nube, {0, 0, 0}, {1, .65f, 1}, {.54f, .60f, .67f}, 8, 4);
    Geometria::elipsoide(luz, {0, 0, 0}, {1, 1, 1}, {1, 1, 1}, 8, 5);
    humo_ = std::make_unique<Malla>(nube);
    propulsor_ = std::make_unique<Malla>(luz);
}

void SectorRatchet::actualizar(float dt) {
    tiempo_ = std::fmod(tiempo_ + std::max(0.f, dt), 780.f);
}
glm::vec3 SectorRatchet::aphelionPosicion() const {
    return ruta(tiempo_);
}
glm::vec3 SectorRatchet::railjackPosicion() const {
    return {PatioRailjack.x,
            (suelo_ ? suelo_(PatioRailjack.x, PatioRailjack.y) : 0.f) + ascensoRailjack(tiempo_) +
                9.f,
            PatioRailjack.y};
}
void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    auto &r = c.renderizador;
    auto alturaSuelo = [this](float x, float z) { return suelo_ ? suelo_(x, z) : 0.f; };
    const ModeloGltf *tipos[] = {edificio1_.get(), edificio2_.get(), edificio3_.get()};
    for (size_t i = 0; i < Edificios.size(); ++i) {
        const auto e = Edificios[i];
        if (tipos[e.tipo])
            tipos[e.tipo]->dibujar(
                r, colocar(m, {e.posicion.x, apoyos_[i], e.posicion.y}, EscalaEdificios));
    }
    if (orokintower_) {
        orokintower_->dibujar(r, colocar(m, {Torre.x, apoyoTorre_, Torre.y}, .85f));
    }
    if (arbol_)
        for (const auto &p : Arboles)
            r.dibujar(*arbol_, glm::translate(m, {p.x, alturaSuelo(p.x, p.y), p.y}));
    if (vallas_)
        r.dibujar(*vallas_, m);

    const ModeloGltf *npc[] = {clank_.get(), blarg_.get(), warframe_.get()};
    for (int i = 0; i < 3; ++i)
        if (npc[i]) {
            r.locomocionRobot(tiempo_ * 3.5f, i == 0);
            if(i==1) r.locomocionBlarg(tiempo_*3.2f);
            npc[i]->dibujar(r, m * matrizNPC(i, tiempo_));
        }
    r.locomocionRobot(0, false);

    // Railjack: detenido en suelo, abre piezas, asciende, espera, baja y se cierra.
    if (railjack_) {
        auto base = glm::translate(
            m, {PatioRailjack.x, apoyoRailjack_ + ascensoRailjack(tiempo_), PatioRailjack.y});
        base = glm::rotate(base, -.18f, {0, 1, 0});
        base = glm::scale(base, glm::vec3(EscalaRailjack));
        std::vector<glm::mat4> partes(20, glm::mat4(1));
        const float a = aperturaRailjack(tiempo_);
        for (int i = 2; i <= 18; ++i) {
            float signo = (i % 2) ? -1.f : 1.f;
            // No alterar el fuselaje: separar solamente partes laterales exportadas.
            partes[size_t(i)] = glm::translate(glm::mat4(1), {signo * 15.f * a, 5.f * a, 0});
        }
        railjack_->dibujar(r, base, 0, nullptr, &partes);
    }

    if (aphelion_) {
        aphelion_->dibujar(r, m * matrizAphelion(tiempo_));
        // Una pequena luz en la parte trasera y cuatro bocanadas reutilizando una malla.
        glm::vec3 p = ruta(tiempo_), d = rumbo(tiempo_);
        if (propulsor_)
            r.texturada(*propulsor_, colocar(m, p - d * 5.6f, .60f), 0, {.36f, .74f, 1.f},
                        {.10f, .24f, .42f});
        if (humo_)
            for (int i = 1; i <= 4; ++i) {
                float edad = float(i) * .28f;
                glm::vec3 q = ruta(tiempo_ - edad + CicloAphelion) -
                              rumbo(tiempo_ - edad + CicloAphelion) * 5.6f;
                q += glm::vec3(0, edad * .25f, 0);
                r.dibujar(*humo_, colocar(m, q, .18f + .16f * float(i)));
            }
    }
}
void SectorRatchet::liberar() noexcept {
    edificio1_.reset();
    edificio2_.reset();
    edificio3_.reset();
    railjack_.reset();
    orokintower_.reset();
    warframe_.reset();
    aphelion_.reset();
    clank_.reset();
    blarg_.reset();
    arbol_.reset();
    vallas_.reset();
    humo_.reset();
    propulsor_.reset();
}
bool SectorRatchet::obstaculo(glm::vec3 p, bool personajes) const {
    if (personajes)
        for (int i = 0; i < 3; ++i) {
            auto q = posicionNPC(i, tiempo_);
            if (glm::length(glm::vec2(p.x - q.x, p.z - q.z)) < (i == 0 ? 3.2f : 2.8f))
                return true;
        }
    for (const auto &e : Edificios) {
        if (std::abs(p.x - e.posicion.x) < e.medio.x * EscalaEdificios + 1.f &&
            std::abs(p.z - e.posicion.y) < e.medio.y * EscalaEdificios + 1.f)
            return true;
    }
    // Colision con la plataforma Railjack incluso durante el vuelo.
    if (glm::length(glm::vec2(p.x - PatioRailjack.x, p.z - PatioRailjack.y)) < 13.f)
        return true;
    if(glm::length(glm::vec2(p.x-Torre.x,p.z-Torre.y))<8.f)return true;
    if(glm::length(glm::vec2(p.x+60,p.z-20))<2.7f)return true;
    return false;
}

glm::vec3 SectorRatchet::posicionNPC(int i, float t) const {
    const glm::vec2 centros[] = {{-60, 20}, {5, 20}, {65, -15}};
    const glm::vec2 radios[] = {{18, 18}, {24, 3}, {18, 16}};
    const float velocidad[] = {.17f, .15f, .13f};
    float a = t * velocidad[i] + i * 1.7f;
    if(i==1) a-=.75f*std::sin(a);
    glm::vec3 p(centros[i].x + radios[i].x * std::sin(a), 0,
                centros[i].y + radios[i].y * std::cos(a));
    p.y = suelo_ ? suelo_(p.x, p.z) : 0;
    return p;
}
glm::mat4 SectorRatchet::matrizNPC(int i, float t) const {
    auto p = posicionNPC(i, t);
    auto d = posicionNPC(i, t + .02f) - posicionNPC(i, t - .02f);
    float angulo = std::atan2(d.x, d.z) - (i == 1 ? Pi / 2 : 0);
    const float escala[] = {.27f, .48f, 4.f};
    auto m = glm::translate(glm::mat4(1), p + glm::vec3(0, i == 2 ? .8f+.12f*std::sin(t*1.2f) : .025f, 0));
    m = glm::rotate(m, angulo, {0, 1, 0});
    return glm::scale(m, glm::vec3(escala[i]));
}
