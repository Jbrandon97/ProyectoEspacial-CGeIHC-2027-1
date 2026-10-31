#include "ModeloGltf.h"
#include "Renderizador.h"
#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4996)
#endif
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <stdexcept>
Textura::Textura(const std::filesystem::path &p) {
    std::ifstream f(p, std::ios::binary);
    if (!f)
        throw std::runtime_error("Textura ausente: " + p.string());
    std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)), {});
    int w, h, c;
    auto *pixels = stbi_load_from_memory(bytes.data(), int(bytes.size()), &w, &h, &c, 4);
    if (!pixels)
        throw std::runtime_error("PNG invalido: " + p.string());
    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
Textura::~Textura() {
    glDeleteTextures(1, &id_);
}
ModeloGltf::ModeloGltf(const std::filesystem::path &p) {
    cgltf_options opciones{};
    cgltf_data *raw = nullptr;
    if (cgltf_parse_file(&opciones, p.string().c_str(), &raw) != cgltf_result_success)
        throw std::runtime_error("glTF invalido: " + p.string());
    std::unique_ptr<cgltf_data, decltype(&cgltf_free)> datos(raw, cgltf_free);
    if (cgltf_load_buffers(&opciones, raw, p.string().c_str()) != cgltf_result_success ||
        cgltf_validate(raw) != cgltf_result_success)
        throw std::runtime_error("Buffer glTF invalido: " + p.string());
    // Cache debil: comparte imagenes, sin retener texturas al descargar la region/contexto.
    static std::map<std::filesystem::path, std::weak_ptr<Textura>> cache;
    size_t vertices = 0;
    for (size_t j = 0; j < raw->nodes_count; ++j) {
        auto &n = raw->nodes[j];
        if (!n.mesh)
            continue;
        for (size_t k = 0; k < n.mesh->primitives_count; ++k) {
            auto &pr = n.mesh->primitives[k];
            if (pr.type != cgltf_primitive_type_triangles)
                throw std::runtime_error("Solo triangulos glTF estaticos");
            const cgltf_accessor *pos = nullptr, *normal = nullptr, *uv = nullptr;
            for (size_t a = 0; a < pr.attributes_count; ++a) {
                auto &at = pr.attributes[a];
                if (at.type == cgltf_attribute_type_position)
                    pos = at.data;
                if (at.type == cgltf_attribute_type_normal)
                    normal = at.data;
                if (at.type == cgltf_attribute_type_texcoord && at.index == 0)
                    uv = at.data;
            }
            if (!pos || !normal)
                throw std::runtime_error("glTF sin posiciones/normales");
            Parte parte;
            cgltf_node_transform_world(&n, glm::value_ptr(parte.local));
            parte.puerta = n.name && std::string(n.name).find("Puerta_independiente") == 0;
            if (pr.material) {
                auto &mat = *pr.material;
                auto &pbr = mat.pbr_metallic_roughness;
                parte.rugosidad = pbr.roughness_factor;
                parte.metal = pbr.metallic_factor;
                parte.color = glm::make_vec3(pbr.base_color_factor);
                parte.emision = glm::make_vec3(mat.emissive_factor);
                if (pbr.base_color_texture.texture) {
                    auto *im = pbr.base_color_texture.texture->image;
                    if (!im || !im->uri)
                        throw std::runtime_error("Se requiere PNG externo");
                    auto ruta = (p.parent_path() / im->uri).lexically_normal();
                    parte.textura = cache[ruta].lock();
                    if (!parte.textura) {
                        parte.textura = std::make_shared<Textura>(ruta);
                        cache[ruta] = parte.textura;
                    }
                }
            }
            size_t count = pr.indices ? pr.indices->count : pos->count;
            std::vector<Vertice> v;
            v.reserve(count);
            for (size_t a = 0; a < count; ++a) {
                size_t ix = pr.indices ? cgltf_accessor_read_index(pr.indices, a) : a;
                Vertice vert{};
                if (!cgltf_accessor_read_float(pos, ix, glm::value_ptr(vert.posicion), 3) ||
                    !cgltf_accessor_read_float(normal, ix, glm::value_ptr(vert.normal), 3))
                    throw std::runtime_error("Accessor invalido");
                if (uv && !cgltf_accessor_read_float(uv, ix, glm::value_ptr(vert.uv), 2))
                    throw std::runtime_error("UV invalida");
                v.push_back(vert);
            }
            for (auto &vert : v)
                (parte.puerta ? puerta_ : triangulos_)
                    .push_back(glm::vec3(parte.local * glm::vec4(vert.posicion, 1)));
            vertices += count;
            parte.malla = std::make_unique<Malla>(v);
            partes_.push_back(std::move(parte));
        }
    }
    std::cout << "GLTF_OK " << p.filename().string() << " lotes=" << partes_.size()
              << " triangulos=" << vertices / 3 << "\n";
}
void ModeloGltf::dibujar(Renderizador &r, const glm::mat4 &m, float elevacionPuerta) const {
    for (auto &p : partes_) {
        auto local = p.local;
        if (p.puerta)
            local = glm::translate(glm::mat4(1), {0, elevacionPuerta, 0}) * local;
        r.texturada(*p.malla, m * local, p.textura ? p.textura->id() : 0, p.color, p.emision,
                    p.rugosidad, p.metal);
    }
}
