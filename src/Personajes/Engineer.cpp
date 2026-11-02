#include "Engineer.h"
#include "Graficos/Renderizador.h"
#include "cgltf.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>
Engineer::Engineer(const std::filesystem::path &p) {
    cgltf_options op{};
    cgltf_data *raw = nullptr;
    if (cgltf_parse_file(&op, p.string().c_str(), &raw) != cgltf_result_success)
        throw std::runtime_error("Engineer glTF invalido");
    std::unique_ptr<cgltf_data, decltype(&cgltf_free)> data(raw, cgltf_free);
    if (cgltf_load_buffers(&op, raw, p.string().c_str()) != cgltf_result_success ||
        cgltf_validate(raw) != cgltf_result_success || raw->skins_count != 1 ||
        raw->skins[0].joints_count != 10)
        throw std::runtime_error("Rig Engineer invalido");
    auto &skin = raw->skins[0];
    for (int j = 0; j < 10; ++j) {
        cgltf_node_transform_local(skin.joints[j], glm::value_ptr(locales_[j]));
        if (!cgltf_accessor_read_float(skin.inverse_bind_matrices, j, glm::value_ptr(inversas_[j]),
                                       16))
            throw std::runtime_error("Bind invalido");
        padres_[j] = -1;
        for (int k = 0; k < j; ++k)
            if (skin.joints[j]->parent == skin.joints[k])
                padres_[j] = k;
    }
    for (size_t mi = 0; mi < raw->meshes_count; ++mi)
        for (size_t pi = 0; pi < raw->meshes[mi].primitives_count; ++pi) {
            auto &pr = raw->meshes[mi].primitives[pi];
            Piel s;
            const cgltf_accessor *pos = nullptr, *nor = nullptr, *uv = nullptr, *jo = nullptr,
                                 *we = nullptr;
            for (size_t a = 0; a < pr.attributes_count; ++a) {
                auto &x = pr.attributes[a];
                if (x.type == cgltf_attribute_type_position)
                    pos = x.data;
                if (x.type == cgltf_attribute_type_normal)
                    nor = x.data;
                if (x.type == cgltf_attribute_type_texcoord)
                    uv = x.data;
                if (x.type == cgltf_attribute_type_joints)
                    jo = x.data;
                if (x.type == cgltf_attribute_type_weights)
                    we = x.data;
            }
            if (!pos || !nor || !uv || !jo || !we)
                throw std::runtime_error("Piel incompleta");
            for (size_t a = 0; a < pos->count; ++a) {
                Vertice v{};
                glm::vec4 ids, w;
                cgltf_accessor_read_float(pos, a, glm::value_ptr(v.posicion), 3);
                cgltf_accessor_read_float(nor, a, glm::value_ptr(v.normal), 3);
                cgltf_accessor_read_float(uv, a, glm::value_ptr(v.uv), 2);
                cgltf_accessor_read_float(jo, a, glm::value_ptr(ids), 4);
                cgltf_accessor_read_float(we, a, glm::value_ptr(w), 4);
                if (std::abs(w.x + w.y + w.z + w.w - 1) > .001f)
                    throw std::runtime_error("Pesos sin normalizar");
                s.base.push_back(v);
                s.juntas.push_back(glm::ivec4(ids));
                s.pesos.push_back(w);
            }
            s.malla = std::make_unique<Malla>(s.base);
            s.textura = std::make_unique<Textura>(
                p.parent_path() /
                pr.material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
            partes_.push_back(std::move(s));
        }
    animar(0, 0);
    std::cout << "ENGINEER_OK piel 10 articulaciones 3 cadenas texturas originales\n";
}
void Engineer::animar(float tiempo, float actividad) {
    float amplitud = .22f * std::sqrt(glm::clamp(actividad, 0.0f, 1.0f));
    // El avatar integra la fase con deltaTime: cambiar de estado no salta de fase.
    for (int j = 0; j < 10; ++j) {
        auto local = locales_[j];
        if (j) {
            int cadena = (j - 1) / 3, tramo = (j - 1) % 3;
            float ang = amplitud * std::sin(tiempo + cadena * 2.1f - tramo * .65f);
            local = glm::rotate(local, ang, glm::normalize(glm::vec3(.35f, 0, 1)));
        }
        pose_[j] = (padres_[j] < 0 ? glm::mat4(1) : pose_[padres_[j]]) * local;
    }
    std::array<glm::mat4, 10> deformar;
    for (int j = 0; j < 10; ++j)
        deformar[j] = pose_[j] * inversas_[j];
    desplazamientos_.fill(0);
    for (auto &s : partes_) {
        auto v = s.base;
        for (size_t i = 0; i < v.size(); ++i) {
            glm::mat4 m(0);
            for (int k = 0; k < 4; ++k)
                m += s.pesos[i][k] * deformar[s.juntas[i][k]];
            v[i].posicion = glm::vec3(m * glm::vec4(s.base[i].posicion, 1));
            v[i].normal = glm::normalize(glm::mat3(m) * s.base[i].normal);
            int grupo = s.pesos[i].x > .999f ? 0 : 1 + (s.juntas[i].y - 1) / 3;
            desplazamientos_[grupo] =
                glm::max(desplazamientos_[grupo], glm::length(v[i].posicion - s.base[i].posicion));
            if (!std::isfinite(v[i].posicion.x) || !std::isfinite(v[i].normal.y))
                throw std::runtime_error("Deformacion no finita");
        }
        s.malla->actualizar(v, true);
    }
}
void Engineer::dibujar(Renderizador &r, const glm::mat4 &m) const {
    for (auto &s : partes_)
        r.texturada(*s.malla, m, s.textura->id(), glm::vec3(1), glm::vec3(0), .65f, .05f);
}
float Engineer::desplazamientoExtremidad() const {
    return glm::length(glm::vec3(pose_[3] * inversas_[3] * glm::vec4(0, -1.47f, 0, 1)) -
                       glm::vec3(0, -1.47f, 0));
}
