#include "KeyframesCohete.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <cstdio>

namespace {
bool finita(glm::vec3 v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
const char *nombres[] = {"Saturn_Etapa_0", "Saturn_Etapa_1", "Saturn_Etapa_2", "Saturn_Capsula"};
} // namespace
bool KeyframesCohete::Leer(const std::string &ruta) {
    std::ifstream in(ruta);
    std::string etiqueta;
    if (!std::getline(in, etiqueta))
        return false;
    while (!etiqueta.empty() && etiqueta.back() == '\r')
        etiqueta.pop_back();
    if (etiqueta != "KEYFRAMES_COHETE_V2")
        return false;
    int n;
    double inicio, fin;
    if (!(in >> n >> inicio >> fin) || n != 4 || !std::isfinite(inicio) || !std::isfinite(fin) ||
        inicio <= 0 || fin <= inicio || fin > 3600)
        return false;
    std::array<PistaCohete, 4> nuevas;
    for (size_t i = 0; i < 4; i++) {
        auto &p = nuevas[i];
        int cantidad;
        if (!(in >> p.nombre >> p.pivote.x >> p.pivote.y >> p.pivote.z >> cantidad) ||
            p.nombre != nombres[i] || !finita(p.pivote) || cantidad < 2 || cantidad > 10000)
            return false;
        for (int k = 0; k < cantidad; k++) {
            CuadroClave c;
            float x, y, z, w;
            if (!(in >> c.tiempo >> c.posicion.x >> c.posicion.y >> c.posicion.z >> x >> y >> z >>
                  w >> c.velocidad.x >> c.velocidad.y >> c.velocidad.z >> c.estado))
                return false;
            c.orientacion = glm::quat(w, x, y, z);
            float largo = glm::length(c.orientacion);
            if (!std::isfinite(c.tiempo) || c.tiempo < 0 || c.tiempo > fin || !finita(c.posicion) ||
                !finita(c.velocidad) || !std::isfinite(largo) || std::abs(largo - 1) > 0.002f ||
                c.estado < 0 || c.estado > 3)
                return false;
            if (k == 0 ? c.tiempo != 0 : c.tiempo <= p.cuadros.back().tiempo)
                return false;
            c.orientacion = glm::normalize(c.orientacion);
            p.cuadros.push_back(c);
        }
        if (p.cuadros.back().tiempo != fin)
            return false;
        auto a = std::find_if(p.cuadros.begin(), p.cuadros.end(),
                              [&](const CuadroClave &c) { return c.tiempo == inicio; });
        const auto &b = p.cuadros.back();
        if (a == p.cuadros.end() || glm::length(a->posicion - b.posicion) > .001f ||
            glm::length(a->velocidad - b.velocidad) > .001f ||
            std::abs(glm::dot(a->orientacion, b.orientacion)) < .99999f || a->estado != b.estado)
            return false;
    }
    std::string resto;
    if (in >> resto)
        return false;
    pistas = std::move(nuevas);
    inicioOrbita = inicio;
    finOrbita = fin;
    cargado = true;
    Reiniciar();
    return true;
}
bool KeyframesCohete::Guardar(const std::string &ruta) const {
    if (!cargado)
        return false;
    std::ofstream out(ruta, std::ios::trunc);
    if (!out)
        return false;
    out << "KEYFRAMES_COHETE_V2\n4 " << std::fixed << std::setprecision(6) << inicioOrbita << ' '
        << finOrbita << '\n';
    for (const auto &p : pistas) {
        out << p.nombre << ' ' << p.pivote.x << ' ' << p.pivote.y << ' ' << p.pivote.z << ' '
            << p.cuadros.size() << '\n';
        for (const auto &c : p.cuadros)
            out << c.tiempo << ' ' << c.posicion.x << ' ' << c.posicion.y << ' ' << c.posicion.z
                << ' ' << c.orientacion.x << ' ' << c.orientacion.y << ' ' << c.orientacion.z << ' '
                << c.orientacion.w << ' ' << c.velocidad.x << ' ' << c.velocidad.y << ' '
                << c.velocidad.z << ' ' << c.estado << '\n';
    }
    out.flush();
    return out.good();
}
bool KeyframesCohete::Reproducir() {
    if (!cargado)
        return false;
    tiempo = 0;
    reproduciendo = true;
    return true;
}
void KeyframesCohete::Reiniciar() {
    tiempo = 0;
    reproduciendo = false;
}
void KeyframesCohete::Actualizar(float dt) {
    if (!reproduciendo || !std::isfinite(dt))
        return;
    tiempo += std::clamp(double(dt), 0.0, .08);
    if (tiempo >= finOrbita)
        tiempo = inicioOrbita + std::fmod(tiempo - inicioOrbita, finOrbita - inicioOrbita);
}
CuadroClave KeyframesCohete::Evaluar(size_t componente, double t) const {
    if (!cargado || componente >= 4)
        return {};
    const auto &v = pistas[componente].cuadros;
    if (t <= v.front().tiempo)
        return v.front();
    if (t >= v.back().tiempo)
        return v.back();
    auto it = std::upper_bound(v.begin(), v.end(), t,
                               [](double x, const CuadroClave &c) { return x < c.tiempo; });
    const auto &a = *(it - 1);
    const auto &b = *it;
    float h = float(b.tiempo - a.tiempo), u = float((t - a.tiempo) / h), u2 = u * u, u3 = u2 * u;
    CuadroClave c;
    c.tiempo = t;
    c.estado = a.estado;
    // Hermite: posiciones y tangentes provienen exclusivamente del TXT.
    c.posicion = (2 * u3 - 3 * u2 + 1) * a.posicion + (u3 - 2 * u2 + u) * h * a.velocidad +
                 (-2 * u3 + 3 * u2) * b.posicion + (u3 - u2) * h * b.velocidad;
    c.velocidad = ((6 * u2 - 6 * u) * a.posicion + (3 * u2 - 4 * u + 1) * h * a.velocidad +
                   (-6 * u2 + 6 * u) * b.posicion + (3 * u2 - 2 * u) * h * b.velocidad) /
                  h;
    glm::quat qb = b.orientacion;
    if (glm::dot(a.orientacion, qb) < 0)
        qb = -qb;
    c.orientacion = glm::normalize(glm::slerp(a.orientacion, qb, u));
    return c;
}
glm::mat4 KeyframesCohete::Matriz(size_t i, glm::vec3 origen) const {
    auto c = Actual(i);
    return glm::translate(glm::mat4(1), origen + c.posicion) * glm::mat4_cast(c.orientacion) *
           glm::translate(glm::mat4(1), -pistas.at(i).pivote);
}
size_t KeyframesCohete::Cantidad() const {
    size_t n = 0;
    for (const auto &p : pistas)
        n += p.cuadros.size();
    return n;
}
void KeyframesCohete::Listar() const {
    for (const auto &p : pistas) {
        printf("%s: %zu cuadros\n", p.nombre.c_str(), p.cuadros.size());
        for (const auto &c : p.cuadros)
            printf("  t=%6.2f pos=(%7.2f,%7.2f,%7.2f) estado=%d\n", c.tiempo, c.posicion.x,
                   c.posicion.y, c.posicion.z, c.estado);
    }
}
