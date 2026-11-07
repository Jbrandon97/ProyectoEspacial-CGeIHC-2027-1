#pragma once
// Lector independiente de OpenGL: alturas correspondientes a los vertices del GLTF.
// Malla 600x600, con separacion de 3 unidades y cuadrantes X/Z en [-300,300].
// Se utiliza para posicionar jugadores, objetos o calcular altura de apoyo.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

class AlturasTerreno {
    static constexpr int N = 201;
    std::vector<float> alturas_;
public:
    explicit AlturasTerreno(const std::filesystem::path& archivo) {
        std::ifstream f(archivo, std::ios::binary | std::ios::ate);
        if (!f || f.tellg() != std::streampos(N*N*sizeof(float)))
            throw std::runtime_error("Archivo de alturas ausente o tamano incorrecto: " + archivo.string());
        f.seekg(0);
        alturas_.resize(N*N);
        f.read(reinterpret_cast<char*>(alturas_.data()), N*N*sizeof(float));
        if (!f) throw std::runtime_error("No se pudo leer mapa de alturas");
    }
    float altura(float x, float z) const {
        float gx = std::clamp((x+300.f)/3.f, 0.f, 200.f);
        float gz = std::clamp((z+300.f)/3.f, 0.f, 200.f);
        int i = int(std::floor(gx)), j = int(std::floor(gz));
        int ii = std::min(200,i+1), jj = std::min(200,j+1);
        float tx = gx-i, tz = gz-j;
        auto h = [&](int a, int b) {return alturas_[a*N+b];};
        return (1-tx)*((1-tz)*h(i,j)+tz*h(i,jj)) + tx*((1-tz)*h(ii,j)+tz*h(ii,jj));
    }
};
