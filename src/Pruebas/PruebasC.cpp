#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Distribucion.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <numeric>
#include <stdexcept>
void Aplicacion::probarC(const std::filesystem::path &dir) {
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto &s = p.sistemas();
    auto original = s;
    auto exigir = [](bool b, const char *msg) {
        if (!b)
            throw std::runtime_error(msg);
    };
    float dia = 0, noche = 0;
    for (int j = 0; j <= 1200; ++j) {
        s.tiempo = j * s.ciclo / 1200;
        auto l = s.luces();
        dia = std::max(dia, l.fuerzaSol);
        noche = std::max(noche, l.noche);
        for (int k = 0; k < 2; ++k) {
            auto q = s.sentinel(k);
            exigir(q.y > TerrenoPlaneta1::altura(q.x, q.z) + 8, "Sentinel toca suelo");
            for (auto &e : Distribucion::edificios)
                exigir(std::abs(q.x - e.centro.x) > e.semiejes.x + 2 ||
                           std::abs(q.z - e.centro.y) > e.semiejes.y + 2,
                       "Sentinel entra en edificio");
        }
    }
    exigir(s.ciclo <= 120 && dia > .9f && noche > .99f, "Ciclo incompleto");
    s = original;
    std::cout << "C_CICLO_OK segundos=" << s.ciclo << " 1201 muestras patrullas libres\n";
    Avatar peat;
    peat.colocar({210, 0, 148}, p);
    for (int k = 0; k < 288; ++k)
        peat.mover({-1, 0}, 1.f / 120, p);
    exigir(peat.posicion().x < 178 && peat.posicion().y < 3.2f, "Acceso interior incompleto");
    for (int k = 0; k < 288; ++k)
        peat.mover({1, 0}, 1.f / 120, p);
    exigir(peat.posicion().x > 209 && peat.posicion().y < 3.2f, "Retorno exterior incompleto");
    // Regreso desde interior hasta nave: cruce de rampa y paseo por recta.
    peat.colocar({177, 0, 148}, p);
    for (int k = 0; k < 128; ++k)
        peat.mover({1, 0}, 1.f / 120, p);
    for (int k = 0; k < 72; ++k)
        peat.mover({0, -1}, 1.f / 120, p);
    exigir(glm::length(glm::vec2(peat.posicion().x - 193, peat.posicion().z - 139)) < 1,
           "No regresa a nave");
    std::cout << "C_ACCESOS_OK exterior interior ambos sentidos regreso nave\n";
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena c{renderizador_, recursos_};
    auto vista = [&](const char *nombre, glm::vec3 ojo, glm::vec3 objetivo, int opcion = 0) {
        auto amb = p.ambiente();
        if (opcion == 1)
            for (auto &l : amb.iluminacion.puntuales)
                l.encendida = false;
        if (opcion == 2) {
            amb.iluminacion.foco = true;
            amb.iluminacion.focoPos = ojo;
            amb.iluminacion.focoDir = glm::normalize(objetivo - ojo);
        }
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .15f, 900.f) *
                  glm::lookAt(ojo, objetivo, {0, 1, 0});
        double ini = glfwGetTime();
        renderizador_.iniciar(w, h, vp, amb, ojo);
        p.dibujar(c, glm::mat4(1));
        glFinish();
        double ms = (glfwGetTime() - ini) * 1000;
        renderizador_.capturar(dir / (std::string(nombre) + ".ppm"), w, h);
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
        return ms;
    };
    s.faseInicial = .25f;
    s.tiempo = 0;
    vista("C_Dia", {28, 23, -49}, {75, 9, -113});
    s.faseInicial = .0f;
    vista("C_Amanecer", {28, 23, -49}, {75, 9, -113});
    s.faseInicial = 2.f / 3.f;
    vista("C_Atardecer", {28, 23, -49}, {75, 9, -113});
    s.faseInicial = 5.f / 6.f;
    vista("C_Noche", {28, 23, -49}, {75, 9, -113});
    vista("C_Sin_puntuales", {28, 23, -49}, {75, 9, -113}, 1);
    vista("C_Foco", {28, 23, -49}, {75, 9, -113}, 2);
    vista("C_Foco_cerca_sin", {58, 8, -70}, {65, 3, -84});
    vista("C_Foco_cerca", {58, 8, -70}, {65, 3, -84}, 2);
    std::vector<double> tiempos;
    for (int i = 0; i < 12; ++i) {
        double ini = glfwGetTime();
        auto ojo = glm::vec3(28, 23, -49);
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .15f, 900.f) *
                  glm::lookAt(ojo, glm::vec3(75, 9, -113), glm::vec3(0, 1, 0));
        renderizador_.iniciar(w, h, vp, p.ambiente(), ojo);
        p.dibujar(c, glm::mat4(1));
        glFinish();
        tiempos.push_back((glfwGetTime() - ini) * 1000);
    }
    std::sort(tiempos.begin(), tiempos.end());
    std::cout << "C_RENDIMIENTO_NOCHE mediana_ms=" << tiempos[6] << " min=" << tiempos.front()
              << " max=" << tiempos.back() << " muestras=12 sin lectura de captura\n";
    s = original;
    s.faseInicial = .25f;
    s.tiempo = 0;
    vista("C_Rampas", {220, 20, 174}, {191, 2, 148});
    // Camera habitual del jugador, misma ubicacion y orientacion en dos fases de reposo.
    avatar_.colocar({-95, 0, -30}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    for (int j = 0; j < 120; ++j) {
        avatar_.mover({0, 0}, 1.f / 60, p);
        actualizarCamara(1.f / 60);
    }
    dibujar(dir / "C_Engineer_A.ppm");
    for (int j = 0; j < 60; ++j)
        avatar_.mover({0, 0}, 1.f / 60, p);
    dibujar(dir / "C_Engineer_B.ppm");
    for (int modo = 0; modo < 2; ++modo) {
        avatar_.colocar({-95, 0, -30}, p);
        for (int j = 0; j < 36; ++j) {
            avatar_.mover({1, 0}, 1.f / 60, p, modo == 1);
            actualizarCamara(1.f / 60);
        }
        dibujar(dir / (modo ? "C_Engineer_Correr.ppm" : "C_Engineer_Caminar.ppm"));
    }
    for (int k = 0; k < 2; ++k) {
        s.tiempo = k * 5.f;
        vista(k ? "C_Patrulla_B" : "C_Patrulla_A", {-92, 22, -18}, {-110, 11, -45});
    }
    for (int n : {1, 3}) {
        s.tiempo = 0;
        s.hunter = 0;
        s.inspeccion = 0;
        auto a = s.animacionNPC(n);
        s.tiempo = 2;
        s.hunter = 2;
        s.inspeccion = 2;
        auto b = s.animacionNPC(n);
        float d = 0;
        for (int j = 0; j < 4; ++j)
            d += glm::length(a[j] - b[j]);
        exigir(d > .1f, "Keyframes sin movimiento");
        s.tiempo = 8;
        s.hunter = 8;
        s.inspeccion = 8;
        auto fin = s.animacionNPC(n);
        exigir(glm::length(fin[0] - a[0]) < .001f, "Keyframes sin cierre");
    }
    OrdenActividad orden;
    orden.avatar = {-77, 3, 145};
    orden.reaccion = true;
    p.controlar(orden);
    exigir(s.reaccion == 0, "Grunt no reacciona");
    p.actualizar(2);
    exigir(glm::length(glm::vec3(s.animacionNPC(0)[3])) > 1, "Grunt no avanza");
    s.reaccion = -1;
    s.tiempo = 0;
    vista("C_NPC", {-46, 13, 188}, {-65, 3, 158});
    std::cout << "C_ANIMACIONES_OK 3 basicas 3 continuas 2 keyframes objetos distintos\n";
    orden = {};
    orden.avatar = {220, 6, 0};
    orden.lanzar = true;
    p.controlar(orden);
    exigir(s.lanzamiento == 0, "Lanzamiento invalido");
    avatar_.colocar(orden.avatar, p);
    observandoCohete_ = true;
    dibujar(dir / "C_Cohete_listo.ppm");
    float anterior = glm::vec3(s.etapas()[3] * glm::vec4(0, 45, 0, 1)).y;
    for (int j = 1; j <= 22 * 60 + 1; ++j) {
        p.actualizar(1.f / 60);
        exigir(glm::vec3(s.etapas()[3] * glm::vec4(0, 45, 0, 1)).y + .001f >= anterior,
               "Cohete retrocede");
        anterior = glm::vec3(s.etapas()[3] * glm::vec4(0, 45, 0, 1)).y;
        if (j == 120)
            dibujar(dir / "C_Motores.ppm");
        if (j == 480)
            dibujar(dir / "C_Ascenso.ppm");
        if (j == 840)
            dibujar(dir / "C_Altura.ppm");
    }
    exigir(glm::vec3(s.etapas()[3] * glm::vec4(0, 45, 0, 1)).y > 150 && s.lanzamiento > 21.9f,
           "Cohete no finaliza");
    dibujar(dir / "C_Final_lanzamiento.ppm");
    orden.lanzar = false;
    orden.reiniciarCohete = true;
    p.controlar(orden);
    for (int j = 0; j < 48 * 60 + 6; ++j)
        p.actualizar(1.f / 60);
    exigir(s.reposicion >= 0 && s.cohete().y < 0, "Cohete reaparece instantaneamente");
    for (int j = 0; j < 9 * 60; ++j)
        p.actualizar(1.f / 60);
    exigir(s.reposicion < 0 && s.lanzamiento < 0 && std::abs(s.cohete().y - 5.22f) < .01f,
           "Cohete no repone");
    observandoCohete_ = false;
    foco_ = false;
    s = original;
    std::cout << "C_COHETE_OK preparacion ascenso final reinicio camara\n";
    std::cout << "C_AUDIO disponible=" << audio_.disponible() << " bloques=" << audio_.bloques()
              << "\n";
}
