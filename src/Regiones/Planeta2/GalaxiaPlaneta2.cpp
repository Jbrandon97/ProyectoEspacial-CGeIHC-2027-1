#include "GalaxiaPlaneta2.h"
#include "Graficos/Renderizador.h"
#include "cgltf.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

namespace {
constexpr double VentanaG = 1.6;
constexpr float EntradaCamara = 2.4f, EntradaGalaxia = 4.6f, Salida = 1.2f;
constexpr glm::vec3 Centro{-155, 161, -266};
constexpr float Radio = 166.f;

std::vector<Vertice> cargarPuntos(const std::filesystem::path &ruta) {
    std::ifstream archivo(ruta, std::ios::binary | std::ios::ate);
    if (!archivo || archivo.tellg() <= 0)
        throw std::runtime_error("No se pudo abrir la galaxia de Planeta 2");
    std::vector<char> bytes(static_cast<size_t>(archivo.tellg()));
    archivo.seekg(0);
    if (!archivo.read(bytes.data(), static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("No se pudo leer el GLB de la galaxia");
    cgltf_options opciones{};
    cgltf_data *raw = nullptr;
    if (cgltf_parse(&opciones, bytes.data(), bytes.size(), &raw) != cgltf_result_success)
        throw std::runtime_error("GLB de la galaxia invalido");
    std::unique_ptr<cgltf_data, decltype(&cgltf_free)> datos(raw, cgltf_free);
    if (cgltf_load_buffers(&opciones, raw, nullptr) != cgltf_result_success ||
        cgltf_validate(raw) != cgltf_result_success)
        throw std::runtime_error("Datos de la galaxia incompletos");

    std::vector<Vertice> puntos;
    glm::dvec3 suma(0.0);
    for (size_t i = 0; i < raw->nodes_count; ++i) {
        const auto &nodo = raw->nodes[i];
        if (!nodo.mesh)
            continue;
        glm::mat4 matriz(1);
        cgltf_node_transform_world(&nodo, glm::value_ptr(matriz));
        for (size_t j = 0; j < nodo.mesh->primitives_count; ++j) {
            const auto &prim = nodo.mesh->primitives[j];
            if (prim.type != cgltf_primitive_type_points)
                continue;
            const auto *pos = cgltf_find_accessor(&prim, cgltf_attribute_type_position, 0);
            const auto *color = cgltf_find_accessor(&prim, cgltf_attribute_type_color, 0);
            if (!pos || !color || pos->count != color->count)
                throw std::runtime_error("Faltan posiciones o colores de la galaxia");
            size_t cantidad = prim.indices ? prim.indices->count : pos->count;
            puntos.reserve(puntos.size() + cantidad);
            for (size_t k = 0; k < cantidad; ++k) {
                const auto indice = prim.indices ? cgltf_accessor_read_index(prim.indices, k) : k;
                glm::vec3 local(0);
                glm::vec4 rgba(1);
                if (!cgltf_accessor_read_float(pos, indice, glm::value_ptr(local), 3) ||
                    !cgltf_accessor_read_float(color, indice, glm::value_ptr(rgba), 4))
                    throw std::runtime_error("Punto invalido en la galaxia");
                auto p = glm::vec3(matriz * glm::vec4(local, 1));
                suma += glm::dvec3(p);
                puntos.push_back({p, {}, glm::vec3(rgba)});
            }
        }
    }
    if (puntos.empty())
        throw std::runtime_error("El GLB no contiene puntos");
    const auto centro = glm::vec3(suma / static_cast<double>(puntos.size()));
    float radio = 0;
    for (auto &p : puntos) {
        p.posicion -= centro;
        radio = std::max(radio, glm::length(p.posicion));
    }
    if (!(radio > 0))
        throw std::runtime_error("Nube de puntos sin extension");
    for (auto &p : puntos)
        p.posicion /= radio;
    return puntos;
}
} // namespace

void GalaxiaPlaneta2::cargar(const std::filesystem::path &recursos) {
    const auto puntos = cargarPuntos(recursos / "Modelos/Planeta2Galaxia/need_some_space.glb");
    shader_ = std::make_unique<Shader>(recursos / "Shaders/galaxia.vert",
                                       recursos / "Shaders/galaxia.frag");
    nube_ = std::make_unique<Malla>(puntos);
    puntos_ = puntos.size();
}

void GalaxiaPlaneta2::liberar() noexcept {
    reiniciar();
    nube_.reset();
    shader_.reset();
    puntos_ = 0;
}

bool GalaxiaPlaneta2::pulsarG(double ahora) {
    if (pulsaciones_ == 0 || ahora - primeraPulsacion_ > VentanaG) {
        primeraPulsacion_ = ahora;
        pulsaciones_ = 0;
    }
    if (++pulsaciones_ != 3)
        return false;
    pulsaciones_ = 0;
    activa_ = !activa_;
    return true;
}

void GalaxiaPlaneta2::cerrar() {
    interrumpir();
    activa_ = false;
}

void GalaxiaPlaneta2::reiniciar() {
    cerrar();
    camara_ = aparicion_ = giro_ = 0;
}

void GalaxiaPlaneta2::actualizar(float dt) {
    if (!enCurso())
        return;
    camara_ = std::clamp(camara_ + dt / (activa_ ? EntradaCamara : -Salida), 0.f, 1.f);
    aparicion_ = std::clamp(aparicion_ + dt / (activa_ ? EntradaGalaxia : -Salida), 0.f, 1.f);
    giro_ = enCurso() ? std::fmod(giro_ + dt, 120.f) : 0.f;
}

float GalaxiaPlaneta2::mezclaCamara() const { return glm::smoothstep(0.f, 1.f, camara_); }
float GalaxiaPlaneta2::presencia() const { return glm::smoothstep(.025f, 1.f, aparicion_); }

void GalaxiaPlaneta2::dibujar(Renderizador &r, const glm::mat4 &mundo, float noche) const {
    const float intensidad = presencia();
    if (!nube_ || intensidad <= 0.f)
        return;
    const auto frente = glm::normalize(Ojo - Centro);
    const auto derecha = glm::normalize(glm::cross(glm::vec3(0, 1, 0), frente));
    const glm::mat4 base(glm::vec4(derecha, 0), glm::vec4(glm::cross(frente, derecha), 0),
                         glm::vec4(frente, 0), glm::vec4(0, 0, 0, 1));
    auto modelo = glm::translate(mundo, Centro) * base;
    modelo = glm::rotate(modelo, glm::radians(-21.f), {1, 0, 0});
    modelo = glm::rotate(modelo, giro_ * glm::radians(3.f), {0, 0, 1});
    modelo = glm::rotate(modelo, glm::radians(90.f), {1, 0, 0});
    modelo = glm::scale(modelo, glm::vec3(Radio * glm::mix(.22f, 1.f, intensidad)));
    r.puntos(*nube_, *shader_, modelo, {intensidad * glm::mix(.83f, 1.f, noche), intensidad, giro_});
}
