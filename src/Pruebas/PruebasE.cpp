#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Espacio/Espacio.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <cmath>
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
    o.avatar = {-65, 3, 180};
    o.hunter = true;
    p.controlar(o);
    exigir(s.hunter == 0, "H no activa Hunter");
    for (float t : {0.f, 2.f, 4.f, 8.f}) {
        s.hunter = t;
        foto("Final_Hunter_" + std::to_string(int(t)), {-48, 9, 184}, {-65, 5, 174});
    }
    s.hunter = -1;
    p.controlar(o);
    exigir(s.hunter == 0, "Hunter no repite");
    o = {};
    o.avatar = {-65, 3, 162};
    o.inspeccion = true;
    p.controlar(o);
    exigir(s.inspeccion == 0, "K no activa Grunt 2");
    s.inspeccion = 3;
    foto("Final_Grunt_inspeccion", {-48, 8, 178}, {-65, 3, 162});
    p.actualizar(6);
    exigir(s.inspeccion < 0, "Grunt 2 no finaliza");
    p.controlar(o);
    exigir(s.inspeccion == 0, "Grunt 2 no repite");
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
    exigir(glm::length(e.ajustarMovimiento(p, {0, 0, 0}) - p) < .001f, "Gateway atravesable");
    exigir(glm::length(e.trafico(0) - e.trafico(40)) < .001f, "Trafico no ciclico");
    exigir(glm::length(e.trafico(12) - e.trafico(18)) < .001f, "Trafico sin espera");
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
    foto("Espacio_Gateway", {90, 45, 100}, {0, 0, 0});
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
    std::cout << "ESPACIO_OK navegacion avance giro altura, colision Gateway, trafico40s, "
                 "persistencia, camaras, portal Planeta1\n";
}
