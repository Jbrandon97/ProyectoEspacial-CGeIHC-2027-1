#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Distribucion.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iomanip>
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
    std::cout << "C_CICLO_OK segundos=" << s.ciclo
              << " 1201 muestras de iluminacion y patrulla de Sentinel\n";
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
    auto vista = [&](const char *nombre, glm::vec3 ojo, glm::vec3 objetivo, int opcion = 0,
                     float fov = 55.f) {
        auto amb = p.ambiente();
        if (opcion == 1)
            for (auto &l : amb.iluminacion.puntuales)
                l.encendida = false;
        if (opcion == 2) {
            amb.iluminacion.foco = true;
            amb.iluminacion.focoPos = ojo;
            amb.iluminacion.focoDir = glm::normalize(objetivo - ojo);
        }
        auto vp = glm::perspective(glm::radians(fov), float(w) / h, .15f, 900.f) *
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
    p.controlar({});
    s.reiniciarPatrullas();
    const glm::vec2 inicios[] = {{-77, 153}, {-65, 162}, {-53, 153}, {-65, 174}};
    for (int i = 0; i < 4; ++i) {
        const auto base = s.posicionPatrulla(i);
        exigir(glm::length(glm::vec2(base.x, base.z) - inicios[i]) < .001f,
               "NPC no inicia en su base");
        exigir(!s.marchaActiva(i) && s.relojPatrulla(i) == 0.f &&
                   s.factorMarchaNPC(i) == 0.f,
               "NPC inicia caminando en reposo");
        exigir(s.periodoPatrulla(i) >= 20 && s.periodoPatrulla(i) <= 30,
               "Periodo de fase de piernas fuera de rango");
    }
    vista("C_P1_NPC_Bases", {-46, 13, 188}, {-65, 3, 158});
    p.actualizar(3.f);
    for (int i = 0; i < 4; ++i)
        exigir(glm::length(s.posicionPatrulla(i) - glm::vec3(inicios[i].x,
                          TerrenoPlaneta1::altura(inicios[i].x, inicios[i].y), inicios[i].y)) <
                   .001f && s.relojPatrulla(i) == 0.f,
               "El NPC se mueve o su reloj avanza sin activacion");
    OrdenActividad hudGrunt;
    hudGrunt.avatar = s.posicionPatrulla(0);
    p.controlar(hudGrunt);
    auto mensajesGrunt = p.actividad().mensajes;
    exigir(std::find(mensajesGrunt.begin(), mensajesGrunt.end(), "G CAMINATA / GRUNT 1") !=
               mensajesGrunt.end(),
           "HUD no muestra la pulsacion unica para Grunt 1");

    auto activar = [](OrdenActividad &o, int npc) {
        if (npc == 0) o.caminarGrunt1 = true;
        if (npc == 1) o.caminarGrunt2 = true;
        if (npc == 3) o.caminarHunter = true;
    };
    exigir(!s.iniciarMarcha(2), "Grunt 3 no debe tener una tecla de marcha");
    for (int npc : {0, 1, 3}) {
        s.reiniciarPatrullas();
        OrdenActividad o;
        o.avatar = s.posicionPatrulla(npc);
        p.controlar(o);
        const auto mensajesAntes = p.actividad().mensajes;
        const char *mensajeEsperado = npc == 0 ? "G CAMINATA / GRUNT 1"
                                     : npc == 1 ? "K CAMINATA / GRUNT 2"
                                                : "H CAMINATA / HUNTER";
        exigir(std::find(mensajesAntes.begin(), mensajesAntes.end(), mensajeEsperado) !=
                   mensajesAntes.end(),
               "HUD no ofrece la tecla correcta al NPC quieto cercano");
        activar(o, npc);
        p.controlar(o);
        exigir(s.marchaActiva(npc), "La tecla asignada no inicia la caminata cercana");
        if (npc == 0) {
            const auto mensajes = p.actividad().mensajes;
            exigir(std::find(mensajes.begin(), mensajes.end(), "G CAMINATA / GRUNT 1") ==
                       mensajes.end(),
                   "HUD ofrece reactivar un NPC que ya camina");
        }
        for (int otro = 0; otro < 4; ++otro)
            if (otro != npc)
                exigir(!s.marchaActiva(otro), "La tecla activa otro NPC");
        p.actualizar(2.f);
        const float relojAntesDeRepetir = s.relojPatrulla(npc);
        o.avatar = s.posicionPatrulla(npc);
        p.controlar(o);
        exigir(std::abs(s.relojPatrulla(npc) - relojAntesDeRepetir) < .001f,
               "Una pulsacion durante la caminata reinicia el ciclo");
        const auto posicionAntesDeAvanzar = s.posicionPatrulla(npc);
        p.actualizar(.1f);
        exigir(s.relojPatrulla(npc) > relojAntesDeRepetir && s.caminandoNPC(npc) &&
                   glm::length(glm::vec2(s.posicionPatrulla(npc).x - posicionAntesDeAvanzar.x,
                                         s.posicionPatrulla(npc).z - posicionAntesDeAvanzar.z)) >
                       .02f,
               "El temporizador no avanza solo durante la marcha activa");
    }
    s.reiniciarPatrullas();
    OrdenActividad marchaG;
    marchaG.avatar = s.posicionPatrulla(0);
    marchaG.caminarGrunt1 = true;
    p.controlar(marchaG);
    p.actualizar(1.25f);
    OrdenActividad marchaK;
    marchaK.avatar = s.posicionPatrulla(1);
    marchaK.caminarGrunt2 = true;
    p.controlar(marchaK);
    exigir(std::abs(s.relojPatrulla(0) - 1.25f) < .001f &&
               s.relojPatrulla(1) == 0.f,
           "Los temporizadores de Grunt 1 y Grunt 2 no son independientes");
    p.actualizar(.5f);
    exigir(std::abs(s.relojPatrulla(0) - 1.75f) < .001f &&
               std::abs(s.relojPatrulla(1) - .5f) < .001f,
           "Los relojes activos no avanzan independientemente por deltaTime");
    OrdenActividad fueraDeRango;
    fueraDeRango.avatar = s.posicionPatrulla(0) + glm::vec3(0, 0, 20);
    fueraDeRango.caminarGrunt1 = true;
    s.reiniciarPatrullas();
    p.controlar(fueraDeRango);
    exigir(!s.marchaActiva(0), "G activa al Grunt 1 a 20 unidades");

    auto verificarCiclo = [&](int npc) {
        s.reiniciarPatrullas();
        OrdenActividad orden;
        orden.avatar = s.posicionPatrulla(npc);
        activar(orden, npc);
        p.controlar(orden);
        const auto base = s.posicionPatrulla(npc);
        const float rumboBase = s.orientacionPatrulla(npc);
        p.actualizar(.6f);
        exigir(glm::length(s.posicionPatrulla(npc) - base) < .001f &&
                   s.factorMarchaNPC(npc) == 0.f,
               "La preparación de marcha desplaza o anima las piernas");
        p.actualizar(1.1f);
        auto anterior = s.posicionPatrulla(npc);
        exigir(glm::length(anterior - base) > .3f && s.caminandoNPC(npc),
               "El cuerpo completo no avanza por la ruta de ida");
        p.actualizar(.1f);
        auto actual = s.posicionPatrulla(npc);
        auto desplazamiento = glm::vec2(actual.x - anterior.x, actual.z - anterior.z);
        float rumboPaso = std::atan2(desplazamiento.y, -desplazamiento.x);
        const auto frenteModelo = glm::vec2(-std::cos(s.orientacionPatrulla(npc)),
                                             std::sin(s.orientacionPatrulla(npc)));
        exigir(glm::abs(std::remainder(rumboPaso - s.orientacionPatrulla(npc), 6.2831853f)) < .15f &&
                   glm::dot(glm::normalize(desplazamiento), frenteModelo) > .98f,
               "El frente del NPC no apunta hacia la marcha de ida");
        const auto puntoIda = actual;
        p.actualizar(3.8f);
        const auto extremo = s.posicionPatrulla(npc);
        const float rumboExtremo = s.orientacionPatrulla(npc);
        exigir(!s.caminandoNPC(npc) && s.factorMarchaNPC(npc) == 0.f,
               "Las piernas no quedan en reposo durante la vuelta");
        p.actualizar(.7f);
        exigir(glm::length(s.posicionPatrulla(npc) - extremo) < .001f,
               "El NPC se desplaza mientras gira en el extremo");
        const float diferenciaGiro = std::abs(std::remainder(
            s.orientacionPatrulla(npc) - rumboExtremo, 6.2831853f));
        exigir(diferenciaGiro > 1.2f && diferenciaGiro < 1.95f,
               "La orientacion no gira gradualmente 180 grados");
        p.actualizar(.7f);
        anterior = s.posicionPatrulla(npc);
        p.actualizar(.1f);
        actual = s.posicionPatrulla(npc);
        desplazamiento = glm::vec2(actual.x - anterior.x, actual.z - anterior.z);
        rumboPaso = std::atan2(desplazamiento.y, -desplazamiento.x);
        const auto frenteRegreso = glm::vec2(-std::cos(s.orientacionPatrulla(npc)),
                                              std::sin(s.orientacionPatrulla(npc)));
        exigir(glm::length(desplazamiento) > .02f &&
                   glm::abs(std::remainder(rumboPaso - s.orientacionPatrulla(npc),
                                           6.2831853f)) < .15f &&
                   glm::dot(glm::normalize(desplazamiento), frenteRegreso) > .98f,
               "El NPC no recorre la ruta de vuelta mirando al frente");
        p.actualizar(3.7f);
        exigir(glm::length(s.posicionPatrulla(npc) - puntoIda) < .001f,
               "El regreso no recorre el mismo arco de la ida en sentido inverso");
        p.actualizar(1.25f);
        exigir(glm::length(s.posicionPatrulla(npc) - base) < .001f &&
                   !s.caminandoNPC(npc) && s.factorMarchaNPC(npc) == 0.f,
               "El NPC no regresa a su base antes de detenerse");
        p.actualizar(14.f - s.relojPatrulla(npc) - .05f);
        exigir(s.marchaActiva(npc) && s.relojPatrulla(npc) > 13.9f &&
                   s.relojPatrulla(npc) < 14.f,
               "El NPC termina antes de completar su temporizador de 14 s");
        p.actualizar(.1f);
        exigir(!s.marchaActiva(npc) && s.relojPatrulla(npc) == 0.f &&
                   glm::length(s.posicionPatrulla(npc) - base) < .001f &&
                   std::abs(std::remainder(s.orientacionPatrulla(npc) - rumboBase,
                                           6.2831853f)) < .001f,
               "El NPC no finaliza en su base, quieto y mirando al frente");
    };
    for (int npc : {0, 1, 3})
        verificarCiclo(npc);

    // Capturas del mismo Grunt 1 en dos fases opuestas y de vuelta en reposo.
    s.reiniciarPatrullas();
    s.faseInicial = .25f;
    s.tiempo = 0.f;
    OrdenActividad comenzar;
    comenzar.avatar = s.posicionPatrulla(0);
    comenzar.caminarGrunt1 = true;
    p.controlar(comenzar);
    constexpr float ciclosGrunt = 24.f;
    const float periodoGrunt = s.periodoPatrulla(0);
    auto capturarGrunt = [&](float tiempoMarcha, const char *nombre, int signoEsperado) {
        s.reiniciarPatrullas();
        p.controlar(comenzar);
        const float t = .6f + tiempoMarcha;
        p.actualizar(t);
        const auto pos = s.posicionPatrulla(0);
        const float fase = 6.28318530718f * ciclosGrunt * s.tiempoPasoPatrulla(0) /
                           periodoGrunt;
        const float angulo = .35f * s.factorMarchaNPC(0) * std::sin(fase);
        exigir(s.caminandoNPC(0) && std::abs(angulo) > .1f &&
                   (signoEsperado > 0 ? angulo > 0 : angulo < 0),
               "La pose de marcha no alcanza la fase de piernas solicitada");
        const float rumbo = s.orientacionPatrulla(0);
        const glm::vec3 frente(-std::cos(rumbo), 0, std::sin(rumbo));
        const glm::vec3 lateral(std::sin(rumbo), 0, std::cos(rumbo));
        const auto centro = pos + glm::vec3(0, 1.15f, 0);
        const auto ojo = centro + frente * 1.4f + lateral * 4.3f + glm::vec3(0, .6f, 0);
        vista(nombre, ojo, centro, 0, 48.f);
        std::cout << "P1_GRUNT1_CAPTURA " << nombre << " pos=" << pos.x << ',' << pos.y << ','
                  << pos.z << " timer=" << s.relojPatrulla(0) << " piernas=" << angulo << ','
                  << -angulo << " rad\n";
    };
    capturarGrunt(periodoGrunt / (4.f * ciclosGrunt), "C_P1_Grunt1_Caminando_A", 1);
    capturarGrunt(3.f * periodoGrunt / (4.f * ciclosGrunt), "C_P1_Grunt1_Caminando_B", -1);
    s.reiniciarPatrullas();
    p.controlar(comenzar);
    p.actualizar(14.f);
    exigir(!s.marchaActiva(0) && s.factorMarchaNPC(0) == 0.f &&
               glm::length(s.posicionPatrulla(0) - glm::vec3(-77,
                   TerrenoPlaneta1::altura(-77, 153), 153)) < .001f,
           "La captura final no muestra al mismo Grunt 1 quieto en la base");
    const auto baseGrunt = s.posicionPatrulla(0);
    const float rumboBaseGrunt = s.orientacionPatrulla(0);
    const glm::vec3 frenteBase(-std::cos(rumboBaseGrunt), 0, std::sin(rumboBaseGrunt));
    const glm::vec3 lateralBase(std::sin(rumboBaseGrunt), 0, std::cos(rumboBaseGrunt));
    const auto centroBase = baseGrunt + glm::vec3(0, 1.15f, 0);
    const auto ojoBase = centroBase + frenteBase * 1.4f + lateralBase * 4.3f + glm::vec3(0, .6f, 0);
    vista("C_P1_Grunt1_Base", ojoBase, centroBase, 0, 48.f);
    std::cout << "P1_MARCHA_OK G/K/H pulsacion_unica; Grunt3 quieto; temporizadores independientes; "
                 "ida, giro, regreso a base y reposo 14s\n";

    s = original;
    s.holocron = 0;
    auto holocronInicio = glm::vec3(s.matrizHolocron()[3]);
    s.actualizar(5.f);
    auto holocronAlto = glm::vec3(s.matrizHolocron()[3]);
    exigir(holocronAlto.y > holocronInicio.y + 7.f, "Keyframes del holocron no elevan el modelo");
    s.actualizar(7.f);
    exigir(s.holocron < 0 &&
               glm::length(glm::vec3(s.matrizHolocron()[3]) - holocronInicio) < .001f,
           "Keyframes del holocron no regresan a la base");
    s = original;
    vista("C_NPC", {-46, 13, 188}, {-65, 3, 158});
    std::cout << "C_ANIMACIONES_OK basicas(3): puerta/G1/G2; continuas(3): Engineer/Sentinel1/"
                 "Sentinel2; keyframes(2): SaturnV/Holocron; G/K/H ciclos de marcha activados\n";
    OrdenActividad orden;
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
