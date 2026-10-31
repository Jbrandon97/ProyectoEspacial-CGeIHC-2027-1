#include "Graficos/Interfaz.h"
#include <algorithm>
#include <array>
#include <map>
namespace {
// Cada byte representa una columna de siete pixeles, bit bajo arriba.
const std::map<char, std::array<unsigned char, 5>> letras = {
    {'A', {126, 9, 9, 9, 126}},   {'B', {127, 73, 73, 73, 54}}, {'C', {62, 65, 65, 65, 34}},
    {'D', {127, 65, 65, 34, 28}}, {'E', {127, 73, 73, 73, 65}}, {'F', {127, 9, 9, 9, 1}},
    {'G', {62, 65, 73, 73, 122}}, {'H', {127, 8, 8, 8, 127}},   {'I', {0, 65, 127, 65, 0}},
    {'J', {32, 64, 65, 63, 1}},   {'K', {127, 8, 20, 34, 65}},  {'L', {127, 64, 64, 64, 64}},
    {'M', {127, 2, 12, 2, 127}},  {'N', {127, 4, 8, 16, 127}},  {'O', {62, 65, 65, 65, 62}},
    {'P', {127, 9, 9, 9, 6}},     {'Q', {62, 65, 81, 33, 94}},  {'R', {127, 9, 25, 41, 70}},
    {'S', {38, 73, 73, 73, 50}},  {'T', {1, 1, 127, 1, 1}},     {'U', {63, 64, 64, 64, 63}},
    {'V', {31, 32, 64, 32, 31}},  {'W', {63, 64, 56, 64, 63}},  {'X', {99, 20, 8, 20, 99}},
    {'Y', {3, 4, 120, 4, 3}},     {'Z', {97, 81, 73, 69, 67}},  {'0', {62, 81, 73, 69, 62}},
    {'1', {0, 66, 127, 64, 0}},   {'2', {66, 97, 81, 73, 70}},  {'3', {33, 65, 69, 75, 49}},
    {'4', {24, 20, 18, 127, 16}}, {'5', {39, 69, 69, 69, 57}},  {'6', {60, 74, 73, 73, 48}},
    {'7', {1, 113, 9, 5, 3}},     {'8', {54, 73, 73, 73, 54}},  {'9', {6, 73, 73, 41, 30}},
    {'/', {32, 16, 8, 4, 2}},     {'-', {8, 8, 8, 8, 8}},       {'+', {8, 8, 62, 8, 8}},
    {'|', {0, 0, 127, 0, 0}},     {'.', {0, 96, 96, 0, 0}},     {':', {0, 54, 54, 0, 0}},
    {'(', {0, 28, 34, 65, 0}},    {')', {0, 65, 34, 28, 0}}};
} // namespace
void Interfaz::rectangulo(float x, float y, float a, float b, glm::vec3 color) {
    glm::vec3 n(0, 0, 1);
    vertices_.insert(vertices_.end(), {{{x, y, 0}, n, color},
                                       {{x + a, y, 0}, n, color},
                                       {{x + a, y + b, 0}, n, color},
                                       {{x, y, 0}, n, color},
                                       {{x + a, y + b, 0}, n, color},
                                       {{x, y + b, 0}, n, color}});
}
void Interfaz::texto(const std::string &s, float x, float y, float escala, glm::vec3 color) {
    for (char c : s) {
        auto it = letras.find(c);
        if (it != letras.end())
            for (int i = 0; i < 5; ++i)
                for (int j = 0; j < 7; ++j)
                    if (it->second[i] & (1 << j))
                        rectangulo(x + i * escala, y + j * escala, escala, escala, color);
        x += escala * 6;
    }
}
void Interfaz::preparar(const std::vector<std::string> &lineas, int ancho, int alto, bool pie) {
    vertices_.clear();
    float escala = ancho < 1000 ? 1.5f : 2.0f;
    float y = 20;
    size_t maximo = 0;
    for (const auto &l : lineas)
        maximo = std::max(maximo, l.size());
    rectangulo(10, 10, std::min(float(ancho - 20), float(maximo) * 6 * escala + 24),
               float(lineas.size()) * 23 + 18, {.025f, .055f, .075f});
    rectangulo(10, 10, 4, float(lineas.size()) * 23 + 18, {.16f, .87f, .85f});
    for (size_t i = 0; i < lineas.size(); ++i) {
        texto(lineas[i], 24, y, escala,
              i == 0 ? glm::vec3(.2f, .95f, .92f) : glm::vec3(.94f, .94f, .85f));
        y += 23;
    }
    if (pie)
        texto("1 PLANETA1  2 PLANETA2  3 ESPACIO  |  ESC SALIR", 20, float(alto - 25), escala,
              {.9f, .93f, .97f});
    malla_.actualizar(vertices_, true);
}
