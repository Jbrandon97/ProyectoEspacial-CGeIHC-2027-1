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
constexpr glm::vec2 Corral{-86.f, 54.f};
constexpr glm::vec2 Plaza{-36.f, 15.f};
constexpr glm::vec2 PatioRailjack{80.f, 65.f};
constexpr glm::vec2 Torre{65.f,-15.f};
constexpr glm::vec2 CentroPatrulla{-18.f, 0.f};
constexpr float RadioPatrulla = 122.f, Esquina = 8.f, VelocidadPatrulla = 7.5f;
constexpr double CuartoPatrulla = 2 * (RadioPatrulla - Esquina) + Pi * Esquina / 2;
constexpr float RadioPuerto = 23.f;
constexpr int MuestrasPavimento = 171;
constexpr float PasoPavimento = 1.8f;
constexpr glm::vec2 InicioPavimento{-171.f, -153.f};
constexpr float PieBanqueta = -.58f, InicioBanqueta = .10f, FinBanqueta = .75f,
                PieExterior = 1.48f, AlturaBanqueta = .18f;
struct Edificio {
    int tipo;
    glm::vec2 posicion;
    glm::vec2 medio;
    float giro;
};
constexpr std::array<Edificio, 6> Edificios{{
    {0, {-105, -24}, {2.1f, 1.72f}, .22f},
    {1, {-88, -46}, {3.3f, 4.72f}, -.24f},
    {2, {-78, -15}, {1.5f, 1.49f}, .38f},
    {0, {-115, -50}, {2.1f, 1.72f}, .70f},
    {1, {-29, -87}, {3.3f, 4.72f}, .06f},
    {2, {-117, -12}, {1.5f, 1.49f}, -.50f},
}};
struct Calle { glm::vec2 a, b; float medio; };
constexpr std::array<Calle, 14> Calles{{
    {{-60, 122}, {-60, 75}, 4.6f}, {{-60, 75}, Plaza, 4.6f},
    {Plaza, {-95, -4}, 5.f}, {{-95, -4}, {-129, -4}, 4.f},
    {{-129, -4}, {-140, -4}, 4.f}, {{-95, -4}, {-99, -42}, 3.6f},
    {Plaza, Corral, 4.f}, {Plaza, {8, 15}, 5.f},
    {{8, 15}, {58, -9}, 5.f}, {{8, 15}, {24, 49}, 4.5f},
    {{24, 49}, PatioRailjack, 4.5f}, {Plaza, {5,-75}, 4.4f},
    {{5,-75},{5,-122},4.4f},
    {{80,86},{80,122},4.4f},
}};
constexpr std::array<glm::vec2, 4> SitiosFocos{{
    {65, 50}, {95, 50}, {95, 80}, {65, 80}
}};
constexpr std::array<glm::vec2, 6> SitiosUtileria{{
    {-103, 8}, {-86, 55}, {59, 69}, {59, 73}, {-40, 55}, {70, -30}
}};
constexpr std::array<int, 6> TipoUtileria{{0, 1, 2, 2, 3, 3}};
constexpr float EscalaTorre = .85f;
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
    {-54, 35, -35}, {-29, 32, -50}, {15, 29, -42}, {48, 34, -28},
    {51, 31, 6}, {34, 27, 28}, {10, 24, 46}, {-35, 24, 64},
    {-53, 27, 49}, {-52, 32, 14},
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
    float yaw = std::atan2(d.x, d.z);
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
glm::vec2 rotarXZ(glm::vec2 p, float angulo) {
    float c = std::cos(angulo), s = std::sin(angulo);
    return {c * p.x + s * p.y, -s * p.x + c * p.y};
}
glm::vec2 esquinaPatrulla(double s) {
    s = std::fmod(s, 4 * CuartoPatrulla);
    if (s < 0)
        s += 4 * CuartoPatrulla;
    const int lado = int(s / CuartoPatrulla);
    const float d = float(s - lado * CuartoPatrulla);
    const float tramo = 2 * (RadioPatrulla - Esquina), cuartoArco = Pi * Esquina / 4;
    const float c = RadioPatrulla - Esquina;
    glm::vec2 p;
    if (d < cuartoArco) {
        const float a = -3 * Pi / 4 + d / Esquina;
        p = glm::vec2(-c, -c) + Esquina * glm::vec2(std::cos(a), std::sin(a));
    } else if (d < cuartoArco + tramo) {
        p = {-c + d - cuartoArco, -RadioPatrulla};
    } else {
        const float a = -Pi / 2 + (d - cuartoArco - tramo) / Esquina;
        p = glm::vec2(c, -c) + Esquina * glm::vec2(std::cos(a), std::sin(a));
    }
    return CentroPatrulla + rotarXZ(p, -lado * Pi / 2);
}
float distanciaSegmento(glm::vec2 p, glm::vec2 a, glm::vec2 b) {
    const auto ab = b - a;
    const float f = glm::clamp(glm::dot(p - a, ab) / std::max(.00001f, glm::dot(ab, ab)), 0.f, 1.f);
    return glm::length(p - a - f * ab);
}
float distanciaAnillo(glm::vec2 p) {
    const auto q = glm::abs(p - CentroPatrulla) - (RadioPatrulla - Esquina);
    return std::abs(glm::length(glm::max(q, glm::vec2(0))) +
                    std::min(std::max(q.x, q.y), 0.f) - Esquina);
}
float bordePavimento(glm::vec2 p) {
    float d = std::min(distanciaAnillo(p) - 3.9f, glm::length(p - Plaza) - 18.f);
    d = std::min(d, glm::length(p - Torre) - 18.5f);
    d = std::min(d, glm::length(p - PatioRailjack) - RadioPuerto);
    d = std::min(d, glm::length((p - Corral) * glm::vec2(1.f, 1.12f)) - 16.f);
    for (const auto &c : Calles)
        d = std::min(d, distanciaSegmento(p, c.a, c.b) - c.medio);
    return d;
}
float perfilBanqueta(float distancia) {
    const float subir = (distancia-PieBanqueta)/(InicioBanqueta-PieBanqueta);
    const float bajar = (PieExterior-distancia)/(PieExterior-FinBanqueta);
    return AlturaBanqueta * glm::clamp(std::min(subir,bajar),0.f,1.f);
}
Renderizador::Volumen volumenHaz(const glm::mat4 &m, const LuzFocal &l, float alcance, float opacidad) {
    const auto y = l.direccion;
    const auto x = glm::normalize(glm::cross(y, glm::vec3(0,0,1)));
    const auto z = glm::cross(x,y);
    glm::mat4 base(glm::vec4(x*alcance,0),glm::vec4(y*alcance,0),
                   glm::vec4(z*alcance,0),glm::vec4(l.posicion,1));
    return {m*base,l.color,opacidad};
}
void cilindro(std::vector<Vertice> &v, glm::vec3 p, float radio, float alto, glm::vec3 color,
              int lados) {
    for (int i = 0; i < lados; ++i) {
        const float a = 2 * Pi * i / lados, b = 2 * Pi * (i + 1) / lados;
        const auto u = p + radio * glm::vec3(std::cos(a), 0, std::sin(a));
        const auto w = p + radio * glm::vec3(std::cos(b), 0, std::sin(b));
        const glm::vec3 y(0, alto, 0);
        Geometria::triangulo(v, p + y, w + y, u + y, color);
        Geometria::triangulo(v, u, u + y, w + y, color);
        Geometria::triangulo(v, u, w + y, w, color);
    }
}
float cruz(glm::vec2 a, glm::vec2 b) { return a.x * b.y - a.y * b.x; }
bool tocaTriangulo(glm::vec2 p, const std::array<glm::vec2, 3> &t) {
    bool positivo = false, negativo = false;
    for (size_t i = 0; i < t.size(); ++i) {
        const auto a = t[i], b = t[(i + 1) % t.size()];
        if (distanciaSegmento(p, a, b) < .72f)
            return true;
        float signo = cruz(b - a, p - a);
        positivo = positivo || signo > 0;
        negativo = negativo || signo < 0;
    }
    return std::abs(cruz(t[1] - t[0], t[2] - t[0])) > .0001f && !(positivo && negativo);
}
} // namespace

