#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <array>
#include <vector>
#include <string>

// Posiciones y velocidades en unidades de modelo; Y arriba. Quaternion XYZW en TXT.
struct CuadroClave {
    double tiempo = 0;
    glm::vec3 posicion{0}, velocidad{0};
    glm::quat orientacion{1, 0, 0, 0};
    int estado = 0; // 0 unido, 1 separado, 2 suelo, 3 orbita
};
struct PistaCohete {
    std::string nombre;
    glm::vec3 pivote{0};
    std::vector<CuadroClave> cuadros;
};
class KeyframesCohete {
  public:
    bool Guardar(const std::string &ruta) const;
    bool Leer(const std::string &ruta);
    bool Reproducir();
    void Reiniciar();
    void Actualizar(float dt);
    CuadroClave Evaluar(size_t componente, double t) const;
    CuadroClave Actual(size_t componente) const {
        return Evaluar(componente, tiempo);
    }
    glm::mat4 Matriz(size_t componente, glm::vec3 origen = glm::vec3(0)) const;
    void Listar() const;
    size_t Cantidad() const;
    bool EstaReproduciendo() const {
        return reproduciendo;
    }
    double Tiempo() const {
        return tiempo;
    }
    double InicioOrbita() const {
        return inicioOrbita;
    }
    double FinOrbita() const {
        return finOrbita;
    }
    const PistaCohete &Pista(size_t i) const {
        return pistas.at(i);
    }

  private:
    std::array<PistaCohete, 4> pistas;
    double tiempo = 0, inicioOrbita = 24, finOrbita = 48;
    bool cargado = false, reproduciendo = false;
};
