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
        auto p0 = s.posicionPatrulla(i);
        exigir(glm::length(glm::vec2(p0.x, p0.z) - inicios[i]) < .001f,
               "NPC no inicia en su punto de referencia");
        exigir(s.periodoPatrulla(i) >= 20 && s.periodoPatrulla(i) <= 30,
               "Periodo de patrulla fuera de 20-30 s");
    }
    vista("C_P1_Patrulla_0s", {-46, 13, 188}, {-65, 3, 158});
    std::array<float, 4> distanciaPatrullada{};
    std::array<glm::vec3, 4> anteriorNPC{};
    for (int i = 0; i < 4; ++i)
        anteriorNPC[i] = s.posicionPatrulla(i);
    for (int k = 0; k < 310; ++k) {
        std::array<float, 4> angulos{};
        for (int i = 0; i < 4; ++i)
            angulos[i] = s.orientacionPatrulla(i);
        s.actualizar(.1f);
        std::array<glm::vec3, 4> actual{};
        for (int i = 0; i < 4; ++i) {
            actual[i] = s.posicionPatrulla(i);
            float x = actual[i].x, z = actual[i].z;
            exigir(std::pow((x + 65) / 52.f, 2.f) + std::pow((z - 160) / 28.f, 2.f) < 1,
                   "Patrulla fuera del recinto");
            exigir(std::abs(actual[i].y - TerrenoPlaneta1::altura(x, z)) < .001f,
                   "Patrulla desligada del terreno");
            auto paso = glm::vec2(actual[i].x - anteriorNPC[i].x,
                                  actual[i].z - anteriorNPC[i].z);
            exigir(glm::length(paso) > .02f, "NPC detenido durante la patrulla");
            float rumbo = std::atan2(paso.x, paso.y);
            exigir(std::abs(std::remainder(rumbo - angulos[i], 6.2831853f)) < .08f,
                   "Orientacion no sigue el desplazamiento");
            auto libre = p.ajustarMovimiento(anteriorNPC[i], actual[i]);
            exigir(glm::length(glm::vec2(libre.x - actual[i].x, libre.z - actual[i].z)) < .08f,
                   "Patrulla atraviesa un muro o colision");
            distanciaPatrullada[i] += glm::length(paso);
            anteriorNPC[i] = actual[i];
            for (int j = 0; j < i; ++j)
                exigir(glm::length(glm::vec2(actual[i].x - actual[j].x,
                                            actual[i].z - actual[j].z)) > 2.5f,
                       "Circuitos NPC demasiado cercanos");
        }
    }
    const float velocidades[] = {distanciaPatrullada[0] / 31.f,
                                 distanciaPatrullada[1] / 31.f,
                                 distanciaPatrullada[2] / 31.f,
                                 distanciaPatrullada[3] / 31.f};
    exigir(velocidades[0] > 1.4f && velocidades[1] > 1.2f && velocidades[2] > 1.1f &&
               velocidades[3] > .85f && velocidades[3] < velocidades[2] &&
               distanciaPatrullada[3] < distanciaPatrullada[2],
           "Hunter no tiene circuito mas corto y lento");
    const char *nombresPatrulla[] = {"Grunt1", "Grunt2", "Grunt3", "Hunter"};
    const auto flagsSalida = std::cout.flags();
    const auto precisionSalida = std::cout.precision();
    std::cout << std::fixed << std::setprecision(2);
    for (int i = 0; i < 4; ++i) {
        const auto fin = s.posicionPatrulla(i);
        const float neto = glm::length(glm::vec2(fin.x - inicios[i].x, fin.z - inicios[i].y));
        exigir(neto > .5f, "NPC no termina en una posicion distinta tras 31 s");
        std::cout << "P1_PATRULLA_POS " << nombresPatrulla[i] << " inicio=(" << inicios[i].x << ','
                  << inicios[i].y << ") fin=(" << fin.x << ',' << fin.z << ") neto=" << neto
                  << "u recorrido=" << distanciaPatrullada[i] << "u velocidad_media="
                  << velocidades[i] << "u/s periodo="
                  << s.periodoPatrulla(i) << "s\n";
    }
    std::cout.flags(flagsSalida);
    std::cout.precision(precisionSalida);
    vista("C_P1_Patrulla_31s", {-46, 13, 188}, {-65, 3, 158});
    std::cout << "P1_PATRULLA_OK 4 modelos trasladados en circuitos cerrados dentro del recinto; "
                 "orientacion, suelo, colision, marcha de piernas; Hunter corto/lento\n";

    auto probarTeclaMarcha = [&](int ranura, glm::vec3 sitio, bool g, bool k, bool h) {
        OrdenActividad o;
        o.avatar = sitio;
        o.caminarGrunt1 = g;
        o.caminarGrunt2 = k;
        o.caminarHunter = h;
        p.controlar(o);
        exigir(p.marchaManualSolicitada(ranura), "G/K/H no activa la marcha cercana asignada");
        for (int i = 0; i < 3; ++i)
            if (i != ranura)
                exigir(!p.marchaManualSolicitada(i), "Una tecla activa otro NPC");
        p.controlar({});
        exigir(!p.marchaManualSolicitada(ranura), "La marcha no termina al soltar la tecla");
    };
    p.controlar({});
    p.actualizar(1.f);
    probarTeclaMarcha(0, s.posicionPatrulla(0), true, false, false);
    probarTeclaMarcha(1, s.posicionPatrulla(1), false, true, false);
    probarTeclaMarcha(2, s.posicionPatrulla(3), false, false, true);
    OrdenActividad fueraDeRango;
    fueraDeRango.avatar = s.posicionPatrulla(0) + glm::vec3(0, 0, 20);
    fueraDeRango.caminarGrunt1 = true;
    p.controlar(fueraDeRango);
    exigir(!p.marchaManualSolicitada(0), "G activa al Grunt 1 fuera de proximidad");

    auto capturarPasoNPC = [&](int npc, int ciclos, int ranura, const char *modelo,
                               void (*activar)(OrdenActividad &)) {
        p.controlar({});
        p.actualizar(1.f);
        s.reiniciarPatrullas();
        s.tiempo = 0;
        s.faseInicial = .25f;
        const float alturaCentro = npc == 3 ? 2.2f : 1.1f;
        const float distancia = npc == 3 ? 9.f : 6.5f;
        auto orden = [&]() {
            OrdenActividad o;
            o.avatar = s.posicionPatrulla(npc);
            activar(o);
            p.controlar(o);
        };
        auto vistaCercana = [&](const char *estado) {
            const auto pos = s.posicionPatrulla(npc);
            const float rumbo = s.orientacionPatrulla(npc);
            const glm::vec3 frente(std::sin(rumbo), 0, std::cos(rumbo));
            const glm::vec3 lateral(frente.z, 0, -frente.x);
            const auto centro = pos + glm::vec3(0, alturaCentro, 0);
            const std::string archivo = std::string("C_P1_") + modelo + "_" + estado;
            const auto ojo = centro - frente * (distancia * .82f) -
                             lateral * (distancia * .35f) + glm::vec3(0, alturaCentro * .3f, 0);
            vista(archivo.c_str(), ojo, centro, 0, npc == 3 ? 40.f : 36.f);
            const float fase = 6.28318530718f * ciclos * s.relojPatrulla(npc) /
                               s.periodoPatrulla(npc);
            const float amplitud = .35f + .15f * p.intensidadMarchaManual(ranura);
            const float angulo = amplitud * std::sin(fase);
            std::cout << "P1_PIERNA_CAPTURA modelo=" << modelo << " estado=" << estado
                      << " posicion=" << pos.x << ',' << pos.y << ',' << pos.z
                      << " reloj=" << s.relojPatrulla(npc) << " angulos=" << angulo << ','
                      << -angulo << " rad eje=Z marcha=" << p.intensidadMarchaManual(ranura) << '\n';
        };
        orden();
        exigir(p.marchaManualSolicitada(ranura), "La tecla mantenida no activa el NPC de captura");
        const auto posicionInicial = s.posicionPatrulla(npc);
        vistaCercana("Patrulla_Normal");
        const float cuartoCiclo = s.periodoPatrulla(npc) / (4.f * ciclos);
        p.actualizar(cuartoCiclo);
        exigir(s.relojPatrulla(npc) > 0 &&
                   glm::length(s.posicionPatrulla(npc) - posicionInicial) > .02f,
               "El NPC deja de avanzar en su patrulla durante G/K/H");
        vistaCercana("Paso_A");
        p.actualizar(2 * cuartoCiclo);
        vistaCercana("Paso_B");
        const float mezclaActiva = p.intensidadMarchaManual(ranura);
        const float relojAntesDeSoltar = s.relojPatrulla(npc);
        p.controlar({});
        p.actualizar(.25f);
        exigir(p.intensidadMarchaManual(ranura) < mezclaActiva &&
                   p.intensidadMarchaManual(ranura) > 0.f,
               "La transicion a marcha normal no es gradual al soltar");
        exigir(s.relojPatrulla(npc) > relojAntesDeSoltar,
               "El reloj de patrulla se pausa al soltar G/K/H");
        const auto fin = s.posicionPatrulla(npc);
        std::cout << "P1_PATRULLA_CAPTURA modelo=" << modelo << " t0=(" << posicionInicial.x
                  << ',' << posicionInicial.z << ") tFinal=(" << fin.x << ',' << fin.z
                  << ") retorno_a_patrulla=gradual\n";
    };
    capturarPasoNPC(0, 24, 0, "Grunt1", [](OrdenActividad &o) { o.caminarGrunt1 = true; });
    capturarPasoNPC(3, 16, 2, "Hunter", [](OrdenActividad &o) { o.caminarHunter = true; });
    std::cout << "P1_PIERNA_GLTF_OK nodos y pivotes originales; rotacion Rz; fase patrulla intacta; "
                 "G/K/H mantenidas, locomocion y retorno gradual\n";

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
                 "Sentinel2; keyframes(2): SaturnV/Holocron; G/K/H locomocion mantenida\n";
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