void SectorRatchet::cargar(ContextoEscena &c) {
    liberar();
    const auto modelo = [&c](const char *ruta, bool indexada = false) {
        return std::make_unique<ModeloGltf>(c.recursos / ruta, indexada);
    };
    edificio1_ = std::make_unique<ModeloGltf>(c.recursos/"Modelos/Kerwan_Edificio1.gltf",true,true);
    edificio2_ = std::make_unique<ModeloGltf>(c.recursos/"Modelos/Kerwan_Edificio2.gltf",true,true);
    edificio3_ = std::make_unique<ModeloGltf>(c.recursos/"Modelos/Kerwan_Edificio3.gltf",true,true);
    railjack_ = modelo("Modelos/Railjack/Railjack_Optimizado.gltf", true);
    orokintower_ = modelo("Originales/OrokinTower/TorreOrokin1.gltf");
    warframe_ = modelo("Modelos/Warframe/Warframe.gltf",true);
    aphelion_ = modelo("Modelos/Aphelion/aphelion.gltf",true);
    clank_ = modelo("Modelos/Clank/Clank.gltf",true);
    blarg_ = modelo("Modelos/Blarg/Blarg.gltf",true);
    estatua_ = modelo("Modelos/P2_Kerwan/Qwark/Qwark.gltf", true);
    munecos_[0] = modelo("Modelos/P2_Kerwan/MunecoRatchet/MunecoRatchet.gltf", true);
    munecos_[1] = modelo("Modelos/P2_Kerwan/MunecoClank/MunecoClank.gltf", true);
    utileria_[0] = modelo("Modelos/P2_Kerwan/Vendor/Vendor.gltf", true);
    utileria_[1] = modelo("Modelos/P2_Kerwan/Pad/Pad.gltf", true);
    utileria_[2] = modelo("Modelos/P2_Kerwan/Caja/Caja.gltf", true);
    utileria_[3] = modelo("Modelos/P2_Kerwan/Nanotech/Nanotech.gltf", true);
    partesRailjack_.resize(railjack_->cantidadNodos(), glm::mat4(1));
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
                auto p = e.posicion + rotarXZ(glm::vec2(v.x, v.z) * EscalaEdificios, e.giro);
                apoyo = std::min(apoyo, alturaSuelo(p.x, p.y) - v.y * EscalaEdificios);
            }
        apoyos_[i] = apoyo - .12f;
        huecosBase_[i] = -1e9f;
        float hundimientoMinimo = 1e9f, hundimientoMaximo = -1e9f;
        for (auto v : vertices)
            if (v.y <= minimo.y + .05f) {
                auto p = e.posicion + rotarXZ(glm::vec2(v.x, v.z) * EscalaEdificios, e.giro);
                float suelo = alturaSuelo(p.x, p.y);
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
        const auto q = e.posicion + rotarXZ({(minimo.x + maximo.x) * EscalaEdificios * .5f,
                                           maximo.z * EscalaEdificios + 2.5f}, e.giro);
        glm::vec3 luz{q.x, 0, q.y};
        luz.y = alturaSuelo(luz.x, luz.z) + 6.f;
        lucesEdificios_[i] = luz;
    }
    std::vector<Vertice> arbol;
    Geometria::caja(arbol, {0, 1.05f, 0}, {.52f, 2.1f, .52f}, {.38f, .27f, .49f});
    Geometria::elipsoide(arbol, {0, 3.5f, 0}, {2.35f, 2.30f, 2.35f}, {.62f, .35f, .85f}, 10, 6);
    arbol_ = std::make_unique<Malla>(arbol);
    pisoPuerto_ = alturaSuelo(PatioRailjack.x, PatioRailjack.y);
    for (int x = -14; x <= 14; x += 2)
        for (int z = -14; z <= 14; z += 2)
            if (x * x + z * z <= 196)
                pisoPuerto_ = std::max(pisoPuerto_, alturaSuelo(PatioRailjack.x+x, PatioRailjack.y+z));
    pisoPuerto_ += .14f;
    apoyoRailjack_ = pisoPuerto_ - railjack_->minimoY() * EscalaRailjack + .12f;
    apoyarTorre();
    construirUrbanismo();
    for (auto p : Arboles)
        if (bordePavimento(p) > 3.f && !obstaculo({p.x, 0, p.y}, false))
            arboles_.push_back({p.x, alturaSuelo(p.x, p.y), p.y});
    std::vector<Vertice> nube, luz;
    Geometria::elipsoide(nube, {0, 0, 0}, {1, .65f, 1}, {.74f, .78f, .84f}, 8, 4);
    Geometria::elipsoide(luz, {0, 0, 0}, {1, 1, 1}, {1, 1, 1}, 8, 5);
    humo_ = std::make_unique<Malla>(nube);
    propulsor_ = std::make_unique<Malla>(luz);
}

