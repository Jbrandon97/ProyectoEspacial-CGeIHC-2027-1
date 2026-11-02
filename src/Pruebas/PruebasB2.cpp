#include "Nucleo/Aplicacion.h"
#include "Personajes/Engineer.h"
#include "Regiones/Planeta1/Planeta1.h"
#include <cctype>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
void Aplicacion::probarB2(const std::filesystem::path &evidencias) {
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto exigir = [](bool b, const char *t) {
        if (!b)
            throw std::runtime_error(t);
    };
    auto caminar = [&](Avatar &a, glm::vec2 d, int n) {
        for (int i = 0; i < n; ++i)
            a.mover(d, 1.0f / 120, p);
    };
    Avatar peat;
    peat.colocar({210, 0, 148}, p);
    caminar(peat, {-1, 0}, 144);
    exigir(peat.posicion().x < 197 && peat.posicion().y > 3.9f, "B2 no sube rampa peatonal");
    caminar(peat, {1, 0}, 144);
    exigir(peat.posicion().x > 209 && peat.posicion().y < 3.2f, "B2 no baja rampa peatonal");
    std::cout << "B2_RAMPA_OK ida vuelta sin salto\n";
    OrdenActividad o;
    o.avatar = {165, 3, -65};
    o.puerta = true;
    p.controlar(o);
    o.puerta = false;
    for (int i = 0; i < 240; ++i) {
        p.controlar(o);
        p.actualizar(1.0f / 60);
    }
    exigir(p.elevacionPuerta() == 0, "B2 puerta no cierra");
    peat.colocar(o.avatar, p);
    caminar(peat, {0, -1}, 320);
    std::cout << "PUERTA_CERRADA z=" << peat.posicion().z << " y=" << peat.posicion().y << "\n";
    exigir(peat.posicion().z > -77.0f, "B2 atraviesa puerta cerrada");
    ContextoEscena contexto{renderizador_, recursos_};
    int w, h;
    ventana_.dimensiones(w, h);
    auto vista = [&](const char *nombre, glm::vec3 ojo, glm::vec3 objetivo) {
        auto vp = glm::perspective(glm::radians(50.0f), float(w) / h, .1f, 1400.0f) *
                  glm::lookAt(ojo, objetivo, {0, 1, 0});
        renderizador_.iniciar(w, h, vp, p.ambiente());
        p.dibujar(contexto, mundo_.transformacion(Region::Planeta1));
        std::string titulo(nombre);
        for (auto &letra : titulo)
            letra = letra == '_' ? ' ' : char(std::toupper(static_cast<unsigned char>(letra)));
        interfaz_.preparar({"BLOQUE B2 / VERIFICACION OPENGL", titulo}, w, h);
        renderizador_.interfaz(interfaz_.malla(), w, h);
        renderizador_.capturar(evidencias / (std::string(nombre) + ".ppm"), w, h);
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
    };
    vista("Hangar_cerrado", {190, 13, -45}, {165, 12, -100});
    o.puerta = true;
    p.controlar(o);
    o.puerta = false;
    for (int i = 0; i < 240; ++i) {
        p.controlar(o);
        p.actualizar(1.0f / 60);
    }
    exigir(p.elevacionPuerta() == 7, "B2 puerta no abre");
    peat.colocar(o.avatar, p);
    caminar(peat, {0, -1}, 320);
    exigir(peat.posicion().z < -100, "B2 no atraviesa puerta abierta");
    vista("Hangar_abierto", {190, 13, -45}, {165, 12, -100});
    o.avatar = {165, 3, -77.5f};
    o.puerta = true;
    p.controlar(o);
    p.actualizar(.1f);
    exigir(p.elevacionPuerta() == 7, "Sensor cierre no protege avatar");
    o.puerta = false;
    o.avatar = {0, 0, 0};
    p.controlar(o);
    std::cout << "B2_HANGAR_OK abrir cerrar paso bloqueado sensor\n";
    vista("Rampa_peatonal", {219, 11, 162}, {200, 3, 148});
    vista("NPC_recinto", {-49, 11, 185}, {-65, 3, 158});
    vista("Sentinel_exterior", {-99, 12, -84}, {-110, 9, -95});
    vista("Espadas_Covenant", {-144, 10, -45}, {-144, 2.1f, -61});
    Engineer engineer(recursos_ / "Modelos/Engineer.gltf");
    for (int estado = 0; estado < 3; ++estado) {
        float act = estado == 0 ? 0 : estado == 1 ? .45f : 1;
        engineer.animar(.35f, act);
        float a = engineer.desplazamientoExtremidad();
        engineer.animar(1.1f, act);
        float b = engineer.desplazamientoExtremidad();
        exigir(engineer.deformacionMaxima(0) < .00001f, "B2 mueve cuerpo en vez de extremidades");
        for (int cadena = 1; cadena <= 3; ++cadena) {
            float d = engineer.deformacionMaxima(cadena);
            exigir(estado == 0 ? d < .00001f : (d > .001f && d < .6f),
                   "E deformacion o reposo incorrecto");
            std::cout << "B2_PIEL estado=" << estado << " cadena=" << cadena << " max=" << d
                      << "\n";
        }

        exigir(estado == 0 ? (a < .00001f && b < .00001f)
                           : (a > .00001f && std::abs(b - a) > .00001f),
               "E extremidad: reposo o locomocion incorrectos");
        std::cout << "B2_RIG_OK estado=" << estado << " desplazamientoExtremo=" << a << "," << b
                  << "\n";
        auto vp = glm::perspective(glm::radians(40.0f), float(w) / h, .1f, 100.0f) *
                  glm::lookAt(glm::vec3(4, 1.5f, 4), glm::vec3(.2f, 0, 0), glm::vec3(0, 1, 0));
        for (int frame = 0; frame < 2; ++frame) {
            engineer.animar(.35f + frame * .75f, act);
            renderizador_.iniciar(w, h, vp, p.ambiente());
            engineer.dibujar(renderizador_, glm::mat4(1));
            interfaz_.preparar({"ENGINEER / TRES CADENAS ARTICULADAS",
                                estado == 0   ? "REPOSO"
                                : estado == 1 ? "DESPLAZAMIENTO"
                                              : "CARRERA",
                                "CAPTURA REAL / CAMARA DE INSPECCION"},
                               w, h);
            renderizador_.interfaz(interfaz_.malla(), w, h);
            renderizador_.capturar(evidencias / ("Engineer_" + std::to_string(estado) + "_" +
                                                 std::to_string(frame) + ".ppm"),
                                   w, h);
            Renderizador::comprobarErrores();
            ventana_.presentar();
        }
    }
    std::cout << "B2_COMPLETO_OK 3 Grunts 1 Hunter 2 Sentinels 2 espadas cargados\n";
}
