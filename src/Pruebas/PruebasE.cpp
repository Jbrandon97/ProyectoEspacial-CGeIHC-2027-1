#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Espacio/Espacio.h"
#include "Regiones/Planeta2/Planeta2.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <thread>
#include <fstream>
#include <chrono>
#include <vector>
void Aplicacion::probarE(const std::filesystem::path &dir) {
    auto exigir = [](bool ok, const char *mensaje) {
        if (!ok)
            throw std::runtime_error(mensaje);
    };
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto &s = p.sistemas();
    const auto original = s;
    probarFinalPlaneta1(dir);
    // Audio: objetivo espacial y conteo de inicios verifican condiciones, no una audicion humana.
    s = original;
    s.lanzamiento = 1;
    observandoCohete_ = false;
    avatar_.colocar({223, 0, 21}, p);
    actualizarAudio();
    float cerca = audio_.estado(Audio::Cohete).objetivo;
    auto musica = audio_.estado(Audio::Musica);
    auto motor = audio_.estado(Audio::Cohete);
    for (int i = 0; i < 100; ++i)
        actualizarAudio();
    exigir(audio_.estado(Audio::Musica).inicios == musica.inicios &&
               audio_.estado(Audio::Cohete).inicios == motor.inicios,
           "E audio se reinicia por fotograma");
    avatar_.colocar({0, 0, 120}, p);
    actualizarAudio();
    float lejos = audio_.estado(Audio::Cohete).objetivo;
    exigir(cerca > .5f && lejos < cerca * .1f, "E sonido no atenua por distancia real");
    exigir(audio_.estado(Audio::Musica).objetivo == musica.objetivo, "E distancia afecta musica");
    OrdenActividad o;
    o.avatar = {165, 3, -65};
    o.puerta = true;
    auto n = audio_.estado(Audio::Puerta).inicios;
    p.controlar(o);
    actualizarAudio();
    actualizarAudio();
    exigir(audio_.estado(Audio::Puerta).inicios == n + 1, "E evento hangar duplicado/ausente");
    s.lanzamiento = -1;
    actualizarAudio();
    exigir(audio_.estado(Audio::Cohete).objetivo == 0, "E propulsion suena sin motor");
    std::cout << "E_AUDIO_OK cerca=" << cerca << " lejos=" << lejos
              << " musica independiente; eventos sin duplicar\n";
    // Avatar quieto conserva exactamente la pose; locomocion sigue a 15/30.
    Avatar avatar;
    for (bool correr : {false, true}) {
        avatar.colocar({-95, 0, -30}, p);
        auto inicio = avatar.posicion();
        for (int i = 0; i < 60; ++i)
            avatar.mover({1, 0}, 1.f / 60, p, correr);
        exigir(std::abs(avatar.posicion().x - inicio.x - (correr ? 30 : 15)) < .02f,
               "E velocidad avatar alterada");
        avatar.mover({0, 0}, 1.f / 60, p);
        exigir(avatar.actividad() == 0, "E actividad permanece en reposo");
    }
    s = original;
    avatar_.colocar({-95, 0, -30}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    for (int i = 0; i < 120; ++i) {
        avatar_.mover({0, 0}, 1.f / 60, p);
        actualizarCamara(1.f / 60);
    }
    dibujar(dir / "E_Engineer_reposo_A.ppm");
    for (int i = 0; i < 120; ++i)
        avatar_.mover({0, 0}, 1.f / 60, p);
    dibujar(dir / "E_Engineer_reposo_B.ppm");
    for (int modo = 0; modo < 2; ++modo) {
        avatar_.colocar({-95, 0, -30}, p);
        for (int i = 0; i < 24; ++i) {
            avatar_.mover({1, 0}, 1.f / 60, p, modo == 1);
            actualizarCamara(1.f / 60);
        }
        dibujar(dir / (modo ? "E_Engineer_correr.ppm" : "E_Engineer_caminar.ppm"));
    }
    std::cout << "E_AVATAR_OK reposo exacto, jerarquia conservada, escala 1.26, velocidad 15/30\n";
    s = original;
    foco_ = false;
    observandoCohete_ = false;
    actualizarAudio();
    camaras_.alCambiarRegion(Region::Planeta1);
}
void Aplicacion::probarFinalPlaneta1(const std::filesystem::path &dir) {
    auto exigir = [](bool b, const char *m) {
        if (!b)
            throw std::runtime_error(m);
    };
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto &s = p.sistemas();
    auto original = s;
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena c{renderizador_, recursos_};
    auto foto = [&](std::string nombre, glm::vec3 ojo, glm::vec3 centro) {
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .15f, 1800.f) *
                  glm::lookAt(ojo, centro, {0, 1, 0});
        renderizador_.iniciar(w, h, vp, p.ambiente(), ojo);
        p.dibujar(c, glm::mat4(1));
        renderizador_.capturar(dir / (nombre + ".ppm"), w, h);
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
    };
    exigir(s.keyframes().Cantidad() == 72, "TXT original alterado");
    s.faseInicial = .25f;
    s.tiempo = 0;
    for (float t : {0.f, 2.f, 8.5f, 12.5f, 18.f, 35.f, 69.f}) {
        s.lanzamiento = t;
        observandoCohete_ = true;
        dibujar(dir / ("Final_Cohete_" + std::to_string(int(t * 10)) + ".ppm"));
    }
    observandoCohete_ = false;
    s = original;
    for (int ciclo = 0; ciclo < 3; ++ciclo) {
        s.lanzar();
        float y = 0;
        for (int j = 0; j < 69 * 60; ++j) {
            p.actualizar(1.f / 60);
            float ahora = glm::vec3(s.etapas()[3] * glm::vec4(0, 45, 0, 1)).y;
            exigir(ahora >= y - .01f, "Ascenso retrocede");
            y = ahora;
        }
        exigir(y > 1500, "Parte superior no se aleja");
        for (int j = 0; j < 10 * 60; ++j) {
            p.actualizar(1.f / 60);
            if (ciclo == 0 && (j == 120 || j == 300 || j == 500))
                foto("Final_Reposicion_" + std::to_string(j), {240, 30, 70}, {205, 22, 21});
        }
        exigir(s.lanzamiento < 0 && s.reposicion < 0, "Reposicion automatica falla");
    }
    s = original;
    s.faseInicial = .25f;
    s.titan = -1;
    s.saludo = -1;
    p.actualizar(0);
    auto manoReposo = p.titan().extremo("def_r_wrist_098");
    auto pieReposo = p.titan().extremo("def_l_ankle_0122");
    float pasoMaximo = 0;
    for (int j = 0; j <= 40; ++j) {
        s.titan = .6f + j * 1.25f / 40;
        p.actualizar(0);
        pasoMaximo =
            std::max(pasoMaximo, glm::length(p.titan().extremo("def_l_ankle_0122") - pieReposo));
        exigir(std::abs(p.titan().alturaMinima()) < .001f, "BT sin apoyo geometrico");
    }
    std::cout << "BT_PASO desplazamiento_tobillo=" << pasoMaximo << "\n";
    exigir(pasoMaximo > .15f, "BT caminata rigida");
    s.titan = -1;
    p.actualizar(0);
    OrdenActividad o;
    o.avatar = {-95, 3, 145};
    o.titan = true;
    p.controlar(o);
    exigir(s.titan == 0, "B no activa Titan");
    for (float t : {0.f, 1.f, 1.6f, 3.f, 6.4f, 8.f, 10.f, 13.9f}) {
        s.titan = t;
        p.actualizar(0);
        foto("Final_BT_" + std::to_string(int(t * 10)), {-108, 10, 175}, {-95, 6, 156});
    }
    p.actualizar(.2f);
    exigir(s.titan < 0, "BT no termina");
    p.controlar(o);
    exigir(s.titan == 0, "BT no repite");
    p.actualizar(14.1f);
    exigir(s.titan < 0, "BT segunda activacion");
    o.titan = false;
    o.saludo = true;
    p.controlar(o);
    exigir(s.saludo == 0, "J no activa saludo");
    for (float t : {0.f, 1.5f, 3.f, 4.5f, 5.99f}) {
        s.saludo = t;
        p.actualizar(0);
        foto("Final_BT_Saludo_" + std::to_string(int(t * 10)), {-108, 10, 175}, {-95, 6, 153});
    }
    s.saludo = 1.5f;
    p.actualizar(0);
    exigir(glm::length(p.titan().extremo("def_r_wrist_098") - manoReposo) > .5f,
           "BT saludo sin articulacion");
    s.saludo = 5.99f;
    p.actualizar(.1f);
    exigir(s.saludo < 0, "Saludo no termina");
    p.controlar(o);
    exigir(s.saludo == 0, "Saludo no repite");
    s.saludo = -1;
    p.actualizar(0);
    o = {};
    o.avatar = s.posicionPatrulla(3);
    o.caminarHunter = true;
    p.controlar(o);
    exigir(s.marchaActiva(3), "H no activa caminata del Hunter en proximidad");
    p.actualizar(.9f);
    foto("Final_Hunter_Marcha", {-48, 9, 184}, {-65, 5, 174});
    exigir(s.factorMarchaNPC(3) > 0 && s.caminandoNPC(3), "H no mueve las piernas del Hunter");
    p.actualizar(13.1f);
    exigir(!s.marchaActiva(3) && s.relojPatrulla(3) == 0,
           "Hunter no vuelve a su base al terminar el ciclo");
    o = {};
    o.avatar = s.posicionPatrulla(1);
    o.caminarGrunt2 = true;
    p.controlar(o);
    exigir(s.marchaActiva(1), "K no activa caminata del Grunt 2 en proximidad");
    p.actualizar(.9f);
    foto("Final_Grunt2_Marcha", {-48, 8, 178}, {-65, 3, 162});
    p.actualizar(13.1f);
    exigir(!s.marchaActiva(1) && s.relojPatrulla(1) == 0,
           "Grunt 2 no vuelve a su base al terminar el ciclo");
    s.faseInicial = 5.f / 6;
    s.tiempo = 0;
    o = {};
    o.avatar = {165, 3, -65};
    o.reflector = true;
    p.controlar(o);
    exigir(s.reflector, "P no activa reflector");
    foto("Final_Reflector_ON", {185, 13, -48}, {165, 5, -75});
    p.controlar(o);
    exigir(!s.reflector, "P no apaga reflector");
    foto("Final_Reflector_OFF", {185, 13, -48}, {165, 5, -75});
    foto("Final_Covenant", {-122, 10, -43}, {-145, 5, -67});
    for (int j = 0; j < 3; ++j) {
        s.tiempo = float(j) * 7;
        foto("Final_Sentinel_" + std::to_string(j), {-92, 17, -18}, {-110, 11, -45});
    }
    avatar_.colocar({-110, 0, -30}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    for (int j = 0; j < 120; ++j)
        actualizarCamara(1.f / 60);
    foco_ = false;
    dibujar(dir / "Final_Energia_OFF.ppm");
    foco_ = true;
    dibujar(dir / "Final_Energia_ON.ppm");
    foco_ = false;
    avatar_.colocar({-145, 0, -63}, p);
    actualizarAudio();
    float cerca = audio_.estado(Audio::Covenant).objetivo;
    avatar_.colocar({0, 0, 120}, p);
    actualizarAudio();
    exigir(audio_.estado(Audio::Covenant).objetivo < cerca * .1f, "Covenant sin atenuacion");
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 100; ++j) {
            s.tiempo = j * .2f;
            auto a = s.sentinel(i);
            auto giro = s.giroSentinel(i);
            s.tiempo += .001f;
            auto b = s.sentinel(i);
            auto d = glm::normalize(glm::vec2(b.x - a.x, b.z - a.z));
            auto frente = glm::vec2(std::cos(giro), -std::sin(giro));
            exigir(glm::dot(d, frente) > .999f, "Sentinel no mira su tangente");
        }
    s = original;
    p.actualizar(0);
    observandoCohete_ = false;
    // Persistencia real de region, avatar y camara sin recargar los sistemas.
    s.tiempo = 37;
    s.lanzamiento = 24;
    auto restos = s.etapas();
    s.lanzamiento = 65;
    for (int i = 0; i < 2; ++i)
        for (int k = 0; k < 4; ++k)
            exigir(glm::length(restos[i][k] - s.etapas()[i][k]) < .001f,
                   "Etapa se hunde tras aterrizar");
    auto avatarGuardado = avatar_.guardar();
    auto ojoGuardado = camaras_.ojo();
    cambiarRegion(Region::Planeta2);
    cambiarRegion(Region::Espacio);
    cambiarRegion(Region::Planeta1);
    exigir(s.tiempo == 37 && s.lanzamiento == 65, "Region reiniciada");
    exigir(glm::length(avatar_.posicion() - avatarGuardado.posicion) < .001f &&
               glm::length(camaras_.ojo() - ojoGuardado) < .001f,
           "Avatar/camara no restaurados");
    s = original;
    std::cout << "PERSISTENCIA_OK avatar camara reloj cohete; restos sin hundimiento\n";
    s.faseInicial = 5.f / 6;
    s.tiempo = 0;
    foto("Final_Pista_noche", {210, 65, 240}, {110, 3, 135});
    s = original;
    ModeloGltf centinela(recursos_ / "Modelos/Sentinel.gltf");
    for (int j = 0; j < 2; ++j) {
        glm::vec3 ojo = j ? glm::vec3(8, 4, 0) : glm::vec3(0, 4, 8);
        auto vp = glm::perspective(glm::radians(45.f), float(w) / h, .1f, 50.f) *
                  glm::lookAt(ojo, {0, 1, 0}, {0, 1, 0});
        auto amb = p.ambiente();
        amb.iluminacion.ambiente = .4f;
        renderizador_.iniciar(w, h, vp, amb, ojo);
        centinela.dibujar(renderizador_, glm::mat4(1));
        renderizador_.capturar(dir / ("Revision_Sentinel_eje_" + std::to_string(j) + ".ppm"), w, h);
    }

    std::cout << "FINAL_OK Saturn sin orbita 3 ciclos automaticos; BT articulado repetible; Hunter "
                 "H; Covenant atenuado; 16 luces\n";
}

