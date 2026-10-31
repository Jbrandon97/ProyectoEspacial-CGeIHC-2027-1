#include "Pruebas/Verificacion.h"
#include "Camaras/GestorCamaras.h"
#include "Mundo/GestorEscenarios.h"
#include "Mundo/MundoEspacial.h"
#include "Mundo/RegistroEscenarios.h"
#include "Personajes/Avatar.h"
#include "Pruebas/PruebasB1.h"
#include "Pruebas/PruebasCarrera.h"
#include "Regiones/Planeta1/Planeta1.h"
#include <iostream>
#include <stdexcept>
namespace {
void exigir(bool v, const char *texto) {
    if (!v)
        throw std::runtime_error(texto);
}
// Doble de prueba: falla despues de comenzar carga; permite verificar rollback.
class Fallo final : public Escenario {
    bool &liberado_;

  public:
    explicit Fallo(bool &l) : liberado_(l) {}
    Region id() const noexcept override {
        return Region::Espacio;
    }
    const char *nombre() const noexcept override {
        return "Fallo controlado";
    }
    void cargar(ContextoEscena &) override {
        liberado_ = false;
        throw std::runtime_error("Fallo esperado");
    }
    void actualizar(float) override {}
    void dibujar(ContextoEscena &, const glm::mat4 &) const override {}
    void liberar() noexcept override {
        liberado_ = true;
    }
    glm::vec3 aparicion() const noexcept override {
        return {};
    }
    float altura(float, float) const noexcept override {
        return 0;
    }
    Ambiente ambiente() const noexcept override {
        return {};
    }
};
} // namespace
void verificarContratos(ContextoEscena &c) {
    verificarCarrera();
    MundoEspacial mundo;
    for (auto r : {Region::Planeta1, Region::Planeta2, Region::Espacio}) {
        auto p = glm::vec3(13, 7, -9);
        exigir(glm::length(mundo.aLocal(r, mundo.aGlobal(r, p)) - p) < .001f,
               "Conversion de coordenadas");
    }
    GestorEscenarios g;
    registrarEscenarios(g);
    exigir(g.cantidad() == 3, "Registro incompleto");
    g.activar(Region::Planeta1, c);
    exigir(!g.activar(Region::Planeta1, c), "Recarga innecesaria");
    verificarB1(g.actual());
    Avatar a, b;
    a.aparecer(g.actual());
    b.aparecer(g.actual());
    for (int i = 0; i < 60; ++i)
        a.mover({1, 0}, 1.0f / 60, g.actual());
    for (int i = 0; i < 30; ++i)
        b.mover({1, 0}, 1.0f / 30, g.actual());
    exigir(glm::length(a.posicion() - b.posicion()) < .001f, "Movimiento dependiente de FPS");
    exigir(std::abs(a.posicion().y - g.actual().altura(a.posicion().x, a.posicion().z) - 1) < .001f,
           "Altura del terreno");
    Avatar caminante, corredor;
    caminante.colocar({0, 0, 0}, g.actual());
    corredor.colocar({0, 0, 0}, g.actual());
    for (int i = 0; i < 60; ++i) {
        caminante.mover({1, 0}, 1.0f / 60, g.actual());
        corredor.mover({1, 1}, 1.0f / 60, g.actual(), true);
    }
    exigir(std::abs(caminante.posicion().x - 15) < .01f, "Velocidad caminar");
    exigir(std::abs(glm::length(glm::vec2(corredor.posicion().x, corredor.posicion().z)) - 30) <
               .01f,
           "Velocidad correr diagonal");
    std::cout << "MOVIMIENTO_OK caminar15 correr30 diagonal normalizada\n";
    GestorCamaras cam;
    const bool esperados[3][4] = {
        {true, true, true, false}, {true, false, true, false}, {false, false, true, true}};
    for (int r = 0; r < 3; ++r) {
        cam.alCambiarRegion(Region(r));
        for (int m = 0; m < 4; ++m) {
            auto antes = cam.modo();
            bool ok = cam.solicitar(ModoCamara(m), Region(r));
            exigir(ok == esperados[r][m], "Compatibilidad incorrecta");
            if (!ok)
                exigir(cam.modo() == antes, "Rechazo altero modo");
        }
    }
    bool duplicado = false;
    try {
        registrarEscenarios(g);
    } catch (const std::logic_error &) {
        duplicado = true;
    }
    exigir(duplicado, "No rechazo duplicado");
    bool liberado = false;
    GestorEscenarios fallo;
    fallo.registrar(std::make_unique<Planeta1>());
    fallo.activar(Region::Planeta1, c);
    fallo.registrar(std::make_unique<Fallo>(liberado));
    try {
        fallo.activar(Region::Espacio, c);
    } catch (const std::runtime_error &) {
    }
    exigir(liberado, "No limpio carga fallida");
    exigir(fallo.actual().id() == Region::Planeta1, "Carga fallida perdio region anterior");
    // Error de destino no registrado conserva el escenario anterior.
    try {
        g.activar(Region(99), c);
    } catch (const std::out_of_range &) {
    }
    exigir(g.actual().id() == Region::Planeta1, "Cambio fallido perdio region");
    std::cout << "CONTRATOS_OK coordenadas, dt, altura, 12 compatibilidades, duplicados, limpieza, "
                 "destino invalido\n";
}
