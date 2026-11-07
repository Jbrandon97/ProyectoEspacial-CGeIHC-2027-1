#pragma once
#include "Mundo/Tipos.h"
#include <filesystem>
#include <array>
#include <functional>
#include "KeyframesCohete.h"
// Estado regional compartido por dibujo, camara, pruebas y eventos. Sin dependencias de GLFW.
class SistemasPlaneta1 {
    KeyframesCohete animacionCohete_;
    std::array<float, 3> ajusteSuelo_{};

  public:
    const KeyframesCohete &keyframes() const {
        return animacionCohete_;
    }
    void calibrarApoyo(const std::array<std::vector<glm::vec3>, 4> &,
                       const std::function<float(float, float)> &);
    glm::vec3 objetivoObservacion() const;

    float tiempo = 0, ciclo = 120, faseInicial = .22f, lanzamiento = -1, reaccion = -1;
    float reposicion = -1, titan = -1, hunter = -1, saludo = -1, inspeccion = -1;
    bool reflector = false;
    bool retirarPreparado = false, reposicionPendiente = false;
    unsigned eventoCohete = 0, eventoGrunt = 0;
    void configurar(const std::filesystem::path &);
    void actualizar(float dt);
    void lanzar() {
        if (lanzamiento < 0 && reposicion < 0) {
            if (!animacionCohete_.Reproducir())
                return;
            lanzamiento = 0;
            ++eventoCohete;
        }
    }
    void reiniciar();
    float aperturaCompuerta() const;
    std::array<glm::mat4, 4> etapas() const;
    bool visibleCohete() const {
        return true;
    }
    float fase() const;
    float faseSolar() const;
    Iluminacion luces() const;
    glm::vec3 cohete() const;
    glm::vec3 fuenteMotor() const;
    glm::vec3 sentinel(int i) const;
    float giroSentinel(int i) const;
    glm::mat4 animacionNPC(int i) const;
    bool motor() const {
        return lanzamiento >= 1 && lanzamiento < 40;
    }
};