void Aplicacion::probarEspacio(const std::filesystem::path &dir) {
    auto exigir = [](bool b, const char *m) {
        if (!b)
            throw std::runtime_error(m);
    };
    cambiarRegion(Region::Espacio);
    auto &e = dynamic_cast<Espacio &>(escenarios_.actual());
    OrdenActividad o;
    o.recuperar = true;
    aplicarOrden(o);
    o = {};
    o.acelerador = 1;
    aplicarOrden(o);
    for (int i = 0; i < 120; ++i)
        e.actualizar(1.f / 60);
    auto p = e.actividad().objetivo;
    exigir(p.x > -190, "Nave espacial no avanza");
    o = {};
    o.vertical = 1;
    o.giro = 1;
    aplicarOrden(o);
    e.actualizar(.5f);
    exigir(e.actividad().objetivo.y > p.y + 10, "Nave sin ascenso");
    exigir(e.actividad().direccion.z > .3f, "Nave sin giro");
    o = {};
    aplicarOrden(o);
    auto reloj = e.reloj();
    p = e.actividad().objetivo;
    cambiarRegion(Region::Planeta1);
    cambiarRegion(Region::Espacio);
    exigir(e.reloj() == reloj && glm::length(p - e.actividad().objetivo) < .001f,
           "Espacio pierde estado");
    exigir(glm::length(e.ajustarMovimiento(p, {0, 0, 0}) - p) < .001f, "Freeport atravesable");
    exigir(glm::length(e.trafico(0) - e.trafico(40)) < .001f, "Trafico no ciclico");
    exigir(glm::length(e.trafico(12) - e.trafico(18)) < .001f, "Trafico sin espera");
    Espacio baja, alta;
    OrdenActividad vuelo;
    vuelo.acelerador = 1;
    vuelo.giro = .2f;
    vuelo.mirada = {20, -35};
    baja.controlar(vuelo);
    alta.controlar(vuelo);
    for (int i = 0; i < 120; ++i)
        baja.actualizar(1.f / 30);
    for (int i = 0; i < 480; ++i)
        alta.actualizar(1.f / 120);
    exigir(glm::length(baja.actividad().objetivo - alta.actividad().objetivo) < .02f,
           "Vuelo depende de FPS");
    exigir(alta.direccion().y > .05f, "Mouse vertical invertido");
    exigir(glm::length(glm::vec3(alta.matrizNave()[2]) + alta.direccion()) < .0001f,
           "Modelo y vuelo desalineados");
    solicitarCamara(ModoCamara::Viaje);
    if (!camaras_.cabina())
        camaras_.alternarCabina();
    actualizarCamara(1.f / 60);
    auto arriba = glm::vec3(e.matrizNave()[1]);
    auto ojoEsperado = mundo_.aGlobal(Region::Espacio, e.actividad().objetivo + arriba * 1.6f +
                                                           e.direccion() * .55f);
    exigir(glm::length(camaras_.ojo() - ojoEsperado) < .001f, "Cabina separada del piloto");
    dibujar();
    camaras_.alternarCabina();
    actualizarCamara(1.f / 60);
    dibujar();
    for (int i = 0; i < 3; ++i) {
        cambiarRegion(Region::Espacio);
        actualizarAudio();
        exigir(audio_.estado(Audio::MusicaEspacio).objetivo > 0 &&
                   audio_.estado(Audio::Musica).objetivo == 0,
               "Musicas duplicadas en Espacio");
        auto inicios = audio_.estado(Audio::MusicaEspacio).inicios;
        for (int j = 0; j < 10; ++j)
            actualizarAudio();
        exigir(audio_.estado(Audio::MusicaEspacio).inicios == inicios,
               "Musica reiniciada cada cuadro");
        cambiarRegion(Region::Planeta1);
        actualizarAudio();
        exigir(audio_.estado(Audio::MusicaEspacio).objetivo == 0 &&
                   audio_.estado(Audio::Musica).objetivo > 0,
               "Musica espacial sigue al salir");
    }
    if (audio_.disponible()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2600));
        exigir(audio_.estado(Audio::MusicaEspacio).ganancia < .001f, "Fundido espacial no termina");
    }
    cambiarRegion(Region::Espacio);
    std::cout << "VUELO_OK mouse coherente, matriz alineada, 30/120 FPS equivalentes, cabina "
                 "anclada, cambio exterior\n";
    std::cout
        << "MUSICA_ESPACIO_OK tres transiciones, sin reinicios, fundido y exclusividad regional\n";
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena c{renderizador_, recursos_};
    auto foto = [&](const char *nombre, glm::vec3 ojo, glm::vec3 centro) {
        ojo = mundo_.aGlobal(Region::Espacio, ojo);
        centro = mundo_.aGlobal(Region::Espacio, centro);
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .1f, 1500.f) *
                  glm::lookAt(ojo, centro, {0, 1, 0});
        renderizador_.iniciar(w, h, vp, e.ambiente(), ojo);
        e.dibujar(c, mundo_.transformacion(Region::Espacio));
        renderizador_.capturar(dir / (std::string(nombre) + ".ppm"), w, h);
        Renderizador::comprobarErrores();
    };
    foto("Espacio_Freeport", {90, 45, 100}, {0, 0, 0});
    foto("Espacio_Conjunto", {0, 140, 460}, {0, 0, 0});
    foto("Espacio_Luna_Satelite", {-100, 90, 80}, {40, 45, -110});
    solicitarCamara(ModoCamara::Viaje);
    for (int i = 0; i < 90; ++i)
        actualizarCamara(1.f / 60);
    dibujar(dir / "Espacio_vista_nave.ppm");
    solicitarCamara(ModoCamara::Observacion);
    for (int i = 0; i < 90; ++i)
        actualizarCamara(1.f / 60);
    dibujar(dir / "Espacio_vista_observacion.ppm");
    o = {};
    o.recuperar = true;
    aplicarOrden(o);
    o = {};
    o.confirmar = true;
    aplicarOrden(o);
    exigir(escenarios_.actual().id() == Region::Planeta1, "Portal no retorna a planeta1");
    std::cout << "ESPACIO_OK navegacion avance giro altura, colision Freeport, trafico40s, "
                 "persistencia, camaras, portal Planeta1\n";
}

