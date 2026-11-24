#pragma once
#include "Graficos/ModeloGltf.h"
#include "Graficos/Iluminacion.h"
#include "Mundo/ContextoEscena.h"
#include <array>
#include <functional>
#include <memory>

// Objetos de ambos sectores en las coordenadas locales del croquis integrado.
class SectorRatchet {
  public:
    static constexpr int CantidadNPC = 3, CantidadMunecos = 6;
    static constexpr float RitmoNinja = 1.5f;
    struct Contacto {
        glm::vec2 centro{}, avance{};
        float radio = 0;
    };

  private:
    std::unique_ptr<ModeloGltf> edificio1_, edificio2_, edificio3_;
    std::unique_ptr<ModeloGltf> railjack_, orokintower_, warframe_, aphelion_;
    std::unique_ptr<ModeloGltf> clank_, blarg_;
    std::unique_ptr<ModeloGltf> estatua_;
    std::array<std::unique_ptr<ModeloGltf>, 2> munecos_;
    std::array<std::unique_ptr<ModeloGltf>, 4> utileria_;
    std::unique_ptr<Malla> arbol_, pavimento_, estructuras_, balizasDia_, humo_, haz_, propulsor_;
    std::array<std::unique_ptr<Malla>, 6> balizas_;
    std::array<glm::vec3, 4> reflectores_{};
    std::vector<std::array<glm::vec2, 3>> huellaTorre_;
    std::vector<glm::vec3> arboles_;
    std::vector<float> distanciasPavimento_;
    std::vector<glm::mat4> partesRailjack_;
    std::function<float(float, float)> suelo_;
    std::array<float, 6> apoyos_{}, huecosBase_{};
    std::array<glm::vec2, 6> contactoBase_{};
    std::array<glm::vec3, 6> lucesEdificios_{}, centrosEdificios_{};
    std::array<Contacto, CantidadNPC + CantidadMunecos> contactos_{};
    float tiempo_ = 0, apoyoTorre_ = 0, apoyoRailjack_ = 0, pisoPuerto_ = 0;
    double distanciaPatrulla_ = 0;

    void construirUrbanismo();
    void actualizarContactos(bool inicial = false);
    glm::vec2 rutaNPC(int i, float t) const;
    glm::vec2 rutaMuneco(int i, double distancia) const;
    void apoyarTorre();
    float sueloConPuerto(float x, float z) const;
    glm::mat4 matrizEstatua() const;

  public:
    const auto &contactos() const { return contactos_; }
    static float fasePasoNinja(float tiempo);
    const auto &huecosBase() const {
        return huecosBase_;
    }
    const auto &contactoBase() const {
        return contactoBase_;
    }
    const auto &centrosEdificios() const {
        return centrosEdificios_;
    }
    glm::vec3 luzEdificio(size_t i) const {
        return lucesEdificios_.at(i);
    }
    void usarSuelo(std::function<float(float, float)> suelo) {
        suelo_ = std::move(suelo);
    }
    void cargar(ContextoEscena &servicios);
    void actualizar(float deltaTime);
    void dibujar(ContextoEscena &servicios, const glm::mat4 &sectorAlMundo, float noche,
                 bool reflectoresActivos) const;
    void dibujarPavimento(Renderizador &, const glm::mat4 &sectorAlMundo, float noche) const;
    void liberar() noexcept;
    bool obstaculo(glm::vec3 posLocal) const;
    glm::vec3 posicionNPC(int i, float t) const;
    glm::mat4 matrizNPC(int i, float t) const;
    float reloj() const {
        return tiempo_;
    }
    glm::vec3 aphelionPosicion() const;
    glm::vec3 railjackPosicion() const;
    float railjackAlturaBase() const;
    float superficie(float x, float z) const;
    glm::vec3 centroPlaza() const;
    glm::vec3 centroTorre() const;
    glm::vec3 posicionAphelion(float t) const;
    glm::vec3 frenteAphelion(float t) const;
    glm::vec3 posicionMuneco(int i, double distancia) const;
    glm::vec3 frenteMuneco(double distancia, int i = 0) const;
    void dibujarMuneco(Renderizador &, const glm::mat4 &alMundo, int i, double distancia,
                       float tiempoBrazos) const;
    double longitudPatrulla() const;
    double distanciaPatrulla() const { return distanciaPatrulla_; }
    std::array<LuzFocal, 4> reflectoresRailjack(float intensidad) const;
    LuzFocal reflectorAphelion(float noche) const;
};
