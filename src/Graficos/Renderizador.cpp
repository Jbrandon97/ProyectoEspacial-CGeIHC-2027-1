#define GLM_ENABLE_EXPERIMENTAL
#include "Graficos/Renderizador.h"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stdexcept>
#include <vector>
Renderizador::Renderizador(const std::filesystem::path &r)
    : shader_(r / "Shaders/basico.vert", r / "Shaders/basico.frag"),
      cielo_(r / "Shaders/cielo.vert", r / "Shaders/cielo.frag"), recursos_(r) {
    glGenVertexArrays(1, &vaoCielo_);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
void Renderizador::iniciar(int a, int b, const glm::mat4 &vp, Ambiente ambiente, glm::vec3 ojo) {
    dibujos_ = 0;
    vp_ = vp;
    glViewport(0, 0, a, b);
    glClearColor(ambiente.fondo.x, ambiente.fondo.y, ambiente.fondo.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto &l = ambiente.iluminacion;
    if (l.activa && !ambiente.cielo.empty()) {
        if (seleccionCielo_ != ambiente.cielo) {
            auto &par = cielos_[ambiente.cielo];
            if (!par.first) {
                par.first = std::make_shared<Textura>(recursos_ / "Cielos" / ambiente.cielo /
                                                      "skybox-day.png");
                if (ambiente.cielo == "Planeta1")
                    par.second = std::make_shared<Textura>(recursos_ / "Cielos" / ambiente.cielo /
                                                           "skybox-night.png");
                else
                    par.second = par.first;
            }
            cieloDia_ = par.first;
            cieloNoche_ = par.second;
            seleccionCielo_ = ambiente.cielo;
        }
        cielo_.usar();
        cielo_.matriz("inversaVP", glm::inverse(vp));
        cielo_.vector("ojo", ojo);
        cielo_.vector("mezcla", {l.noche,
                                 ambiente.cielo == "Espacio"
                                     ? .35f
                                     : (ambiente.cielo == "Planeta2" ? .65f - .32f * l.noche : 1.f),
                                 ambiente.cielo == "Planeta2" ? 1.f : 0.f});
        glm::vec3 astro = glm::normalize(glm::vec3(-.54f, .78f, .31f));
        cielo_.matriz("rotDia",
                      ambiente.cielo == "Espacio"
                          ? glm::rotate(glm::mat4(1), glm::radians(-108.f), glm::vec3(0, 1, 0))
                          : glm::mat4_cast(glm::rotation(glm::normalize(l.sol), astro)));
        cielo_.matriz("rotNoche",
                      ambiente.cielo == "Espacio"
                          ? glm::rotate(glm::mat4(1), glm::radians(-108.f), glm::vec3(0, 1, 0))
                          : glm::mat4_cast(glm::rotation(-glm::normalize(l.sol), astro)));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cieloDia_->id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, (cieloNoche_ ? cieloNoche_->id() : cieloDia_->id()));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        cielo_.entero("dia", 0);
        cielo_.entero("noche", 1);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glBindVertexArray(vaoCielo_);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
    }
    shader_.usar();
    shader_.vector("sistema", {l.activa ? 1.f : 0.f, l.fuerzaSol, l.ambiente});
    shader_.vector("solDir", l.sol);
    shader_.vector("solColor", l.colorSol);
    shader_.vector("ojo", ojo);
    int cantidad = 0;
    for (const auto &p : l.puntuales) {
        if(!p.encendida || p.intensidad<=0) continue;
        auto nombre = "pLuces[" + std::to_string(cantidad++) + "]";
        shader_.vector((nombre + ".pos").c_str(), p.posicion);
        shader_.vector((nombre + ".color").c_str(), p.color * (p.encendida ? p.intensidad : 0));
        shader_.vector((nombre + ".atenuacion").c_str(), p.atenuacion);
    }
    shader_.entero("cantidadPuntuales", cantidad);
    for (int i = 0; i < 2; ++i) {
        const auto &f = l.focales[i];
        auto n = "focales[" + std::to_string(i) + "]";
        shader_.vector((n + ".pos").c_str(), f.posicion);
        shader_.vector((n + ".dir").c_str(), f.direccion);
        shader_.vector((n + ".color").c_str(), f.color * (f.encendida ? f.intensidad : 0));
        shader_.vector((n + ".atenuacion").c_str(), f.atenuacion);
        shader_.vector((n + ".cono").c_str(),
                       {std::cos(glm::radians(f.interno)), std::cos(glm::radians(f.externo)), 0});
    }
    shader_.entero("usaEmision", 0);
    recortarDebajo(-100000.f);
    shader_.vector("focoPos", l.focoPos);
    shader_.vector("focoDir", l.focoDir);
    shader_.vector("focoDatos", {l.foco ? 3.8f : 0.f, std::cos(glm::radians(22.f)),
                                 std::cos(glm::radians(31.f))});
    material(.85f);

    shader_.matriz("vistaProyeccion", vp);
    shader_.vector("luz", ambiente.luz);
    shader_.vector("opciones", {0, 0, 0});
    shader_.vector("texturizado", {0, 0, 0});
    shader_.vector("emision", {0, 0, 0});
}
void Renderizador::dibujar(const Objeto &o, const glm::mat4 &base) {
    auto m = glm::translate(base, o.posicion);
    m = glm::rotate(m, o.giro, glm::vec3(0, 1, 0));
    m = glm::scale(m, o.escala);
    shader_.matriz("modelo", m);
    shader_.matriz("matrizNormal", glm::transpose(glm::inverse(m)));
    shader_.vector("color", o.color);
    cubo_.dibujar();
    ++dibujos_;
}
void Renderizador::capturar(const std::filesystem::path &p, int a, int b) const {
    if (!capturasHabilitadas_)
        return;
    // PPM sin dependencias extra. OpenGL entrega filas desde abajo: se invierten al escribir.
    std::vector<unsigned char> pix(size_t(a) * size_t(b) * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, a, b, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::binary);
    if (!f)
        throw std::runtime_error("No se pudo crear captura");
    f << "P6\n" << a << ' ' << b << "\n255\n";
    for (int y = b - 1; y >= 0; --y)
        f.write(reinterpret_cast<const char *>(pix.data() + size_t(y) * a * 3),
                std::streamsize(a) * 3);
    if (!f)
        throw std::runtime_error("Captura incompleta");
}
void Renderizador::comprobarErrores() {
    auto e = glGetError();
    if (e != GL_NO_ERROR)
        throw std::runtime_error("Error OpenGL: " + std::to_string(e));
}

void Renderizador::dibujar(const Malla &m, const glm::mat4 &modelo, glm::vec3 tinte) {
    shader_.matriz("modelo", modelo);
    shader_.matriz("matrizNormal", glm::transpose(glm::inverse(modelo)));
    shader_.vector("color", tinte);
    m.dibujar();
    ++dibujos_;
}
void Renderizador::interfaz(const Malla &m, int a, int b) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    shader_.matriz("vistaProyeccion", glm::ortho(0.0f, float(a), float(b), 0.0f, -1.0f, 1.0f));
    shader_.vector("opciones", {1, 0, 0});
    dibujar(m, glm::mat4(1));
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void Renderizador::texturada(const Malla &m, const glm::mat4 &modelo, GLuint textura,
                             glm::vec3 color, glm::vec3 emision, float rugosidad, float metal,
                             GLuint emisionTextura) {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emisionTextura);
    shader_.entero("mapaEmision", 2);
    shader_.entero("usaEmision", emisionTextura ? 1 : 0);
    material(rugosidad, metal);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textura);
    shader_.vector("texturizado", {textura ? 1.0f : 0.0f, 0, 0});
    shader_.vector("emision", emision);
    glDisable(GL_CULL_FACE);
    dibujar(m, modelo, color);
    glEnable(GL_CULL_FACE);
    shader_.vector("texturizado", {0, 0, 0});
    shader_.vector("emision", {0, 0, 0});
    material(.85f);
}

bool Renderizador::visible(glm::vec3 c, float radio) const {
    auto t = glm::transpose(vp_);
    for (int i = 0; i < 3; ++i)
        for (float signo : {-1.0f, 1.0f}) {
            auto p = t[3] + signo * t[i];
            if (glm::dot(glm::vec3(p), c) + p.w < -radio * glm::length(glm::vec3(p)))
                return false;
        }
    return true;
}

Renderizador::~Renderizador() {
    glDeleteVertexArrays(1, &vaoCielo_);
}
void Renderizador::material(float r, float m) {
    shader_.vector("propiedades", {r, m, 0});
}
void Renderizador::locomocionRobot(float fase, bool activa) {
    shader_.vector("locomocionClank", {fase, activa ? 1.f : 0.f, 0});
}
void Renderizador::locomocionPatrulla(float fase, bool hunter) {
    shader_.vector("locomocionClank", {fase, 3.f, hunter ? 1.f : 0.f});
}

void Renderizador::locomocionBlarg(float fase) { shader_.vector("locomocionClank",{fase,2,0}); }