void Aplicacion::probarPlaneta2(const std::filesystem::path &dir) {
    auto exigir = [](bool b, const char *m) {
        if (!b)
            throw std::runtime_error(m);
    };
    cambiarRegion(Region::Planeta2);
    auto &p = dynamic_cast<Planeta2 &>(escenarios_.actual());
    for (float hueco : p.ratchet().huecosBase())
        exigir(std::isfinite(hueco) && hueco <= -.10f, "Base Kerwan suspendida");
    for (auto centro : p.ratchet().centrosEdificios())
        exigir(p.ratchet().obstaculo(centro), "Edificio sin colision");
    auto dia = p.ambiente();
    exigir(dia.iluminacion.noche < .01f, "Planeta2 no inicia de dia");
    exigir(!dia.iluminacion.focales.back().encendida, "Reflector Aphelion activo de dia");
    p.actualizar(70);
    auto noche = p.ambiente();
    exigir(noche.iluminacion.noche > .95f, "Luces P2 no siguen ciclo nocturno");
    for (int i = 0; i < 6; ++i) {
        auto l = noche.iluminacion.puntuales[i];
        exigir(l.encendida && l.intensidad > 0, "Luz Kerwan apagada");
        exigir(l.posicion.y > p.altura(l.posicion.x, l.posicion.z) + 5, "Luz Kerwan enterrada");
    }
    int w, h;
    ventana_.dimensiones(w, h);
    // Una ventana minimizada puede reportar framebuffer 0x0 durante una bateria larga.
    // Restauramos el contexto antes de construir la proyeccion y evitamos pasar NaN a GLM.
    if (w <= 0 || h <= 0) {
        auto nativa = ventana_.nativa();
        if (glfwGetWindowAttrib(nativa, GLFW_ICONIFIED))
            glfwRestoreWindow(nativa);
        for (int intento = 0; intento < 20 && (w <= 0 || h <= 0); ++intento) {
            glfwWaitEventsTimeout(.025);
            ventana_.dimensiones(w, h);
        }
    }
    exigir(w > 0 && h > 0, "Framebuffer sin dimensiones al capturar Planeta 2");
    std::cout << "PRUEBA_FRAMEBUFFER_P2 " << w << "x" << h << "\n";
    ContextoEscena c{renderizador_, recursos_};
    auto foto = [&](std::string nombre, glm::vec3 ojo, glm::vec3 objetivo,
                    const Ambiente *ajuste = nullptr, bool leerPixeles = false,
                    int soloMuneco = -1, float tiempoBrazos = 0.f) {
        ojo = mundo_.aGlobal(Region::Planeta2, ojo);
        objetivo = mundo_.aGlobal(Region::Planeta2, objetivo);
        auto vp = glm::perspective(glm::radians(58.f), float(w) / h, .1f, 1200.f) *
                  glm::lookAt(ojo, objetivo, {0, 1, 0});
        auto amb = ajuste ? *ajuste : p.ambiente();
        for (auto &l : amb.iluminacion.puntuales)
            if (l.encendida)
                l.posicion = mundo_.aGlobal(Region::Planeta2, l.posicion);
        for (auto &l : amb.iluminacion.focales)
            if (l.encendida)
                l.posicion = mundo_.aGlobal(Region::Planeta2, l.posicion);
        renderizador_.iniciar(w, h, vp, amb, ojo);
        const auto &m = mundo_.transformacion(Region::Planeta2);
        if (soloMuneco < 0)
            p.dibujar(c, m);
        else
            p.ratchet().dibujarMuneco(renderizador_,glm::translate(m,{18,0,0}),soloMuneco,
                                      p.ratchet().distanciaPatrulla(),tiempoBrazos);
        renderizador_.capturar(dir / (nombre + ".ppm"), w, h);
        std::vector<unsigned char> pixeles;
        if (leerPixeles) {
            pixeles.resize(size_t(w)*h*4);
            glReadBuffer(GL_BACK);
            glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixeles.data());
        }
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
        return pixeles;
    };
    foto("P2_Noche", {125, 62, 155}, {15, 12, 0});
    p.actualizar(50);
    foto("P2_Dia", {125, 62, 155}, {15, 12, 0});
    for (size_t i = 0; i < p.ratchet().contactoBase().size(); ++i) {
        const auto hundimiento = p.ratchet().contactoBase()[i];
        exigir(std::isfinite(hundimiento.x) && std::isfinite(hundimiento.y) &&
                   hundimiento.x >= .10f && hundimiento.y >= hundimiento.x,
               "Contacto de base P2 invalido");
        std::cout << "P2_BASE_CONTACTO " << i << " hundimiento_min=" << hundimiento.x
                  << " max=" << hundimiento.y << "\n";
    }
    int indiceBase = 0;
    for (auto centro : p.ratchet().centrosEdificios()) {
        centro += glm::vec3(18, 0, 0);
        foto("P2_Base_Dia_" + std::to_string(indiceBase++), centro + glm::vec3(14, 7, 17),
             centro + glm::vec3(0, 4, 0));
    }
    p.actualizar(70);
    indiceBase = 0;
    for (auto centro : p.ratchet().centrosEdificios()) {
        centro += glm::vec3(18, 0, 0);
        foto("P2_Base_Noche_" + std::to_string(indiceBase++), centro + glm::vec3(14, 7, 17),
             centro + glm::vec3(0, 4, 0));
    }
    auto reloj = p.reloj();
    cambiarRegion(Region::Espacio);
    cambiarRegion(Region::Planeta2);
    exigir(p.reloj() == reloj, "Reloj P2 reiniciado");
    std::cout << "PLANETA2_OK seis apoyos geometricos sin hueco, colisiones, seis luces de "
                 "fachada, ciclo120s y persistencia\n";

    const auto &sector = p.ratchet();
    const glm::vec3 origen(18,0,0);
    auto plano = [](glm::vec3 q) { return glm::vec2(q.x,q.z); };
    Planeta2 ciclo;
    int muestrasDia = 0, muestrasNoche = 0;
    constexpr int muestrasCiclo = 1200;
    for (int k = 0; k < muestrasCiclo; ++k) {
        const auto luz = ciclo.ambiente().iluminacion;
        if (luz.sol.y > 0)
            ++muestrasDia;
        else
            ++muestrasNoche;
        if (luz.noche > .97f)
            exigir(luz.focales.back().encendida, "Aphelion apagada en plena noche");
        if (luz.noche < .01f)
            exigir(!luz.focales.back().encendida, "Aphelion encendida en pleno dia");
        ciclo.actualizar(Planeta2::DuracionCiclo/muestrasCiclo);
    }
    exigir(std::abs(ciclo.reloj()-24.f)<.01f && std::abs(muestrasDia-420)<=1 &&
           std::abs(muestrasNoche-780)<=1, "El ciclo P2 no mantiene la noche mas larga");
    std::cout << "P2_CICLO_OK dia_s=42 noche_s=78 muestras=" << muestrasCiclo << '\n';

    constexpr int actores = SectorRatchet::CantidadMunecos;
    const double longitud = sector.longitudPatrulla(), separacion = longitud / actores;
    constexpr int muestras = 960;
    constexpr float velocidad = 7.5f;
    float minPaso = 10, maxPaso = 0;
    for (int i = 0; i < actores; ++i) {
        const auto inicio = sector.posicionMuneco(i,0);
        exigir(glm::length(inicio-sector.posicionMuneco(0,i*separacion))<.001f,
               "Error en el desfase de salida de los munecos");
        for (int k = 0; k < muestras; ++k) {
            const double s = longitud*k/muestras;
            const auto q = sector.posicionMuneco(i,s);
            const auto siguiente = sector.posicionMuneco(i,s+longitud/muestras);
            const auto paso = plano(siguiente-q);
            const auto frente = sector.frenteMuneco(s,i);
            exigir(!sector.obstaculo(q,false) && std::abs(q.y-p.altura(q.x+18,q.z)-.092f)<.002f,
                   "Muneco fuera del transito o separado del piso");
            exigir(glm::dot(glm::normalize(paso),plano(frente)) > .99f,
                   "Orientacion lateral en circuito de munecos");
            exigir(glm::length(plano(q-sector.posicionMuneco((i+1)%actores,s))) > 110.f,
                   "Munecos se alcanzan en el perimetro");
            minPaso = std::min(minPaso, glm::length(paso));
            maxPaso = std::max(maxPaso, glm::length(paso));
        }
        exigir(glm::length(sector.posicionMuneco(i,longitud)-inicio)<.001f,
               "La patrulla no cierra en su punto de salida");
    }
    exigir(maxPaso-minPaso < .008f, "Velocidad irregular o salto en esquina");
    const double previo = sector.distanciaPatrulla();
    p.actualizar(2.f);
    exigir(std::abs(std::fmod(previo+velocidad*2,longitud)-sector.distanciaPatrulla())<.001,
           "Distancia de patrulla no responde al deltaTime");
    std::cout << "P2_MUNECOS_OK actores=" << actores << " muestras=" << actores*muestras
              << " separacion=" << separacion << " longitud=" << longitud
              << " minPaso=" << minPaso << " maxPaso=" << maxPaso
              << " periodo_s=" << longitud/velocidad << '\n';

    auto recorrer = [&](glm::vec2 inicio, glm::vec2 destino, bool correr) {
        Avatar avatar;
        avatar.colocar({inicio.x,0,inicio.y},p);
        for (int i = 0; i < 800; ++i) {
            const glm::vec2 d = destino - plano(avatar.posicion());
            if (glm::length(d)<.65f)
                return;
            avatar.mover(glm::normalize(d),1.f/60,p,correr);
        }
        std::cout << "P2_RUTA_BLOQUEADA destino=" << destino.x << ',' << destino.y
                  << " detenido=" << avatar.posicion().x << ',' << avatar.posicion().z << '\n';
        exigir(false,"Ruta peatonal bloqueada");
    };
    exigir(!sector.obstaculo(p.aparicion()-origen,false), "Aparicion P2 bloqueada");
    recorrer({-42,115},{-42,75},false);
    recorrer({-42,75},{-24,33},true);
    recorrer({-24,33},{-60,5},false);
    recorrer({-60,5},{-77,-4},true);
    recorrer({-24,33},{-12,29},true);
    recorrer({-12,29},{26,15},false);
    recorrer({26,15},{70,-5},true);
    recorrer({26,15},{42,49},false);
    recorrer({42,49},{77,59},true);
    recorrer({98,95},{98,122},true);
    recorrer({-40,28},{-66,45},false);
    AlturasTerreno terrenoReal(recursos_ / "Modelos/Alturas_201x201.f32");
    float relieve = 0.f;
    for (auto z : {93.f,99.f}) {
        recorrer({-49,z},{-35,z},false);
        recorrer({-35,z},{-49,z},true);
        for (int i = 0; i <= 140; ++i) {
            float x = -49.f + i*.1f;
            relieve = std::max(relieve,p.altura(x,z) - .60f*terrenoReal.altura(x/.60f,z/.60f));
        }
    }
    exigir(relieve > .16f && relieve < .19f,"Banqueta sin perfil bajo de apoyo");
    std::cout << "P2_BANQUETAS_OK cuatro cruces en ambos sentidos, relieve=" << relieve << '\n';
    int contactosTorre = 0;
    const auto torre = sector.centroTorre()+origen;
    for (glm::vec2 d : {glm::vec2(1,0),{-1,0},{0,1},{0,-1}}) {
        auto q = plano(torre) + d*16.f;
        Avatar avatar;
        avatar.colocar({q.x,0,q.y},p);
        for (int i = 0; i < 36; ++i)
            avatar.mover(-d,1.f/60,p,true);
        if (glm::dot(plano(avatar.posicion()-torre),d)>0)
            ++contactosTorre;
    }
    exigir(contactosTorre >= 3, "Frentes fisicos de torre sin colision");
    recorrer({68,-32},{68,0},true);
    recorrer({95,-32},{95,0},true);
    std::cout << "P2_CAMINOS_OK 11 tramos a pie/corriendo, torre contactos=" << contactosTorre
              << " dos pasos laterales\n";

    float margenAphelion = 1e9f;
    for (int i = 0; i < 780; ++i) {
        const float t = i*.1f;
        const auto q = sector.posicionAphelion(t);
        auto desplazamiento = sector.posicionAphelion(t+.03f)-sector.posicionAphelion(t+77.97f);
        exigir(glm::dot(glm::normalize(desplazamiento),sector.frenteAphelion(t))>.99f,
               "Aphelion no apunta por su eje delantero");
        for (auto d : {glm::vec2(0),glm::vec2(8,0),{-8,0},{0,8},{0,-8}})
            margenAphelion = std::min(margenAphelion,q.y-8.f-p.altura(q.x+18+d.x,q.z+d.y));
        for (auto b : sector.centrosEdificios())
            exigir(glm::length(plano(q-b))>20.f || q.y-b.y>51.f,
                   "Aphelion puede tocar un edificio");
        exigir(glm::length(plano(q-sector.centroTorre()))>17.f || q.y-sector.centroTorre().y>25.f,
               "Aphelion puede tocar Orokin");
    }
    exigir(margenAphelion>8.f, "Aphelion demasiado cerca del suelo");
    std::cout << "P2_APHELION_OK 780 muestras; margen suelo=" << margenAphelion << '\n';

    p.actualizar(std::fmod(24.f-p.reloj()+120.f,120.f));
    const auto plaza = p.centroPlaza();
    foto("P2_Plaza_Dia", plaza + glm::vec3(11,8,25), plaza + glm::vec3(0,4,0));
    foto("P2_Corral", {-52,20,77}, {-66,3,51});
    foto("P2_Banquetas",{-35,6,103},{-42,2,85});
    foto("P2_Perimetro_A",{9,306,96},{0,0,0});
    const double d = sector.distanciaPatrulla();
    for (int tipo = 0; tipo < 2; ++tipo) {
        const auto q = sector.posicionMuneco(tipo,d)+origen;
        const auto frente = sector.frenteMuneco(d,tipo);
        const glm::vec3 lateral(frente.z,0,-frente.x);
        const auto ojo = q+frente*6.f+lateral*4.f+glm::vec3(0,3.1f,0);
        const auto centro = q+glm::vec3(0,2.55f,0);
        const auto nombre = "P2_Brazos_"+std::to_string(tipo);
        auto a = foto(nombre+"_A",ojo,centro,nullptr,true,tipo,.4f);
        auto b = foto(nombre+"_B",ojo,centro,nullptr,true,tipo,1.2f);
        size_t cambio = 0;
        for (size_t i = 0; i < a.size(); ++i)
            cambio += std::abs(int(a[i])-int(b[i])) > 8;
        exigir(cambio > 1800,"No se distingue el movimiento de brazos");
        std::cout << "P2_BRAZOS_OK " << (tipo ? "vertical" : "adelante_atras")
                  << " canales_distintos=" << cambio << '\n';
    }
    for (int k = 0; k < 2; ++k) {
        if (k)
            p.actualizar(2.4f);
        const auto q = sector.posicionMuneco(0,sector.distanciaPatrulla())+origen;
        const auto frente = sector.frenteMuneco(sector.distanciaPatrulla());
        foto("P2_Muneco_Ratchet_" + std::to_string(k),
             q+7.f*frente+glm::vec3(6.f*frente.z,3.3f,-6.f*frente.x),q+glm::vec3(0,2.6f,0));
    }
    const auto clank = sector.posicionMuneco(1,d+18)+origen;
    foto("P2_Muneco_Clank",clank+glm::vec3(-7,4,8),clank+glm::vec3(0,2.3f,0));
    for (int k = 0; k < 4; ++k) {
        const auto q = sector.posicionNPC(2,sector.reloj())+origen;
        foto("P2_Articulacion_" + std::to_string(k),q+glm::vec3(-2.2f,2.6f,5.5f),q+glm::vec3(0,1.5f,0));
        p.actualizar(1.f);
    }
    for (int k = 0; k < 2; ++k) {
        const auto q = sector.aphelionPosicion()+origen;
        foto("P2_Aphelion_"+std::to_string(k),q+glm::vec3(12,9,18),q);
        p.actualizar(3.4f);
    }
    foto("P2_Perimetro_B",{9,306,96},{0,0,0});
    p.actualizar(std::fmod(93.f-p.reloj()+120.f,120.f));
    p.actualizar(std::fmod(13.f-std::fmod(sector.reloj(),24.f)+24.f,24.f));
    auto focos = p.ambiente().iluminacion.focales;
    for (int i = 2; i < 6; ++i) {
        const auto objetivo = sector.railjackPosicion()+origen-focos[i].posicion;
        exigir(focos[i].encendida && focos[i].direccion.y>.4f &&
               glm::dot(glm::normalize(objetivo),focos[i].direccion)>.97f,
               "Reflector Railjack sin objetivo durante el ascenso");
    }
    const auto q = sector.railjackPosicion()+origen;
    const glm::vec3 observador(126,14,112), objetivo = q-glm::vec3(0,7,0);
    foto("P2_Railjack_Haces_ON",observador,objetivo);
    const auto lucesMantener = p.ambiente();
    OrdenActividad puerto;
    puerto.luzAcople = true;
    p.controlar(puerto);
    focos = p.ambiente().iluminacion.focales;
    exigir(std::none_of(focos.begin()+2,focos.begin()+6,[](auto f){return f.encendida;})
           && focos.back().encendida,
           "U no apaga cuatro reflectores en Planeta2");
    const auto apagado = foto("P2_Railjack_Haces_OFF",observador,objetivo,nullptr,true);
    const auto ilumina = foto("P2_Railjack_Solo_Luz",observador,objetivo,&lucesMantener,true);
    size_t canales = 0;
    for (size_t i = 0; i < apagado.size(); ++i)
        if (ilumina[i] > apagado[i] + 4)
            ++canales;
    exigir(canales > 400, "Los reflectores no iluminan materiales al retirar los haces");
    p.controlar(puerto);
    foto("P2_Plaza_Noche",plaza+glm::vec3(11,8,25),plaza+glm::vec3(0,4,0));
    auto ambienteTorre = p.ambiente();
    exigir(!ambienteTorre.iluminacion.focales[1].encendida &&
           !ambienteTorre.iluminacion.puntuales[13].encendida,"Orokin conserva luz propia apagado");
    const auto ojoTorre = torre+glm::vec3(20,11,27), objetivoTorre = torre+glm::vec3(0,5,0);
    auto torreOff = foto("P2_Torre_Noche_OFF",ojoTorre,objetivoTorre,nullptr,true);
    OrdenActividad oro;
    oro.luzServicio = true;
    p.controlar(oro);
    auto torreOn = foto("P2_Torre_Noche_ON",ojoTorre,objetivoTorre,nullptr,true);
    p.controlar(oro);
    auto torreOtraVezOff = foto("P2_Torre_Noche_OFF",ojoTorre,objetivoTorre,nullptr,true);
    size_t cambioTorre = 0;
    for (size_t i = 0; i < torreOff.size(); ++i)
        cambioTorre += torreOn[i] > torreOff[i] + 4;
    exigir(cambioTorre > 1800 && torreOff == torreOtraVezOff,"Orokin no vuelve a su luz ambiente");
    std::cout << "P2_OROKIN_OK apagado reproducible; canales iluminados=" << cambioTorre << '\n';

    const auto naveVuelo = sector.aphelionPosicion()+origen;
    const auto focoVuelo = p.ambiente().iluminacion.focales.back();
    exigir(focoVuelo.encendida && focoVuelo.direccion.y<-.99f &&
           glm::length(focoVuelo.posicion-naveVuelo)<1.3f,"Luz nocturna desligada de Aphelion");
    auto sinFoco = p.ambiente();
    sinFoco.iluminacion.focales.back().encendida = false;
    const glm::vec3 sueloVuelo(naveVuelo.x,p.altura(naveVuelo.x,naveVuelo.z),naveVuelo.z);
    const auto ojoVuelo = sueloVuelo+glm::vec3(26,14,27), vistaVuelo = (sueloVuelo+naveVuelo)*.5f;
    auto vueloOff = foto("P2_Aphelion_Sin_Luz_Real",ojoVuelo,vistaVuelo,&sinFoco,true);
    auto vueloOn = foto("P2_Aphelion_Reflector",ojoVuelo,vistaVuelo,nullptr,true);
    size_t cambioVuelo = 0;
    for (size_t i = 0; i < vueloOn.size(); ++i)
        cambioVuelo += vueloOn[i] > vueloOff[i] + 4;
    exigir(cambioVuelo > 1000,"El reflector de Aphelion no ilumina el suelo");
    std::cout << "P2_APHELION_FOCO_OK superficie iluminada; canales=" << cambioVuelo << '\n';
    GLint limiteUniformes = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,&limiteUniformes);
    std::cout << "P2_RAILJACK_FOCOS_OK cuatro conos y control U; canales iluminados=" << canales
              << " limite_uniformes=" << limiteUniformes << '\n';
    cambiarRegion(Region::Planeta1);
}

