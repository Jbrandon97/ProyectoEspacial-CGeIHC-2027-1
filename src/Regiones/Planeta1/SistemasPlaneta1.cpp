#include "SistemasPlaneta1.h"
#include "TerrenoPlaneta1.h"
#include "CircuitoCarreras.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>
void SistemasPlaneta1::configurar(const std::filesystem::path &p) {
    if (!animacionCohete_.Leer((p.parent_path() / "Animaciones/keyframescohete.txt").string()) ||
        animacionCohete_.Cantidad() != 72)
        throw std::runtime_error("Saturn V: no se pudieron cargar los 72 keyframes definitivos");
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
    const float avance = std::max(0.f, dt);
    const bool pausados[] = {reaccion >= 0, inspeccion >= 0, false, hunter >= 0};
    for (int i = 0; i < 4; ++i)
        if (!pausados[i])
            relojesPatrulla_[i] += avance;
    tiempo += dt;
    if (holocron >= 0 && (holocron += dt) >= 12) holocron = -1;
    if (inspeccion >= 0 && (inspeccion += dt) >= 8)
        inspeccion = -1;
    if (saludo >= 0 && (saludo += dt) >= 6)
        saludo = -1;
    if (titan >= 0 && (titan += dt) >= 14)
        titan = -1;
    if (hunter >= 0 && (hunter += dt) >= 8)
        hunter = -1;
    float avanceReposicion = dt;
    if (lanzamiento >= 0) {
        float previo = lanzamiento;
        lanzamiento += dt;
        if (lanzamiento >= 70) {
            reposicionPendiente = false;
            reiniciar();
            avanceReposicion = std::max(0.f, previo + dt - 70.f);
        }
    }
    if (reposicion >= 0) {
        reposicion += avanceReposicion;
        if (reposicion >= 8.5f)
            reposicion = -1;
    }
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
namespace {
constexpr float DosPi = 6.28318530718f;
constexpr glm::vec2 InicioPatrulla[] = {{-77, 153}, {-65, 162}, {-53, 153}, {-65, 174}};
constexpr glm::vec2 RadioPatrulla[] = {{6, 5}, {6, 5}, {6, 5}, {4.5f, 5}};
constexpr float FasePatrulla[] = {0, 3.14159265359f, 0, 1.57079632679f};
constexpr float PeriodoPatrulla[] = {22, 25, 28, 30};
float faseNPC(int npc, float reloj) {
    return FasePatrulla[npc] + DosPi * reloj / PeriodoPatrulla[npc];
}
}
glm::vec3 SistemasPlaneta1::posicionPatrulla(int npc) const {
    if (npc < 0 || npc >= 4)
        return {};
    const auto inicio = InicioPatrulla[npc];
    const auto radio = RadioPatrulla[npc];
    const float fase = faseNPC(npc, relojesPatrulla_[npc]);
    const float inicial = FasePatrulla[npc];
    const float x = inicio.x + radio.x * (std::sin(fase) - std::sin(inicial));
    // Hunter recorre el arco superior del patio; mantiene su volumen separado del Grunt 2.
    const float sentidoZ = npc == 3 ? -1.f : 1.f;
    const float z = inicio.y + sentidoZ * radio.y * (std::cos(fase) - std::cos(inicial));
    return {x, TerrenoPlaneta1::altura(x, z), z};
}
float SistemasPlaneta1::orientacionPatrulla(int npc) const {
    if (npc < 0 || npc >= 4)
        return 0;
    const float fase = faseNPC(npc, relojesPatrulla_[npc]);
    const float velocidadAngular = DosPi / PeriodoPatrulla[npc];
    const float dx = RadioPatrulla[npc].x * std::cos(fase) * velocidadAngular;
    const float sentidoZ = npc == 3 ? 1.f : -1.f;
    const float dz = sentidoZ * RadioPatrulla[npc].y * std::sin(fase) * velocidadAngular;
    return std::atan2(dx, dz);
}
float SistemasPlaneta1::relojPatrulla(int npc) const {
    return npc < 0 || npc >= 4 ? 0.f : relojesPatrulla_[npc];
}
float SistemasPlaneta1::periodoPatrulla(int npc) const {
    return npc < 0 || npc >= 4 ? 0.f : PeriodoPatrulla[npc];
}
void SistemasPlaneta1::reiniciarPatrullas() {
    relojesPatrulla_.fill(0.f);
}
Iluminacion SistemasPlaneta1::luces() const {
    Iluminacion l;
    l.activa = true;
    float a = faseSolar() * 6.2831853f;
    l.sol = glm::normalize(glm::vec3(std::cos(a), std::sin(a), .2f * std::cos(a)));
    float dia = glm::smoothstep(-.15f, .2f, l.sol.y);
    l.noche = 1 - dia;
    l.fuerzaSol = glm::smoothstep(-.04f, .3f, l.sol.y) * .95f;
    l.ambiente = .18f + .085f * dia;
    l.colorSol = glm::mix(glm::vec3(1, .48f, .23f), glm::vec3(1, .98f, .9f),
                          glm::smoothstep(0.f, .55f, l.sol.y));
    const glm::vec3 pos[] = {{165, 11, -72}, {40, 9, -95},  {90, 9, -66},
                             {85, 9, -122},  {205, 11, -7}, {-145, 12, -63}};
    for (int i = 0; i < 6; ++i) {
        auto &p = l.puntuales[i];
        p.posicion = pos[i];
        p.color = i == 5   ? glm::vec3(.7f, .18f, 1)
                  : i == 4 ? glm::vec3(.4f, .85f, 1)
                           : glm::vec3(1, .68f, .38f);
        p.intensidad = (i == 5 ? 7.f : 5.f) * l.noche;
        p.encendida = l.noche > .001f;
    }
    // Reutilizar las dos plazas libres: luz corta solidaria a cada patrulla.
    for (int i = 0; i < 2; ++i) {
        auto &p = l.puntuales[6 + i];
        p.posicion = sentinel(i) + glm::vec3(0, -.3f, 0);
        p.color = {.35f, .65f, 1.f};
        p.intensidad = 6.5f * l.noche;
        p.atenuacion = {1.f, .07f, .025f};
        p.encendida = l.noche > .001f;
    }
    for (int i = 0; i < 4; ++i) {
        auto q = CircuitoCarreras::muestra(CircuitoCarreras::Longitud * (i + .125f) / 4);
        auto &p = l.puntuales[9 + i];
        p.posicion = CircuitoCarreras::origen() + q.posicion + glm::vec3(0, 7, 0);
        p.color = {.3f, .65f, 1};
        p.intensidad = 4 * l.noche;
        p.atenuacion = {1, .045f, .008f};
        p.encendida = l.noche > .001f;
    }
    // Dos luminarias de paso; mantienen el contraste nocturno fuera del camino.
    const glm::vec3 pasos[] = {{132, 7, -70}, {172, 7, -10}};
    for (int i=0;i<2;++i) {
        auto &p=l.puntuales[13+i]; p.posicion=pasos[i]; p.color={1,.75f,.48f};
        p.intensidad=3.2f*l.noche; p.atenuacion={1,.045f,.012f}; p.encendida=l.noche>.001f;
    }
    auto &f=l.focales[0]; f.encendida=reflectorCovenant;
    f.posicion={-130,13,-48}; f.direccion=glm::normalize(glm::vec3(-15,-10,-15));
    f.color={.55f,.4f,1}; f.intensidad=2.6f; f.interno=20; f.externo=34;
    f.atenuacion={1,.025f,.0025f};
    l.foco = reflector;
    l.focoPos = {165, 12, -76};
    l.focoDir = glm::normalize(glm::vec3(0, -.65f, 1));
    return l;
}
namespace {
float suave(float a, float b, float t) {
    return glm::smoothstep(a, b, t);
}
const glm::vec3 origenSaturn(205, 5.22f, 21);
const float separacion[] = {7, 11, 15};
const float contacto[] = {17, 21, 24};
glm::mat4 matrizClave(const KeyframesCohete &a, int i, double t) {
    auto c = a.Evaluar(i, t);
    return glm::translate(glm::mat4(1), c.posicion) * glm::mat4_cast(c.orientacion) *
           glm::translate(glm::mat4(1), -a.Pista(i).pivote);
}
} // namespace
void SistemasPlaneta1::reiniciar() {
    if (reposicion >= 0)
        return;
    if (lanzamiento >= 0 && lanzamiento < 70) {
        reposicionPendiente = true;
        return;
    }
    retirarPreparado = lanzamiento < 0;
    animacionCohete_.Reiniciar();
    lanzamiento = -1;
    reposicion = 0;
}
float SistemasPlaneta1::aperturaCompuerta() const {
    if (reposicion < 0)
        return 0;
    return suave(0, 1, reposicion) * (1 - suave(7, 8.5f, reposicion));
}
glm::vec3 SistemasPlaneta1::cohete() const {
    auto p = origenSaturn;
    if (reposicion >= 0) {
        float bajada = retirarPreparado ? suave(1, 3.5f, reposicion) : 1;
        p.y -= 54 * bajada * (1 - suave(3.5f, 7, reposicion));
    }
    return p; // Raiz constante durante el vuelo. El TXT contiene todo el ascenso.
}
void SistemasPlaneta1::calibrarApoyo(const std::array<std::vector<glm::vec3>, 4> &v,
                                     const std::function<float(float, float)> &altura) {
    for (int i = 0; i < 3; ++i) {
        if (v[i].empty())
            throw std::runtime_error("Saturn V: etapa sin geometria");
        auto m = matrizClave(animacionCohete_, i, 24);
        float ajuste = -10000;
        for (auto q : v[i]) {
            q = origenSaturn + glm::vec3(m * glm::vec4(q, 1));
            ajuste = std::max(ajuste, altura(q.x, q.z) + .03f - q.y);
        }
        ajusteSuelo_[i] = ajuste;
    }
    if (v[3].empty())
        throw std::runtime_error("Saturn V: falta capsula independiente");
}
std::array<glm::mat4, 4> SistemasPlaneta1::etapas() const {
    std::array<glm::mat4, 4> m;
    for (int i = 0; i < 4; ++i) {
        m[i] = matrizClave(animacionCohete_, i, std::max(0.f, lanzamiento));
        if (i < 3 && lanzamiento >= separacion[i]) {
            // Ajuste vertical del apoyo; conserva los keyframes y la capsula.
            // Alcanzar el ajuste antes del contacto evita hundir piezas en la ladera.
            float u = suave(separacion[i], contacto[i] - 2, lanzamiento);
            m[i] = glm::translate(glm::mat4(1), {0, ajusteSuelo_[i] * u, 0}) * m[i];
        }
    }
    // Variante del proyecto: S-IVB y capsula permanecen unidas tras la segunda separacion.
    if (lanzamiento >= 11) {
        float u = lanzamiento - 11;
        auto c = animacionCohete_.Evaluar(2, 11);
        auto pos = c.posicion + c.velocidad * u + glm::vec3(.08f * u * u, .5f * u * u, 0);
        auto giro = glm::angleAxis(-.28f * glm::smoothstep(0.f, 30.f, u), glm::vec3(0, 0, 1));
        auto vuelo = glm::translate(glm::mat4(1), pos) * glm::mat4_cast(giro) *
                     glm::translate(glm::mat4(1), -animacionCohete_.Pista(2).pivote);
        m[2] = m[3] = vuelo;
    }
    // Los restos permanecen apoyados hasta reiniciar el ciclo de lanzamiento.
    return m;
}
glm::vec3 SistemasPlaneta1::fuenteMotor() const {
    int i = lanzamiento < 7 ? 0 : lanzamiento < 11 ? 1 : 2;
    float base = i == 0 ? 0.f : i == 1 ? 17.949953f : 31.532417f;
    return cohete() + glm::vec3(etapas()[i] * glm::vec4(0, base, 0, 1));
}
glm::vec3 SistemasPlaneta1::objetivoObservacion() const {
    float t = std::min(13.f, std::max(0.f, lanzamiento));
    return origenSaturn + animacionCohete_.Evaluar(3, t).posicion - glm::vec3(0, 22, 0);
}

glm::vec3 SistemasPlaneta1::sentinel(int i) const {
    glm::vec2 c = i == 0 ? glm::vec2(-110, -45) : glm::vec2(123, -38);
    float a = tiempo * (i == 0 ? .22f : -.18f) + i * 2;
    float x = c.x + 12 * std::cos(a), z = c.y + 8 * std::sin(a);
    return {x, TerrenoPlaneta1::altura(x, z) + 9 + .4f * std::sin(tiempo * 1.3f + i), z};
}
float SistemasPlaneta1::giroSentinel(int i) const {
    float v = i == 0 ? .22f : -.18f, a = tiempo * v + i * 2;
    // El frente de la malla original es +X, comprobado desde vistas ortogonales.
    return std::atan2(-12 * std::sin(a) * v, 8 * std::cos(a) * v) - 1.570796327f;
}
glm::mat4 SistemasPlaneta1::animacionNPC(int i) const {
    glm::mat4 m(1);
    if (i == 0 && reaccion >= 0) {
        float u = reaccion / 5;
        float caminar = std::sin(u * 3.14159265f);
        m = glm::translate(m, {0, 0, 2.8f * caminar});
        return glm::rotate(m, .4f * std::sin(u * 6.2831853f), {0, 1, 0});
    }
    if (i == 1 || i == 3) {
        // Dos objetos distintos: Grunt 2 inspecciona; Hunter gira en vigilancia.
        const float t[] = {0, 2, 4, 6, 8};
        const float giro[] = {0, .5f, -.35f, .25f, 0};
        const float incl[] = {0, .12f, .05f, -.07f, 0};
        float u = i == 3 ? std::max(0.f, hunter) : std::max(0.f, inspeccion);
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

glm::mat4 SistemasPlaneta1::matrizHolocron() const {
    struct Clave { float t, altura, escala, giro; };
    const Clave claves[]={{0,0,1,0},{2,4,1.8f,.8f},{5,8,3.5f,3.14f},
                         {8,8,3.5f,9.42f},{10,4,1.8f,11.7f},{12,0,1,12.5663706f}};
    float t=std::max(0.f,holocron); int i=0;
    while(i<4 && t>claves[i+1].t) ++i;
    auto a=claves[i], b=claves[i+1];
    float u=glm::smoothstep(a.t,b.t,t);
    auto m=glm::translate(glm::mat4(1),{-125,4.4f+glm::mix(a.altura,b.altura,u),-48});
    m=glm::rotate(m,glm::mix(a.giro,b.giro,u),glm::vec3(0,1,0));
    return glm::scale(m,glm::vec3(1.5f*glm::mix(a.escala,b.escala,u)));
}