float SectorRatchet::sueloConPuerto(float x, float z) const {
    if (!suelo_)
        return 0.f;
    const float suelo = suelo_(x, z);
    const float r = glm::length(glm::vec2(x, z) - PatioRailjack);
    return glm::mix(pisoPuerto_, suelo, glm::smoothstep(12.f, RadioPuerto - 1.f, r));
}

float SectorRatchet::superficie(float x, float z) const {
    const float suelo = sueloConPuerto(x,z);
    const auto celda = (glm::vec2(x,z)-InicioPavimento)/PasoPavimento;
    if (distanciasPavimento_.empty() || celda.x < 0 || celda.y < 0 ||
        celda.x >= MuestrasPavimento-1 || celda.y >= MuestrasPavimento-1)
        return suelo;
    const int i = int(celda.x), j = int(celda.y), k = i*MuestrasPavimento+j;
    const float tx = celda.x-i, tz = celda.y-j;
    const auto &d = distanciasPavimento_;
    // La banqueta y el avatar consultan la misma diagonal de la malla.
    const float distancia = tx >= tz
        ? (1-tx)*d[k] + (tx-tz)*d[k+MuestrasPavimento] + tz*d[k+MuestrasPavimento+1]
        : (1-tz)*d[k] + (tz-tx)*d[k+1] + tx*d[k+MuestrasPavimento+1];
    return suelo + perfilBanqueta(distancia);
}

glm::vec3 SectorRatchet::centroPlaza() const {
    return {Plaza.x, superficie(Plaza.x, Plaza.y), Plaza.y};
}

glm::vec3 SectorRatchet::centroTorre() const {
    return {Torre.x, superficie(Torre.x, Torre.y), Torre.y};
}