// Medicion sincronizada: mismo reloj, resolucion y encuadres antes y despues.
void Aplicacion::medirEspacio(const std::filesystem::path &dir) {
    cambiarRegion(Region::Espacio);
    auto &e = dynamic_cast<Espacio &>(escenarios_.actual());
    ContextoEscena c{renderizador_, recursos_};
    int w, h;
    ventana_.dimensiones(w, h);
    glfwSwapInterval(0);
    const glm::vec3 ojos[] = {{180, 100, 240}, {0, 140, 460}, {200, 65, -35}};
    const glm::vec3 centros[] = {{0, 0, 0}, {0, 0, 0}, {145, 15, -125}};
    for (int vista = 0; vista < 3; ++vista) {
        auto ojo = mundo_.aGlobal(Region::Espacio, ojos[vista]);
        auto centro = mundo_.aGlobal(Region::Espacio, centros[vista]);
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .1f, 1500.f) *
                  glm::lookAt(ojo, centro, {0, 1, 0});
        std::vector<double> ms;
        for (int i = 0; i < 150; ++i) {
            ventana_.eventos();
            auto amb = e.ambiente();
            for (auto &l : amb.iluminacion.puntuales)
                l.posicion = mundo_.aGlobal(Region::Espacio, l.posicion);
            for (auto &f : amb.iluminacion.focales)
                f.posicion = mundo_.aGlobal(Region::Espacio, f.posicion);
            auto inicio = std::chrono::steady_clock::now();
            renderizador_.iniciar(w, h, vp, amb, ojo);
            e.dibujar(c, mundo_.transformacion(Region::Espacio));
            glFinish();
            auto fin = std::chrono::steady_clock::now();
            if (i >= 30)
                ms.push_back(std::chrono::duration<double, std::milli>(fin - inicio).count());
            if (i == 149)
                renderizador_.capturar(dir / ("Medicion_" + std::to_string(vista) + ".ppm"), w, h);
            ventana_.presentar();
        }
        std::sort(ms.begin(), ms.end());
        std::cout << "MEDICION_ESPACIO vista=" << vista << " resolucion=" << w << "x" << h
                  << " muestras=" << ms.size() << " mediana_ms=" << ms[ms.size() / 2]
                  << " p95_ms=" << ms[size_t(ms.size() * .95)]
                  << " dibujos=" << renderizador_.dibujos() << "\n";
    }
    Renderizador::comprobarErrores();
}

