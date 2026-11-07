#include "Titan.h"
#include "Graficos/Renderizador.h"
#include "cgltf.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <iostream>
namespace {
// Estados temporales de un paso: apoyo, impulso, vuelo y nuevo apoyo.
float clave(float u, const std::array<float, 5> &v) {
    u = std::fmod(u, 1.f);
    int k = std::min(3, int(u * 4));
    float a = glm::smoothstep(0.f, 1.f, u * 4 - k);
    return glm::mix(v[k], v[k + 1], a);
}
} // namespace
Titan::Titan(const std::filesystem::path &ruta) {
    cgltf_options op{};
    cgltf_data *raw = nullptr;
    if (cgltf_parse_file(&op, ruta.string().c_str(), &raw) != cgltf_result_success)
        throw std::runtime_error("BT glTF invalido");
    std::unique_ptr<cgltf_data, decltype(&cgltf_free)> datos(raw, cgltf_free);
    if (cgltf_load_buffers(&op, raw, ruta.string().c_str()) != cgltf_result_success ||
        cgltf_validate(raw) != cgltf_result_success || raw->skins_count != 1)
        throw std::runtime_error("BT rig invalido");
    nodos_.resize(raw->nodes_count);
    for (size_t i = 0; i < nodos_.size(); ++i) {
        auto &n = nodos_[i];
        auto &r = raw->nodes[i];
        n.nombre = r.name ? r.name : "";
        n.padre = r.parent ? int(r.parent - raw->nodes) : -1;
        if (n.padre >= int(i))
            throw std::runtime_error("BT jerarquia fuera de orden");
        cgltf_node_transform_local(&r, glm::value_ptr(n.local));
        cgltf_node_transform_world(&r, glm::value_ptr(n.global));
        n.reposo = n.global;
    }
    auto &skin = raw->skins[0];
    for (size_t j = 0; j < skin.joints_count; ++j) {
        juntas_.push_back(int(skin.joints[j] - raw->nodes));
        glm::mat4 m;
        if (!cgltf_accessor_read_float(skin.inverse_bind_matrices, j, glm::value_ptr(m), 16))
            throw std::runtime_error("BT bind invalido");
        inversas_.push_back(m);
    }
    std::map<std::string, std::shared_ptr<Textura>> texturas;
    for (size_t ni = 0; ni < raw->nodes_count; ++ni) {
        auto &n = raw->nodes[ni];
        if (!n.skin || !n.mesh)
            continue;
        for (size_t pi = 0; pi < n.mesh->primitives_count; ++pi) {
            auto &p = n.mesh->primitives[pi];
            Piel s;
            const cgltf_accessor *pos = nullptr, *nor = nullptr, *uv = nullptr, *jo = nullptr,
                                 *we = nullptr;
            for (size_t j = 0; j < p.attributes_count; ++j) {
                auto &a = p.attributes[j];
                if (a.type == cgltf_attribute_type_position)
                    pos = a.data;
                if (a.type == cgltf_attribute_type_normal)
                    nor = a.data;
                if (a.type == cgltf_attribute_type_texcoord && a.index == 0)
                    uv = a.data;
                if (a.type == cgltf_attribute_type_joints)
                    jo = a.data;
                if (a.type == cgltf_attribute_type_weights)
                    we = a.data;
            }
            if (!pos || !nor || !uv || !jo || !we)
                throw std::runtime_error("BT piel incompleta");
            for (size_t j = 0; j < pos->count; ++j) {
                Vertice v{};
                glm::vec4 ids, w;
                cgltf_accessor_read_float(pos, j, glm::value_ptr(v.posicion), 3);
                cgltf_accessor_read_float(nor, j, glm::value_ptr(v.normal), 3);
                cgltf_accessor_read_float(uv, j, glm::value_ptr(v.uv), 2);
                cgltf_accessor_read_float(jo, j, glm::value_ptr(ids), 4);
                cgltf_accessor_read_float(we, j, glm::value_ptr(w), 4);
                float suma = w.x + w.y + w.z + w.w;
                if (suma <= 0)
                    throw std::runtime_error("BT peso nulo");
                w /= suma;
                for (int k = 0; k < 4; ++k)
                    if (ids[k] < 0 || ids[k] >= juntas_.size())
                        throw std::runtime_error("BT junta invalida");
                s.base.push_back(v);
                s.juntas.push_back(glm::ivec4(ids));
                s.pesos.push_back(w);
            }
            size_t count = p.indices ? p.indices->count : pos->count;
            for (size_t j = 0; j < count; ++j)
                s.indices.push_back(
                    unsigned(p.indices ? cgltf_accessor_read_index(p.indices, j) : j));
            auto &mat = p.material->pbr_metallic_roughness;
            s.color = glm::make_vec3(mat.base_color_factor);
            if (mat.base_color_texture.texture) {
                std::string nombre = mat.base_color_texture.texture->image->uri;
                if (!texturas.count(nombre))
                    texturas[nombre] = std::make_shared<Textura>(ruta.parent_path() / nombre);
                s.textura = texturas[nombre];
            }
            s.deformada = s.base;
            s.dibujo.resize(count);
            s.malla = std::make_unique<Malla>(s.dibujo);
            pieles_.push_back(std::move(s));
        }
    }
    animar(-1);
    float minY = 1e9f, maxY = -1e9f;
    for (auto &s : pieles_)
        for (auto &v : s.deformada) {
            minY = std::min(minY, v.posicion.y);
            maxY = std::max(maxY, v.posicion.y);
        }
    escala_ = 8.5f / (maxY - minY);
    ultimo_ = -100;
    animar(-1);
    std::cout << "BT_RIG_OK juntas=" << juntas_.size() << " pieles=" << pieles_.size()
              << " altura=8.5\n";
}
void Titan::animar(float t, float saludo) {
    if (t == ultimo_ && saludo == ultimoSaludo_)
        return;
    ultimo_ = t;
    ultimoSaludo_ = saludo;
    bool andando = (t >= .6f && t < 5.6f) || (t >= 7.f && t < 12.f);
    float tramo = t < 7 ? t - .6f : t - 7.f;
    float fase = std::max(0.f, tramo) / 1.25f;
    float amplitud =
        andando ? glm::smoothstep(0.f, .35f, tramo) * (1 - glm::smoothstep(4.65f, 5.f, tramo)) : 0;
    // Dos segmentos por pierna: mismos objetivos de apoyo, fases opuestas y paso ligado al avance.
    // Corregimos la asimetria de la pose original antes de aplicar el ciclo.
    std::array<glm::vec3, 2> correcciones{};
    auto sitioReposo = [&](const std::string &nombre) {
        for (auto &n : nodos_)
            if (n.nombre == nombre)
                return glm::vec3(n.reposo[3]) * escala_;
        throw std::runtime_error("BT rig de pierna ausente");
    };
    for (int lado = 0; lado < 2; ++lado) {
        auto hip = sitioReposo(lado ? "def_r_thigh_0134" : "def_l_thigh_0119");
        auto rod = sitioReposo(lado ? "def_r_knee_0136" : "def_l_knee_0121");
        auto tob = sitioReposo(lado ? "def_r_ankle_0137" : "def_l_ankle_0122");
        float l1 = glm::length(glm::vec2(rod.y - hip.y, rod.z - hip.z));
        float l2 = glm::length(glm::vec2(tob.y - rod.y, tob.z - rod.z));
        float u = std::fmod(fase + lado * .5f, 1.f), z, levantamiento = 0;
        if (u < .5f)
            z = .375f - 1.5f * u;
        else {
            float v = (u - .5f) * 2;
            z = glm::mix(-.375f, .375f, glm::smoothstep(0.f, 1.f, v));
            levantamiento = .32f * std::sin(v * 3.14159265f);
        }
        float referenciaHip =
            std::min(sitioReposo("def_l_thigh_0119").y, sitioReposo("def_r_thigh_0134").y);
        float y = referenciaHip - hip.y - .92f * (l1 + l2) + levantamiento;
        float d = std::min(std::sqrt(y * y + z * z), l1 + l2 - .001f);
        float apertura =
            std::acos(glm::clamp((l1 * l1 + d * d - l2 * l2) / (2 * l1 * d), -1.f, 1.f));
        float direccion = std::atan2(z, -y) + apertura;
        float ky = -l1 * std::cos(direccion), kz = l1 * std::sin(direccion);
        auto envolver = [](float x) { return std::atan2(std::sin(x), std::cos(x)); };
        float a = envolver(std::atan2(kz, ky) - std::atan2(rod.z - hip.z, rod.y - hip.y));
        float b =
            envolver(std::atan2(z - kz, y - ky) - std::atan2(tob.z - rod.z, tob.y - rod.y) - a);
        correcciones[lado] = {a, b, -a - b};
    }
    for (size_t i = 0; i < nodos_.size(); ++i) {
        auto &n = nodos_[i];
        auto local = n.local;
        float ang = 0;
        auto &nombre = n.nombre;
        bool izq = nombre.find("def_l_") == 0, der = nombre.find("def_r_") == 0;
        float u = fase + (der ? .5f : 0);
        if (izq || der) {
            if (nombre.find("_thigh_") != std::string::npos)
                ang = correcciones[der ? 1 : 0].x;
            if (nombre.find("_knee_") != std::string::npos)
                ang = correcciones[der ? 1 : 0].y;
            if (nombre.find("_ankle_") != std::string::npos)
                ang = correcciones[der ? 1 : 0].z;
            if (nombre.find("_shoulder_") != std::string::npos)
                ang = clave(u, {.24f, 0, -.24f, 0, .24f});
            if (nombre.find("_elbow_") != std::string::npos)
                ang = clave(u, {0, .16f, .24f, .1f, 0});
        }
        float angSaludo = 0, lateral = 0;
        if (saludo >= 0) {
            float levantar =
                glm::smoothstep(0.f, 1.2f, saludo) * (1 - glm::smoothstep(4.8f, 6.f, saludo));
            if (nombre == "def_r_shoulder_096")
                angSaludo = -1.05f * levantar;
            if (nombre == "def_r_elbow_097")
                angSaludo = -1.4f * levantar;
            const float vaiven[] = {0, -.38f, .38f, -.38f, .38f, -.38f, .38f, 0};
            float faseSaludo = glm::clamp((saludo - 1.2f) / .5f, 0.f, 7.f);
            int k = std::min(6, int(faseSaludo));
            float onda =
                glm::mix(vaiven[k], vaiven[k + 1], glm::smoothstep(0.f, 1.f, faseSaludo - k)) *
                levantar;
            if (nombre == "def_r_elbow_097")
                lateral = onda * .6f;
            if (nombre == "def_r_wrist_098")
                lateral = onda * .4f;
        }
        auto padre = n.padre < 0 ? glm::mat4(1) : nodos_[n.padre].global;
        glm::vec3 piv(local[3]);
        auto girar = [&](float angulo, glm::vec3 mundo) {
            if (angulo == 0)
                return;
            auto eje = glm::normalize(glm::inverse(glm::mat3(padre)) * mundo);
            local = glm::translate(glm::mat4(1), piv) * glm::rotate(glm::mat4(1), angulo, eje) *
                    glm::translate(glm::mat4(1), -piv) * local;
        };
        girar(ang * amplitud + angSaludo, {1, 0, 0});
        girar(lateral, {0, 0, 1});
        n.global = padre * local;
    }
    std::vector<glm::mat4> m(juntas_.size());
    for (size_t i = 0; i < m.size(); ++i)
        m[i] = nodos_[juntas_[i]].global * inversas_[i];
    float minimo = 1e9f;
    desplazamiento_ = 0;
    for (auto &s : pieles_)
        for (size_t i = 0; i < s.base.size(); ++i) {
            glm::mat4 d(0);
            for (int k = 0; k < 4; ++k)
                d += s.pesos[i][k] * m[s.juntas[i][k]];
            auto &v = s.deformada[i];
            v = s.base[i];
            v.posicion = glm::vec3(d * glm::vec4(v.posicion, 1));
            v.normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(d))) * v.normal);
            minimo = std::min(minimo, v.posicion.y);
            if (!std::isfinite(v.posicion.y))
                throw std::runtime_error("BT deformacion invalida");
        }
    apoyo_ = minimo;
    for (auto &s : pieles_) {
        for (size_t i = 0; i < s.indices.size(); ++i) {
            auto v = s.deformada[s.indices[i]];
            v.posicion = (v.posicion - glm::vec3(0, apoyo_, 0)) * escala_;
            s.dibujo[i] = v;
        }
        s.malla->actualizar(s.dibujo, true);
    }
    desplazamiento_ = amplitud * std::abs(clave(fase, {-.32f, 0, .32f, 0, -.32f}));
}
glm::vec3 Titan::posicion(float t) {
    float z = 0;
    if (t >= .6f && t < 5.6f)
        z = 6 * (t - .6f) / 5;
    else if (t >= 5.6f && t < 7)
        z = 6;
    else if (t >= 7 && t < 12)
        z = 6 * (1 - (t - 7) / 5);
    return {-95, 2.03f, 153 + z};
}
float Titan::giro(float t) {
    if (t < 5.6f)
        return 0;
    if (t < 7)
        return 3.14159265f * glm::smoothstep(5.6f, 7.f, t);
    if (t < 12)
        return 3.14159265f;
    return 3.14159265f * (1 - glm::smoothstep(12.f, 14.f, t));
}
void Titan::dibujar(Renderizador &r, const glm::mat4 &m) const {
    for (auto &s : pieles_)
        r.texturada(*s.malla, m, s.textura ? s.textura->id() : 0, s.color, {0, 0, 0}, .7f, .3f);
}

glm::vec3 Titan::extremo(const std::string &nombre) const {
    for (auto &n : nodos_)
        if (n.nombre == nombre)
            return (glm::vec3(n.global[3]) - glm::vec3(0, apoyo_, 0)) * escala_;
    throw std::runtime_error("BT articulacion ausente");
}
float Titan::alturaMinima() const {
    float y = 1e9f;
    for (auto &p : pieles_)
        for (auto &v : p.dibujo)
            y = std::min(y, v.posicion.y);
    return y;
}