void SectorRatchet::apoyarTorre() {
    const auto &vertices = orokintower_->triangulos();
    apoyoTorre_ = 1e9f;
    for (auto v : vertices)
        if (v.y < .08f)
            apoyoTorre_ = std::min(apoyoTorre_,
                suelo_(Torre.x + v.x * EscalaTorre, Torre.y + v.z * EscalaTorre) - v.y * EscalaTorre);
    apoyoTorre_ -= .14f;
    for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
        std::array<glm::vec3, 3> t;
        for (size_t j = 0; j < t.size(); ++j)
            t[j] = vertices[i+j] * EscalaTorre + glm::vec3(Torre.x, apoyoTorre_, Torre.y);
        const auto normal = glm::cross(t[1] - t[0], t[2] - t[0]);
        if (glm::length(normal) < .0001f)
            continue;
        std::vector<glm::vec3> poligono(t.begin(), t.end());
        // Recorta la malla a la franja que intersecta el cuerpo del avatar.
        for (int plano : {0, 1}) {
            auto anterior = std::move(poligono);
            poligono.clear();
            auto distancia = [&](glm::vec3 p) {
                float h = p.y - suelo_(p.x, p.z);
                return plano == 0 ? h - .12f : 2.7f - h;
            };
            for (size_t j = 0; j < anterior.size(); ++j) {
                const auto a = anterior[j], b = anterior[(j+1) % anterior.size()];
                const float da = distancia(a), db = distancia(b);
                if (da >= 0)
                    poligono.push_back(a);
                if ((da >= 0) != (db >= 0))
                    poligono.push_back(glm::mix(a, b, da / (da - db)));
            }
        }
        for (size_t j = 2; j < poligono.size(); ++j) {
            auto xz = [](auto p) { return glm::vec2(p.x, p.z); };
            huellaTorre_.push_back({xz(poligono[0]), xz(poligono[j-1]), xz(poligono[j])});
        }
    }
}

glm::mat4 SectorRatchet::matrizEstatua() const {
    const auto p = centroPlaza();
    auto m = glm::translate(glm::mat4(1), p + glm::vec3(0, 1.68f, 0));
    return glm::rotate(m, Pi, {0, 1, 0});
}