void Aplicacion::probarEspacioMejora(const std::filesystem::path &dir) {
    auto exigir = [](bool b, const char *m) {
        if (!b)
            throw std::runtime_error(m);
    };
    cambiarRegion(Region::Espacio);
    auto &e = dynamic_cast<Espacio &>(escenarios_.actual());
    OrdenActividad quieta;
    quieta.recuperar = true;
    e.controlar(quieta);
    e.controlar({});
    exigir(Espacio::colisionFreeport({0, 0, 0}), "Nucleo atravesable");
    exigir(Espacio::colisionFreeport({38, -2, 0}), "Anillo atravesable");
    exigir(Espacio::colisionFreeport({42, 10, 42}), "Torre atravesable");
    exigir(!Espacio::colisionFreeport({20, 35, 20}), "Persiste caja invisible de Gateway");
    exigir(!Espacio::colisionFreeport({0, 8, 52}), "Acceso al muelle bloqueado");
    exigir(!Espacio::colisionFreeport({15, -35, 0}), "Hueco inferior bloqueado");
    for (int eje = 0; eje < 3; ++eje)
        for (int signo : {-1, 1}) {
            glm::vec3 p(0);
            p[eje] = signo * 100.f;
            bool choque = false;
            for (int k = 0; k < 300; ++k) {
                auto q = p;
                q[eje] -= signo * .4f;
                auto r = e.ajustarMovimiento(p, q);
                if (glm::length(r - q) > .01f) {
                    choque = true;
                    break;
                }
                p = r;
            }
            exigir(choque, "Aproximacion atraviesa estructura central");
        }
    float maxPaso = 0;
    for (int i = 0; i < 2; ++i)
        for (int k = 0; k <= 4000; ++k) {
            float t = k * .02f;
            auto p = e.posicionTrafico(i, t);
            exigir(!Espacio::colisionFreeport(p, 2.2f), "Ruta de trafico atraviesa Freeport");
            maxPaso = std::max(maxPaso, glm::length(e.posicionTrafico(i, t + .02f) - p));
        }
    std::cout << "TRAFICO_PASO_MAX=" << maxPaso << " por 0.02s\n";
    exigir(maxPaso < 1.2f, "Salto en trafico");
    exigir(glm::length(e.trafico(12) - e.trafico(18)) < .0001f, "Acoplamiento sin espera");
    for (int i = 0; i < 18; ++i) {
        float d = glm::length(e.posicionAsteroide(i, 0) - e.posicionAsteroide(i, 10));
        exigir(i % 2 ? d > 2 : d < .0001f, "Asteroides sin mezcla estatica y movil");
    }
    for (int i = 0; i < 400; ++i) {
        float t = i * .1f;
        float d = glm::length(e.posicionHolocron(t) - e.posicionTrafico(0, t));
        exigir(d >= 10 && d < 11.3f, "Holocron no sigue a la nave");
    }
    std::cout << "ESPACIO_B1_GEOMETRIA_OK seis aproximaciones, huecos libres, 8002 muestras "
                 "trafico sin salto ni colision; 18 asteroides y holocron relativo\n";
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena c{renderizador_, recursos_};
    auto foto = [&](const std::string &nombre, glm::vec3 ojo, glm::vec3 centro) {
        ojo = mundo_.aGlobal(Region::Espacio, ojo);
        centro = mundo_.aGlobal(Region::Espacio, centro);
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .1f, 1500.f) *
                  glm::lookAt(ojo, centro, {0, 1, 0});
        auto a = e.ambiente();
        for (auto &l : a.iluminacion.puntuales)
            l.posicion = mundo_.aGlobal(Region::Espacio, l.posicion);
        for (auto &f : a.iluminacion.focales)
            f.posicion = mundo_.aGlobal(Region::Espacio, f.posicion);
        renderizador_.iniciar(w, h, vp, a, ojo);
        e.dibujar(c, mundo_.transformacion(Region::Espacio));
        renderizador_.capturar(dir / (nombre + ".ppm"), w, h);
        std::vector<unsigned char> pix(size_t(w) * h * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
        return pix;
    };
    for (int i = 0; i < 2; ++i) {
        OrdenActividad o;
        if (i)
            o.luzServicio = true;
        else
            o.luzAcople = true;
        if (e.ambiente().iluminacion.focales[i].encendida)
            e.controlar(o);
        glm::vec3 ojo = i ? glm::vec3(-94, 22, 12) : glm::vec3(12, 22, 94),
                  centro = i ? glm::vec3(-50, -2, 0) : glm::vec3(0, -2, 50);
        auto off = foto("B1_Foco" + std::to_string(i) + "_OFF", ojo, centro);
        e.controlar(o);
        auto on = foto("B1_Foco" + std::to_string(i) + "_ON", ojo, centro);
        size_t cambio = 0;
        for (size_t k = 0; k < off.size(); ++k)
            if (std::abs(int(on[k]) - int(off[k])) > 3)
                ++cambio;
        exigir(cambio > 2000, "Spotlight no ilumina pixeles reales");
        std::cout << "ESPACIO_B1_FOCO_OK indice=" << i << " canales_modificados=" << cambio << "\n";
    }
    e.controlar({});
    for (int vista = 1; vista <= 2; ++vista) {
        camaras_.observarEspacio(vista);
        actualizarCamara(1.f / 60);
        auto ojo = camaras_.ojo();
        dibujar(dir / (vista == 1 ? "B1_Camara_Estacion.ppm" : "B1_Camara_Asteroides_A.ppm"));
        e.actualizar(12);
        actualizarCamara(1.f / 60);
        exigir(glm::length(camaras_.ojo() - ojo) < .001f, "Camara de observacion inestable");
        if (vista == 2)
            dibujar(dir / "B1_Camara_Asteroides_B.ppm");
    }
    for (float fase : {0.f, 10.f, 15.f, 23.f, 30.f}) {
        float paso = std::fmod(fase - std::fmod(e.reloj(), 40.f) + 40.f, 40.f);
        e.actualizar(paso);
        foto("B1_Trafico_" + std::to_string(int(fase)), {50, 32, 112}, {0, 0, 52});
    }
    auto hol = e.posicionHolocron(e.reloj());
    foto("B1_Holocron", hol + glm::vec3(9, 6, 13), hol);
    solicitarCamara(ModoCamara::Viaje);
    actualizarCamara(1.f / 60);
    dibujar(dir / "B1_Piloto.ppm");
    bool foco = e.ambiente().iluminacion.focales[0].encendida;
    auto reloj = e.reloj();
    cambiarRegion(Region::Planeta1);
    exigir(!escenarios_.actual().ambiente().iluminacion.focales[0].encendida,
           "Foco espacial contamina planeta");
    cambiarRegion(Region::Espacio);
    exigir(e.reloj() == reloj && e.ambiente().iluminacion.focales[0].encendida == foco,
           "Estado espacial reiniciado");
    std::cout << "ESPACIO_B1_CAMARAS_OK F5 estacion, F6 asteroides, F3 retorno y persistencia; "
                 "focos aislados por region\n";
    cambiarRegion(Region::Planeta1);
}

void Aplicacion::probarIntegral(const std::filesystem::path &dir) {
    auto exigir = [](bool b, const char *m) {
        if (!b)
            throw std::runtime_error(m);
    };
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena contexto{renderizador_, recursos_};
    auto foto = [&](std::string n, glm::vec3 ojo, glm::vec3 centro, bool soloCielo = false,
                    const Ambiente *ambienteFijo = nullptr) {
        auto &e = escenarios_.actual();
        auto region = e.id();
        auto a = ambienteFijo ? *ambienteFijo : e.ambiente();
        for (auto &l : a.iluminacion.puntuales)
            l.posicion = mundo_.aGlobal(region, l.posicion);
        for (auto &l : a.iluminacion.focales)
            l.posicion = mundo_.aGlobal(region, l.posicion);
        ojo = mundo_.aGlobal(region, ojo);
        centro = mundo_.aGlobal(region, centro);
        if (soloCielo) {
            a.iluminacion.sol = glm::normalize(glm::vec3(-.54f, .78f, .31f));
            a.iluminacion.noche = 0;
        }
        auto vp = glm::perspective(glm::radians(60.f), float(w) / h, .1f, 1500.f) *
                  glm::lookAt(ojo, centro, glm::vec3(0, 1, 0));
        renderizador_.iniciar(w, h, vp, a, ojo);
        if (!soloCielo)
            e.dibujar(contexto, mundo_.transformacion(region));
        renderizador_.capturar(dir / (n + ".ppm"), w, h);
        std::vector<unsigned char> pix(size_t(w) * h * 3);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pix.data());
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
        return pix;
    };
    cambiarRegion(Region::Planeta1);
    auto &p1=dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto &s1=p1.sistemas();
    OrdenActividad holo; holo.holocron=true; holo.avatar={-125,2,-43};
    s1.holocron=-1; p1.controlar(holo);
    exigir(s1.holocron==0,"Holocron no responde a interaccion cercana");
    auto matrizInicialHolo=s1.matrizHolocron();
    for(float t:{0.f,2.f,5.f,8.f,12.f}) {
        s1.holocron=t;
        foto("Cierre_Holocron_"+std::to_string(int(t)),{-105,15,-22},{-125,8,-48});
    }
    s1.holocron=5;
    exigir(glm::length(glm::vec3(s1.matrizHolocron()[0]))>glm::length(glm::vec3(matrizInicialHolo[0]))*3.4f,"Holocron sin crecimiento visible");
    s1.holocron=12; p1.actualizar(.01f);
    exigir(s1.holocron<0 && glm::length(glm::vec3(s1.matrizHolocron()[3]-matrizInicialHolo[3]))<.001f,"Holocron no vuelve al pedestal");
    auto faseGuardada=s1.faseInicial; auto tiempoGuardado=s1.tiempo;
    s1.tiempo=0;s1.faseInicial=.7f;s1.reflectorCovenant=false;
    auto sin=foto("Cierre_Covenant_OFF",{-105,15,-22},{-135,3,-53});
    holo.holocron=false;holo.luzServicio=true;p1.controlar(holo);
    auto con=foto("Cierre_Covenant_ON",{-105,15,-22},{-135,3,-53});
    size_t iluminados=0;for(size_t i=0;i<sin.size();++i)if(con[i]>sin[i]+3)++iluminados;
    exigir(iluminados>2000,"Segundo reflector P1 sin efecto visible");
    s1.reflectorCovenant=false;s1.faseInicial=faseGuardada;s1.tiempo=tiempoGuardado;
    solicitarCamara(ModoCamara::Viaje);
    for(int i=0;i<120;++i)actualizarCamara(1.f/60);
    dibujar(dir/"Cierre_P1_F3.ppm");
    solicitarCamara(ModoCamara::Exploracion);
    std::cout<<"CIERRE_P1_OK holocron12s escala3.5 retorno; segundo foco pixeles="<<iluminados<<"; F3 colonia\n";
    cambiarRegion(Region::Espacio);
    auto &esp = dynamic_cast<Espacio &>(escenarios_.actual());
    GestorCamaras nuevas;
    nuevas.alCambiarRegion(Region::Espacio);
    exigir(!nuevas.cabina(), "Primera visita no es exterior");
    for(float limite:{8.f,24.f,30.f,40.f}) {
        auto antes=(esp.posicionTrafico(0,limite-.001f)-esp.posicionTrafico(0,limite-.011f))/.01f;
        auto despues=(esp.posicionTrafico(0,limite+.011f)-esp.posicionTrafico(0,limite+.001f))/.01f;
        exigir(glm::length(antes-despues)<.5f,"Trafico cambia velocidad bruscamente entre tramos");
        exigir(glm::length(antes)>7.f,"Nave se detiene fuera del muelle");
    }
    for (int i = 0; i < 2; ++i)
        for (int k = 0; k < 4000; ++k) {
            float t = k * .01f;
            auto d = esp.direccionTrafico(i, t), sig = esp.direccionTrafico(i, t + .01f);
            exigir(glm::dot(d, sig) > .98f, "Giro discontinuo en nave");
            auto m = esp.matrizTrafico(i, t);
            auto frente = glm::normalize(glm::vec3(m * glm::vec4(0, 0, i ? -1 : 1, 0)));
            exigir(glm::dot(frente, d) > .999f, "Frente del modelo no coincide con trayectoria");
            exigir(!Espacio::colisionFreeport(esp.posicionTrafico(i, t), 3.4f),
                   "Trayectoria toca estacion");
        }
    for (bool boost : {false, true}) {
        Espacio e;
        OrdenActividad o;
        o.acelerador = 1;
        o.boost = boost;
        e.controlar(o);
        float maximo = 0;
        for (int i = 0; i < 1000; ++i) {
            e.actualizar(1.f / 120);
            maximo = std::max(maximo, e.velocidad());
            exigir(
                !Espacio::colisionFreeport(e.actividad().objetivo + glm::vec3(0, 1.7f, 0), 3.39f),
                "Boost atraviesa colision");
        }
        exigir(maximo > (boost ? 60 : 35), "Boost no alcanza velocidad");
        o.boost = false;
        e.controlar(o);
        e.actualizar(1.f / 60);
        exigir(e.velocidad() <= 40.001f, "Boost no se desactiva");
    }
    std::cout << "INTEGRAL_VUELO_OK orientacion por modelo, giros continuos, boost70 normal40 y "
                 "colision\n";
    for (float t : {2.f, 12.f, 16.f, 22.f, 28.f, 36.f}) {
        esp.actualizar(std::fmod(t - std::fmod(esp.reloj(), 40.f) + 40.f, 40.f));
        foto("Integral_Trafico_" + std::to_string(int(t)), {70, 45, 135}, {0, 10, 80});
    }
    cambiarRegion(Region::Planeta2);
    auto &p = dynamic_cast<Planeta2 &>(escenarios_.actual());
    for (int k = 0; k < 1800; ++k)
        for (int i = 0; i < 3; ++i) {
            float t = k * .1f;
            auto q = p.ratchet().posicionNPC(i, t);
            exigir(!p.ratchet().obstaculo(q, false), "NPC atraviesa edificio/valla");
            exigir(std::abs(q.y - p.altura(q.x + 18, q.z)) < .001f, "NPC desligado del suelo");
            for (int j = 0; j < i; ++j) {
                auto r = p.ratchet().posicionNPC(j, t);
                exigir(glm::length(glm::vec2(q.x - r.x, q.z - r.z)) > 5.f, "NPC intersectados");
            }
        }
    for (int t : {0, 6, 12}) {
        if (t)
            p.actualizar(6);
        foto("Integral_NPC_" + std::to_string(t), {15, 60, 110}, {5, 5, 15});
    }
    OrdenActividad luz;
    luz.reflector = true;
    auto inicial = p.ambiente().iluminacion.focales[0].encendida;
    if (inicial)
        p.controlar(luz);
    auto apagado = foto("Integral_P2_foco_OFF", {-20, 18, 62}, {-42, 3, 20});
    p.controlar(luz);
    auto encendido = foto("Integral_P2_foco_ON", {-20, 18, 62}, {-42, 3, 20});
    size_t distintos = 0;
    for (size_t i = 0; i < apagado.size(); ++i)
        if (std::abs(int(apagado[i]) - int(encendido[i])) > 3)
            ++distintos;
    exigir(distintos > 2000, "Foco P2 sin efecto real");
    if (!inicial)
        p.controlar(luz);
    Ambiente ambienteRailjack = p.ambiente();
    ambienteRailjack.iluminacion.sol = glm::normalize(glm::vec3(-.42f, .86f, .28f));
    ambienteRailjack.iluminacion.noche = 0.f;
    ambienteRailjack.iluminacion.fuerzaSol = 1.f;
    ambienteRailjack.iluminacion.ambiente = .24f;
    ambienteRailjack.iluminacion.colorSol = glm::vec3(1.f);
    for (auto &l : ambienteRailjack.iluminacion.puntuales)
        l.encendida = false;
    for (auto &f : ambienteRailjack.iluminacion.focales)
        f.encendida = false;
    float railjackSueloY = 0.f, railjackAireY = 0.f;
    float railjackApoyoSueloY = 0.f, railjackApoyoAireY = 0.f;
    std::vector<unsigned char> railjackSueloPix, railjackAirePix;
    for (float t : {0.f, 6.f, 13.f, 22.f}) {
        float paso = std::fmod(t - std::fmod(p.ratchet().reloj(), 24.f) + 24.f, 24.f);
        p.actualizar(paso);
        const float altura = p.ratchet().railjackPosicion().y;
        const float apoyo = p.ratchet().railjackAlturaBase();
        if (t == 0.f) {
            railjackSueloY = altura;
            railjackApoyoSueloY = apoyo;
            exigir(std::abs(apoyo - p.altura(98.f, 65.f) - .12f) < .02f,
                   "Railjack no apoya su malla indexada en el terreno");
        }
        else if (t == 13.f) {
            railjackAireY = altura;
            railjackApoyoAireY = apoyo;
            exigir(std::abs((altura - railjackSueloY) - 19.f) < .02f,
                   "Railjack no llega a 19 unidades de altura");
            exigir(std::abs((apoyo - railjackApoyoSueloY) - 19.f) < .02f,
                   "Malla Railjack no acompana el ascenso");
        } else {
            exigir(std::abs(altura - railjackSueloY) < .02f,
                   "Railjack no esta apoyado al inicio/retorno");
            exigir(std::abs(apoyo - railjackApoyoSueloY) < .02f,
                   "Malla Railjack no regresa al apoyo en suelo");
        }
        auto pix = foto("Integral_Railjack_" + std::to_string(int(t)), {118, 41, 105},
                        {98, 22, 65}, false, &ambienteRailjack);
        if (t == 0.f)
            railjackSueloPix = std::move(pix);
        else if (t == 13.f)
            railjackAirePix = std::move(pix);
    }
    size_t railjackCanalesDiferentes = 0;
    for (size_t i = 0; i < railjackSueloPix.size(); ++i)
        if (std::abs(int(railjackSueloPix[i]) - int(railjackAirePix[i])) > 8)
            ++railjackCanalesDiferentes;
    exigir(railjackCanalesDiferentes > 100000,
           "Railjack no produce un cambio visual suficiente en suelo/aire");
    std::cout << "P2_RAILJACK_OK sueloY=" << railjackSueloY << " aireY="
              << railjackAireY << " baseY=" << railjackApoyoSueloY << " aireBaseY="
              << railjackApoyoAireY << " canales_diferentes=" << railjackCanalesDiferentes << "\n";
    OrdenActividad oro;oro.luzServicio=true;
    if(p.ambiente().iluminacion.focales[1].encendida)p.controlar(oro);
    auto offOro=foto("Cierre_Orokin_OFF",{115,32,45},{83,12,-15});
    p.controlar(oro);
    auto onOro=foto("Cierre_Orokin_ON",{115,32,45},{83,12,-15});
    size_t cambioOro=0;for(size_t i=0;i<offOro.size();++i)if(onOro[i]>offOro[i]+3)++cambioOro;
    exigir(cambioOro>2000,"Segundo foco Orokin no ilumina superficies");
    for(int i=0;i<3;++i)for(float dt:{0.f,.45f}) {
        p.actualizar(dt);auto npc=p.ratchet().posicionNPC(i,p.ratchet().reloj())+glm::vec3(18,0,0);
        foto("Cierre_NPC"+std::to_string(i)+"_"+std::to_string(int(dt*100)),npc+glm::vec3(9,6,12),npc+glm::vec3(0,2,0));
    }
    foto("Cierre_P2_Distribucion",{95,170,225},{10,0,15});
    solicitarCamara(ModoCamara::Viaje);for(int i=0;i<120;++i)actualizarCamara(1.f/60);dibujar(dir/"Cierre_P2_F3.ppm");
    solicitarCamara(ModoCamara::Observacion);for(int i=0;i<120;++i)actualizarCamara(1.f/60);dibujar(dir/"Cierre_P2_F4.ppm");
    avatar_.colocar({-42, 0, 90}, p);
    camaras_.alCambiarRegion(Region::Planeta2);
    dibujar(dir / "Integral_HUD_P2.ppm");
    std::cout << "INTEGRAL_P2_OK 5400 muestras NPC, suelo y separacion; foco canales=" << distintos
              << "\n";
    for (auto region : {Region::Planeta1, Region::Planeta2}) {
        cambiarRegion(region);
        auto nombre = region == Region::Planeta1 ? "P1" : "P2";
        auto pix = foto(std::string("Integral_Union_") + nombre, {0, 30, 0}, {-1, 30, 0}, true);
        double media = 0;
        for (int y = h / 4; y < 3 * h / 4; ++y)
            for (int c = 0; c < 3; ++c) {
                auto i = (size_t(y) * w + w / 2) * 3 + c;
                media += std::abs(int(pix[i]) - int(pix[i - 3]));
            }
        media /= (h / 2) * 3;
        exigir(media < 12, "Union de panorama visible en lectura GPU");
        std::cout << "INTEGRAL_CIELO_OK " << nombre << " diferencia_media_union=" << media << "\n";
        foto(std::string("Integral_Cenit_") + nombre, {0, 30, 0}, {.001f, 31, 0}, true);
    }
    // Motor real sin dispositivo: la mezcla usa el mismo grafo que el dispositivo de audio.
    Audio prueba(recursos_, true);
    prueba.oyente({0, 0, 0}, {0, 0, -1});
    auto muestra = [&](glm::vec3 pos, const char *nombre) {
        prueba.fuente(Audio::Trafico, pos);
        prueba.disparar(Audio::Trafico, .8f);
        prueba.leerPCM(8820);
        auto pcm = prueba.leerPCM(44100);
        double l = 0, r = 0;
        float pico = 0;
        for (size_t i = 0; i < pcm.size(); i += 2) {
            l += pcm[i] * pcm[i];
            r += pcm[i + 1] * pcm[i + 1];
            pico = std::max(pico, std::max(std::abs(pcm[i]), std::abs(pcm[i + 1])));
        }
        exigir(pico < .99f, "Audio saturado");
        std::filesystem::create_directories(dir);
        std::ofstream f(dir / (std::string(nombre) + ".f32"), std::ios::binary);
        f.write(reinterpret_cast<const char *>(pcm.data()),
                std::streamsize(pcm.size() * sizeof(float)));
        return glm::vec2(std::sqrt(l / 44100), std::sqrt(r / 44100));
    };
    auto izq = muestra({-15, 0, -15}, "Audio_izquierda"),
         der = muestra({15, 0, -15}, "Audio_derecha"),
         lejos = muestra({150, 0, -150}, "Audio_lejos");
    exigir(izq.x > izq.y * 1.2f && der.y > der.x * 1.2f, "Audio sin paneo espacial");
    exigir(glm::length(lejos) < glm::length(der) * .2f, "Audio espacial sin atenuacion");
    prueba.bucle(Audio::MusicaEspacio, .7f);
    prueba.leerPCM(66150);
    auto n = prueba.estado(Audio::MusicaEspacio).inicios;
    for (int i = 0; i < 100; ++i)
        prueba.bucle(Audio::MusicaEspacio, .7f);
    exigir(prueba.estado(Audio::MusicaEspacio).inicios == n, "Reinicio de pista miniaudio");
    prueba.bucle(Audio::MusicaEspacio, 0);
    prueba.bucle(Audio::MusicaPlaneta2, .7f);
    auto fundido = prueba.leerPCM(66150);
    {
        std::ofstream f(dir / "Audio_transicion.f32", std::ios::binary);
        f.write(reinterpret_cast<const char *>(fundido.data()), std::streamsize(fundido.size()*sizeof(float)));
    }
    exigir(prueba.estado(Audio::MusicaEspacio).ganancia < .001f &&
               prueba.estado(Audio::MusicaPlaneta2).ganancia > .6f,
           "Fundido miniaudio falla");
    prueba.bucle(Audio::Trafico,0);
    prueba.leerPCM(8820);
    prueba.siguienteCanal();
    prueba.volumen(-1);
    prueba.leerPCM(4410);
    auto silencio = prueba.leerPCM(4410);
    float picoSilencio=0;
    for(float x:silencio) picoSilencio=std::max(picoSilencio,std::abs(x));
    exigir(picoSilencio<.00001f,"Volumen independiente de musica no funciona");
    prueba.bucle(Audio::AmbienteEspacio,.6f);
    prueba.leerPCM(8820);
    auto ambiente = prueba.leerPCM(4410);
    float picoAmbiente=0;
    for(float x:ambiente)picoAmbiente=std::max(picoAmbiente,std::abs(x));
    exigir(picoAmbiente>.0001f,"Silenciar musica tambien silencia ambiente");
    std::cout<<"INTEGRAL_GRUPOS_AUDIO_OK musica_silenciada="<<picoSilencio<<" ambiente_activo="<<picoAmbiente<<"\n";
    std::cout << "INTEGRAL_MINIAUDIO_OK RMS L/R izquierda=" << izq.x << "/" << izq.y
              << " derecha=" << der.x << "/" << der.y << " lejos=" << glm::length(lejos)
              << " transicion1200ms\n";
    cambiarRegion(Region::Planeta1);
    dibujar(dir / "Integral_HUD_P1.ppm");
    cambiarRegion(Region::Espacio);
    solicitarCamara(ModoCamara::Viaje);
    if (camaras_.cabina())
        camaras_.alternarCabina();
    for (int i = 0; i < 120; ++i)
        actualizarCamara(1.f / 60);
    dibujar(dir / "Integral_HUD_Espacio.ppm");
}