void SectorRatchet::construirUrbanismo() {
    struct Punto { glm::vec3 p; float borde; };
    constexpr int N = MuestrasPavimento;
    std::vector<Punto> grilla(N*N);
    distanciasPavimento_.resize(N*N);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j) {
            const auto p = InicioPavimento + PasoPavimento*glm::vec2(i,j);
            const float d = bordePavimento(p);
            grilla[i*N+j] = {{p.x, sueloConPuerto(p.x,p.y) + .085f, p.y}, d};
            distanciasPavimento_[i*N+j] = d;
        }
    std::vector<Vertice> piso, construcciones;
    using Poligono = std::array<Punto, 6>;
    const auto cortar = [](const Poligono &entrada, size_t n, float borde, bool mayor,
                           Poligono &salida) {
        size_t cantidad = 0;
        for (size_t i = 0; i < n; ++i) {
            const auto &a = entrada[i], &b = entrada[(i+1)%n];
            const bool dentroA = mayor ? a.borde >= borde : a.borde <= borde;
            const bool dentroB = mayor ? b.borde >= borde : b.borde <= borde;
            if (dentroA)
                salida[cantidad++] = a;
            if (dentroA != dentroB && a.borde != borde && b.borde != borde)
                salida[cantidad++] = {glm::mix(a.p,b.p,(borde-a.borde)/(b.borde-a.borde)),borde};
        }
        return cantidad;
    };
    auto emitir = [&](const Poligono &recorte, size_t cantidad, bool banqueta) {
        if (cantidad < 3)
            return;
        std::array<glm::vec3,6> puntos;
        for (size_t i = 0; i < cantidad; ++i)
            puntos[i] = recorte[i].p + glm::vec3(0,perfilBanqueta(recorte[i].borde),0);
        const auto p = puntos[0];
        const glm::vec2 xz(p.x, p.z);
        glm::vec3 color(.31f, .36f, .41f);
        if (glm::length(xz - Plaza) < 17.7f)
            color = {.45f, .44f, .43f};
        if (glm::length(xz - PatioRailjack) < 22.7f)
            color = {.27f, .33f, .38f};
        const int losa = int(std::floor((p.x + 200.f) / 3.6f)) + int(std::floor((p.z + 180.f) / 3.6f));
        if (banqueta)
            color = {.59f,.55f,.47f};
        color *= losa % 2 ? 1.f : .93f;
        for (size_t i = 2; i < cantidad; ++i) {
            auto n = glm::cross(puntos[i-1]-p,puntos[i]-p);
            if (glm::length(n) < .00001f)
                continue;
            n = glm::normalize(n);
            for (auto v : {p,puntos[i-1],puntos[i]}) {
                const auto q = glm::vec2(v.x,v.z);
                const float radio = glm::length(q - PatioRailjack);
                const float pintura = (!banqueta && radio>10.4f && radio<12.2f) ? .45f : 0.f;
                const auto tono = glm::mix(color, glm::vec3(.64f,.54f,.34f), pintura);
                piso.push_back({v,n,tono});
            }
        }
    };
    auto recortar = [&](const Poligono &t) {
        const float minimo = std::min({t[0].borde,t[1].borde,t[2].borde});
        const float maximo = std::max({t[0].borde,t[1].borde,t[2].borde});
        if (minimo > PieExterior)
            return;
        if (maximo <= PieBanqueta) {
            emitir(t,3,false);
            return;
        }
        Poligono intermedio, tramo;
        const auto dentro = cortar(t,3,PieBanqueta,false,tramo);
        emitir(tramo,dentro,false);
        const float bordes[] = {PieBanqueta,InicioBanqueta,FinBanqueta,PieExterior};
        for (size_t i = 0; i < 3; ++i) {
            if (maximo <= bordes[i] || minimo >= bordes[i+1])
                continue;
            const auto cantidad = cortar(t,3,bordes[i],true,intermedio);
            const auto n = cortar(intermedio,cantidad,bordes[i+1],false,tramo);
            emitir(tramo,n,true);
        }
    };
    for (int i = 0; i + 1 < N; ++i)
        for (int j = 0; j + 1 < N; ++j) {
            const int k = i*N+j;
            recortar({grilla[k], grilla[k+1], grilla[k+N+1]});
            recortar({grilla[k], grilla[k+N+1], grilla[k+N]});
        }
    pavimento_ = std::make_unique<Malla>(piso);
    const auto p = centroPlaza();
    float fondo = p.y;
    for (int i = 0; i < 24; ++i) {
        float a = i * 2 * Pi / 24;
        fondo = std::min(fondo, superficie(p.x + 4.3f*std::cos(a), p.z + 4.3f*std::sin(a)));
    }
    cilindro(construcciones, {p.x, fondo-.18f, p.z}, 4.3f, p.y+.65f-fondo,
             {.28f,.30f,.34f}, 8);
    cilindro(construcciones, p + glm::vec3(0,.44f,0), 3.65f, 1.0f, {.46f,.41f,.33f}, 8);
    cilindro(construcciones, p + glm::vec3(0,1.44f,0), 3.78f, .24f, {.67f,.49f,.24f}, 8);

    for (size_t i = 0; i < SitiosFocos.size(); ++i) {
        const auto q = SitiosFocos[i];
        glm::vec3 pie(q.x, superficie(q.x, q.y), q.y);
        Geometria::caja(construcciones, pie + glm::vec3(0,.38f,0), {2.6f,.85f,2.6f}, {.32f,.34f,.37f});
        Geometria::caja(construcciones, pie + glm::vec3(0,1.25f,0), {1.45f,1.30f,1.45f}, {.19f,.24f,.30f});
        cilindro(construcciones, pie + glm::vec3(0,1.9f,0), .83f, .22f, {.76f,.69f,.51f}, 10);
        reflectores_[i] = pie + glm::vec3(0,2.13f,0);
    }
    auto poste = [&](glm::vec2 q, float h, glm::vec3 color) {
        float y = superficie(q.x,q.y);
        Geometria::caja(construcciones, {q.x,y+h*.5f,q.y}, {.46f,h,.46f}, {.24f,.25f,.29f});
        Geometria::caja(construcciones, {q.x,y+h+.14f,q.y}, {.76f,.26f,.76f}, color);
    };
    for (auto q : {glm::vec2(-57,8),{-20,31},{-50,30},{-19,-1},{48,-24},{72,2},{-96,39},{-76,65}})
        poste(q, 3.4f, {.76f,.61f,.31f});
    for (float x : {-66.f, -54.f})
        poste({x, 107.f}, 6.f, {.36f,.70f,.86f});
    // La cerca tiene dos entradas alineadas con el patio y deja libre el camino de llegada.
    for (float z : {41.f, 66.f}) {
        for (int i = 0; i < 6; ++i) {
            const float x = -101.f + 5.5f*i;
            if (i == 2 || i == 3)
                continue;
            const float y = superficie(x,z);
            Geometria::caja(construcciones, {x,y+.57f,z}, {4.6f,.30f,.38f}, {.33f,.38f,.42f});
            poste({x-2,z}, 1.10f, {.56f,.48f,.33f});
        }
    }
    estructuras_ = std::make_unique<Malla>(construcciones);

    std::array<std::vector<Vertice>, 6> marcas;
    auto baliza = [&](glm::vec2 q, size_t color, float medida) {
        auto &m = marcas[color % marcas.size()];
        auto esquina = [&](float dx, float dz) {
            float x = q.x+dx, z = q.y+dz;
            return glm::vec3(x, superficie(x,z)+.128f, z);
        };
        const auto a = esquina(-medida,-medida), b = esquina(-medida,medida);
        const auto c = esquina(medida,medida), d = esquina(medida,-medida);
        Geometria::triangulo(m, a, b, c, {.65f,.48f,.23f});
        Geometria::triangulo(m, a, c, d, {.65f,.48f,.23f});
    };
    for (int i = 0; i < 96; ++i) {
        const double s = longitudPatrulla() * i / 96;
        auto q = esquinaPatrulla(s);
        auto t = frenteMuneco(s);
        baliza(q + glm::vec2(-t.z,t.x)*2.65f, i, .48f);
    }
    for (int i = 0; i < 24; ++i) {
        const float a = i * 2 * Pi / 24;
        const glm::vec2 radial(std::cos(a),std::sin(a));
        baliza(Plaza + 15.8f * radial, i, .68f);
        baliza(PatioRailjack + 18.9f * radial, 5, .43f);
        baliza(Torre + 16.3f * radial, 5, .39f);
    }
    for (const auto &calle : Calles) {
        const auto d = calle.b-calle.a;
        const float n = glm::length(d);
        const auto t = d/n;
        for (int i = 1; 8.f*i < n-4.f; ++i) {
            auto q = calle.a + 8.f*i*t + glm::vec2(-t.y,t.x)*(calle.medio-.85f);
            if (glm::length(q-Plaza) > 18.5f && glm::length(q-PatioRailjack) > RadioPuerto &&
                glm::length(q-Torre) > 19.f)
                baliza(q, i, .51f);
        }
    }
    std::vector<Vertice> marcasDia;
    for (size_t i = 0; i < balizas_.size(); ++i) {
        balizas_[i] = std::make_unique<Malla>(marcas[i]);
        marcasDia.insert(marcasDia.end(), marcas[i].begin(), marcas[i].end());
    }
    balizasDia_ = std::make_unique<Malla>(marcasDia);

    std::vector<Vertice> cono;
    for (int i = 0; i < 14; ++i) {
        const auto punto = [&](int j, float y) {
            const float a = j * 2 * Pi / 14;
            const glm::vec3 n(std::cos(a), -.14f, std::sin(a));
            const float radio = .065f + y * .142f;
            return Vertice{{n.x*radio,y,n.z*radio},glm::normalize(n),{1.f,1.f,1.f},{0,y}};
        };
        const auto a = punto(i,0), b = punto(i+1,0), c = punto(i+1,1), d = punto(i,1);
        cono.insert(cono.end(), {a,c,b,a,d,c});
    }
    haz_ = std::make_unique<Malla>(cono);
}