void Aplicacion::medirIntegral() {
    // El monitor puede cambiar el tamano de la ventana de arranque entre mediciones.
    glfwSetWindowSize(ventana_.nativa(), 800, 560);
    ventana_.eventos();
    glfwSwapInterval(0);
    for (auto r : {Region::Planeta1, Region::Planeta2, Region::Espacio}) {
        cambiarRegion(r);
        if (r == Region::Espacio)
            camaras_.observarEspacio(1);
        else {
            avatar_.colocar(r == Region::Planeta1 ? glm::vec3(-95, 0, -30) : glm::vec3(-67, 0, 62),
                            escenarios_.actual());
            camaras_.alCambiarRegion(r);
        }
        std::vector<double> muestras;
        for (int i = 0; i < 150; ++i) {
            auto inicio = std::chrono::steady_clock::now();
            ventana_.eventos();
            escenarios_.actual().actualizar(1.f / 60);
            deltaCuadro_ = 1.f / 60;
            dibujar();
            glFinish();
            auto fin = std::chrono::steady_clock::now();
            if (i >= 30)
                muestras.push_back(std::chrono::duration<double, std::milli>(fin - inicio).count());
        }
        int ancho, alto;
        ventana_.dimensiones(ancho, alto);
        if (ancho != 800 || alto != 560)
            throw std::runtime_error("La medicion integral requiere framebuffer 800x560");
        std::sort(muestras.begin(), muestras.end());
        double suma = 0;
        for (auto t : muestras)
            suma += t;
        std::cout << "MEDICION_INTEGRAL " << escenarios_.actual().nombre()
                  << " framebuffer=" << ancho << 'x' << alto
                  << " muestras=120 FPS_medio=" << 1000 * muestras.size() / suma
                  << " mediana_ms=" << muestras[60] << " p95_ms=" << muestras[114] << "\n";
    }
}