void SectorRatchet::actualizar(float dt) {
    tiempo_ = std::fmod(tiempo_ + std::max(0.f, dt), 9360.f);
    distanciaPatrulla_ = std::fmod(distanciaPatrulla_ + double(std::max(0.f, dt)) * VelocidadPatrulla,
                                 longitudPatrulla());
    const float a = aperturaRailjack(tiempo_);
    for (size_t i = 2; i <= 18 && i < partesRailjack_.size(); ++i) {
        const float signo = i % 2 ? -1.f : 1.f;
        partesRailjack_[i] = glm::translate(glm::mat4(1), {signo * 15.f * a, 5.f * a, 0});
    }
}
double SectorRatchet::longitudPatrulla() const { return 4 * CuartoPatrulla; }
glm::vec3 SectorRatchet::posicionMuneco(int i, double distancia) const {
    const auto q = esquinaPatrulla(distancia + i * longitudPatrulla()/CantidadMunecos);
    return {q.x, superficie(q.x, q.y) + .092f, q.y};
}
glm::vec3 SectorRatchet::frenteMuneco(double distancia, int i) const {
    distancia += i * longitudPatrulla()/CantidadMunecos;
    const auto d = glm::normalize(esquinaPatrulla(distancia+.20) - esquinaPatrulla(distancia-.20));
    return {d.x, 0, d.y};
}
void SectorRatchet::dibujarMuneco(Renderizador &r, const glm::mat4 &m, int i, double distancia,
                                  float tiempoBrazos) const {
    const auto p = posicionMuneco(i,distancia);
    const auto d = frenteMuneco(distancia,i);
    const float giro = std::atan2(d.x,d.z);
    const auto base = glm::rotate(glm::translate(m,p),giro,{0,1,0});
    const auto tipo = i % 2;
    r.brazosMuneco(std::sin(tiempoBrazos*(2*Pi/1.6f)+i*.85f), tipo == 1);
    munecos_[tipo]->dibujar(r,base);
    r.locomocionRobot(0,false);
}
glm::vec3 SectorRatchet::aphelionPosicion() const {
    return ruta(tiempo_);
}
glm::vec3 SectorRatchet::posicionAphelion(float t) const { return ruta(t); }
glm::vec3 SectorRatchet::frenteAphelion(float t) const {
    return glm::normalize(glm::vec3(matrizAphelion(t)[2]));
}
glm::vec3 SectorRatchet::railjackPosicion() const {
    return {PatioRailjack.x,
            pisoPuerto_ + ascensoRailjack(tiempo_) + 6.f,
            PatioRailjack.y};
}
float SectorRatchet::railjackAlturaBase() const {
    return apoyoRailjack_ + railjack_->minimoY() * EscalaRailjack + ascensoRailjack(tiempo_);
}
std::array<LuzFocal, 4> SectorRatchet::reflectoresRailjack(float fuerza) const {
    std::array<LuzFocal, 4> focos;
    for (size_t i = 0; i < focos.size(); ++i) {
        const auto q = reflectores_[i];
        const auto d = railjackPosicion() - q;
        auto &f = focos[i];
        f.posicion = q;
        f.direccion = glm::normalize(d + glm::vec3(d.x*.06f,2.0f,d.z*.06f));
        f.color = i % 2 ? glm::vec3(.72f,.83f,1.f) : glm::vec3(1.f,.81f,.57f);
        f.intensidad = fuerza * 9.0f;
        f.atenuacion = {1,.031f,.005f};
        f.interno = 7;
        f.externo = 15;
        f.encendida = fuerza > .001f;
    }
    return focos;
}
LuzFocal SectorRatchet::reflectorAphelion(float noche) const {
    LuzFocal f;
    const float fuerza = suave(.25f,.96f,noche);
    if (fuerza <= .001f)
        return f;
    const float fase = tiempo_*(2*Pi/12.f);
    f.posicion = aphelionPosicion() - glm::vec3(0,1.15f,0);
    f.direccion = glm::normalize(glm::vec3(.07f*std::cos(fase),-1,.07f*std::sin(fase)));
    f.color = glm::mix(glm::vec3(.76f,.88f,1.f),glm::vec3(1.f,.95f,.84f),.5f+.5f*std::sin(fase));
    f.atenuacion = {1.f,.012f,.0036f};
    const float altura = f.posicion.y-superficie(f.posicion.x,f.posicion.z);
    f.intensidad = .92f*fuerza*(1.f+.012f*altura+.0036f*altura*altura);
    f.interno = 7.5f;
    f.externo = 13.5f;
    f.encendida = true;
    return f;
}

void SectorRatchet::dibujarPavimento(Renderizador &r, const glm::mat4 &m, float noche) const {
    const glm::vec3 colores[] = {{1,.08f,.04f},{.09f,1,.18f},{.12f,.32f,1},
                                 {.05f,.9f,1},{.84f,.13f,1},{1,.69f,.05f}};
    if (noche < .001f)
        r.dibujar(*balizasDia_, m);
    else
        for (size_t i = 0; i < balizas_.size(); ++i)
            r.texturada(*balizas_[i], m, 0, glm::vec3(glm::mix(1.f,.13f,noche)),
                        colores[i] * .86f * noche);
    r.dibujar(*pavimento_, m);
}

void SectorRatchet::dibujar(ContextoEscena &c, const glm::mat4 &m, float noche,
                             bool reflectoresActivos) const {
    auto &r = c.renderizador;
    const ModeloGltf *tipos[] = {edificio1_.get(), edificio2_.get(), edificio3_.get()};
    for (size_t i = 0; i < Edificios.size(); ++i) {
        const auto e = Edificios[i];
        auto base = glm::translate(m, {e.posicion.x, apoyos_[i], e.posicion.y});
        base = glm::rotate(base, e.giro, {0, 1, 0});
        tipos[e.tipo]->dibujar(r, glm::scale(base, glm::vec3(EscalaEdificios)));
    }
    if (orokintower_) {
        orokintower_->dibujar(r, colocar(m, {Torre.x, apoyoTorre_, Torre.y}, EscalaTorre));
    }
    if (arbol_)
        for (const auto &p : arboles_)
            if (r.visible(glm::vec3(m * glm::vec4(p + glm::vec3(0,3,0),1)),4.f))
                r.dibujar(*arbol_, glm::translate(m, p));
    r.dibujar(*estructuras_, m);
    estatua_->dibujar(r, m * matrizEstatua());
    for (size_t i = 0; i < SitiosUtileria.size(); ++i) {
        const auto p = SitiosUtileria[i];
        auto &modelo = utileria_[TipoUtileria[i]];
        const auto base = glm::translate(m, {p.x, superficie(p.x,p.y) + .08f, p.y});
        modelo->dibujar(r, glm::rotate(base, float(i)*.44f, {0,1,0}));
    }

    const ModeloGltf *npc[] = {clank_.get(), blarg_.get(), warframe_.get()};
    for (int i = 0; i < 3; ++i)
        if (npc[i]) {
            r.locomocionRobot(tiempo_ * 3.5f, i == 0);
            if(i==1) r.locomocionBlarg(tiempo_*3.2f);
            if (i == 2)
                r.guardiaWarframe(.5f - .5f*std::cos(tiempo_ * (2 * Pi / 4.f)));
            npc[i]->dibujar(r, m * matrizNPC(i, tiempo_));
        }
    r.locomocionRobot(0, false);

    // Railjack: detenido en suelo, abre piezas, asciende, espera, baja y se cierra.
    if (railjack_) {
        auto base = glm::translate(
            m, {PatioRailjack.x, apoyoRailjack_ + ascensoRailjack(tiempo_), PatioRailjack.y});
        base = glm::rotate(base, -.18f, {0, 1, 0});
        base = glm::scale(base, glm::vec3(EscalaRailjack));
        railjack_->dibujar(r, base, 0, nullptr, &partesRailjack_);
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
    std::array<Renderizador::Volumen, 4*CantidadMunecos> vapor;
    size_t cantidadVapor = 0;
    for (int i = 0; i < CantidadMunecos; ++i) {
        const auto p = posicionMuneco(i, distanciaPatrulla_);
        if (!r.visible(glm::vec3(m * glm::vec4(p + glm::vec3(0,3,0),1)), 10.f))
            continue;
        dibujarMuneco(r,m,i,distanciaPatrulla_,tiempo_);
        for (int j = 0; j < 4; ++j) {
            float edad = std::fmod(tiempo_+j*.25f+i*.14f, 1.f);
            auto q = posicionMuneco(i, distanciaPatrulla_ - VelocidadPatrulla*edad);
            q.y += 4.0f + 2.9f*edad;
            q.x += .28f*std::sin(edad*Pi+i);
            vapor[cantidadVapor++] = {colocar(m,q,.30f+.80f*edad),
                                      {.79f,.88f,.96f}, .55f*glm::smoothstep(0.f,.12f,edad)*(1.f-edad)};
        }
    }
    r.volumenes(*humo_, vapor.data(), cantidadVapor, false);
    std::array<Renderizador::Volumen, 5> haces;
    size_t cantidadHaces = 0;
    if (reflectoresActivos && noche > .001f) {
        for (const auto &l : reflectoresRailjack(noche)) {
            const float alcance = glm::length(railjackPosicion() - l.posicion) + 7.f;
            haces[cantidadHaces++] = volumenHaz(m,l,alcance,.25f*noche);
        }
    }
    const auto focoVuelo = reflectorAphelion(noche);
    if (focoVuelo.encendida) {
        const auto p = focoVuelo.posicion;
        float alcance = (p.y-superficie(p.x,p.z))/-focoVuelo.direccion.y;
        const auto q = p + focoVuelo.direccion*alcance;
        alcance = (p.y-superficie(q.x,q.z))/-focoVuelo.direccion.y;
        auto centro = glm::vec3(m*glm::vec4(p+focoVuelo.direccion*(alcance*.5f),1));
        if (alcance > 0.f && r.visible(centro,alcance*.60f))
            haces[cantidadHaces++] = volumenHaz(m,focoVuelo,alcance,.21f*suave(.25f,.96f,noche));
    }
    r.volumenes(*haz_,haces.data(),cantidadHaces,true);
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
    estatua_.reset();
    for (auto &a : munecos_) a.reset();
    for (auto &a : utileria_) a.reset();
    for (auto &a : balizas_) a.reset();
    pavimento_.reset();
    estructuras_.reset();
    balizasDia_.reset();
    haz_.reset();
    huellaTorre_.clear();
    arboles_.clear();
    distanciasPavimento_.clear();
    partesRailjack_.clear();
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
        const auto q = glm::abs(rotarXZ(glm::vec2(p.x,p.z) - e.posicion, -e.giro));
        if (q.x < e.medio.x * EscalaEdificios + .72f && q.y < e.medio.y * EscalaEdificios + .72f)
            return true;
    }
    const glm::vec2 xz(p.x, p.z);
    if (glm::length(xz - PatioRailjack) < 12.f || glm::length(xz - Plaza) < 5.f)
        return true;
    if (glm::length(xz - Torre) < 16.f)
        for (const auto &t : huellaTorre_)
            if (tocaTriangulo(xz,t))
                return true;
    for (auto q : SitiosFocos)
        if (glm::all(glm::lessThan(glm::abs(xz-q),glm::vec2(1.95f))))
            return true;
    for (float z : {41.f,66.f})
        if (std::abs(p.z-z) < .94f)
            for (int i : {0,1,4,5})
                if (std::abs(p.x-(-101.f + 5.5f*i)) < 3.02f)
                    return true;
    for (size_t i = 0; i < SitiosUtileria.size(); ++i)
        if (TipoUtileria[i] != 1 && glm::length(xz-SitiosUtileria[i]) < (i == 0 ? 2.6f : 1.5f))
            return true;
    for (auto q : arboles_)
        if (glm::length(xz-glm::vec2(q.x,q.z)) < 1.f)
            return true;
    return false;
}

glm::vec3 SectorRatchet::posicionNPC(int i, float t) const {
    const glm::vec2 centros[] = {Plaza, {Corral.x-1.f, Corral.y}, {Corral.x+19.f, Corral.y+12.f}};
    const glm::vec2 radios[] = {{13, 13}, {10, 3}, {2.5f, 2.5f}};
    const float velocidad[] = {.17f, .15f, .13f};
    float a = t * velocidad[i] + i * 1.7f;
    if(i==1) a-=.75f*std::sin(a);
    glm::vec3 p(centros[i].x + radios[i].x * std::sin(a), 0,
                centros[i].y + radios[i].y * std::cos(a));
    p.y = superficie(p.x, p.z);
    return p;
}
glm::mat4 SectorRatchet::matrizNPC(int i, float t) const {
    auto p = posicionNPC(i, t);
    auto d = posicionNPC(i, t + .02f) - posicionNPC(i, t - .02f);
    float angulo = std::atan2(d.x, d.z) - (i == 1 ? Pi / 2 : 0);
    const float escala[] = {.27f, .48f, 4.f};
    const float apoyo = i == 2 ? -.04f - warframe_->minimoY()*escala[i] : .025f;
    auto m = glm::translate(glm::mat4(1), p + glm::vec3(0, apoyo, 0));
    m = glm::rotate(m, angulo, {0, 1, 0});
    return glm::scale(m, glm::vec3(escala[i]));
}
