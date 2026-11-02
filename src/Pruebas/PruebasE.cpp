#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Planeta1.h"
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
    int w, h;
    ventana_.dimensiones(w, h);
    ContextoEscena c{renderizador_, recursos_};
    auto captura = [&](const char *nombre, glm::vec3 ojo, glm::vec3 centro) {
        auto vp = glm::perspective(glm::radians(55.f), float(w) / h, .15f, 1800.f) *
                  glm::lookAt(ojo, centro, {0, 1, 0});
        renderizador_.iniciar(w, h, vp, p.ambiente(), ojo);
        p.dibujar(c, glm::mat4(1));
        renderizador_.capturar(dir / (std::string(nombre) + ".ppm"), w, h);
        Renderizador::comprobarErrores();
        ventana_.presentar();
        ventana_.eventos();
    };
    // Fraccionar deltaTime no altera el estado final ni permite teletransportar un vuelo activo.
    for (int fps : {30, 60, 144}) {
        SistemasPlaneta1 a;
        a.lanzar();
        for (int i = 0; i < fps * 5; ++i)
            a.actualizar(1.f / fps);
        auto antes = a.cohete();
        a.reiniciar();
        exigir(a.reposicionPendiente && glm::length(a.cohete() - antes) < .001f,
               "E reset corta vuelo activo");
        for (int i = 0; i < fps * 24; ++i)
            a.actualizar(1.f / fps);
        exigir(a.reposicion < 0 && a.lanzamiento < 0 && std::abs(a.cohete().y - 5.22f) < .001f,
               "E reposicion no determinista");
    }
    s = SistemasPlaneta1();
    s.faseInicial = .3f;
    s.lanzar();
    for (int i = 0; i <= 1200; ++i) {
        s.lanzamiento = i / 60.f;
        auto e = s.etapas();
        for (auto &m : e)
            for (int col = 0; col < 4; ++col)
                for (int row = 0; row < 4; ++row)
                    exigir(std::isfinite(m[col][row]), "E matriz de etapa no finita");
        exigir(glm::length(glm::vec3(e[2][3])) < .00001f, "E etapa superior duplicada/movida");
    }
    for (float t : {7.f, 11.f}) {
        s.lanzamiento = t;
        auto a = s.etapas();
        s.lanzamiento = t + .001f;
        auto b = s.etapas();
        int etapa = t == 7 ? 0 : 1;
        exigir(glm::length(glm::vec3(a[etapa][3] - b[etapa][3])) < .2f, "E salto en separacion");
    }
    s.lanzamiento = 9;
    exigir(glm::length(glm::vec3(s.etapas()[0][3])) > 5, "E primera etapa inmovil");
    exigir(glm::length(glm::vec3(s.etapas()[1][3])) < .001f, "E segunda etapa se adelanta");
    captura("E_Separacion_1", s.cohete() + glm::vec3(85, 25, 110), s.cohete() + glm::vec3(0, 8, 0));
    s.lanzamiento = 12.5f;
    exigir(glm::length(glm::vec3(s.etapas()[1][3])) > 5, "E segunda etapa inmovil");
    captura("E_Separacion_2", s.cohete() + glm::vec3(65, 15, 85), s.cohete() + glm::vec3(0, 20, 0));
    s.lanzamiento = 18;
    exigir(s.cohete().y > 1200 && s.visibleCohete(), "E ascenso insuficiente");
    observandoCohete_ = true;
    dibujar(dir / "E_Altura_observacion.ppm");
    s.lanzamiento = 20;
    exigir(!s.visibleCohete(), "E vuelo no termina");
    s.reiniciar();
    exigir(s.cohete().y < 0 && s.reposicion == 0, "E nuevo cohete aparece de golpe");
    for (float t : {0.f, .7f, 2.5f, 4.8f, 6.5f, 7.7f}) {
        s.reposicion = t;
        std::string nombre = "E_Reposicion_" + std::to_string(int(t * 10));
        captura(nombre.c_str(), {224, 19, 41}, {205, 8, 21});
    }
    s.actualizar(1);
    exigir(s.reposicion < 0 && s.aperturaCompuerta() == 0, "E compuerta no termina cerrada");
    s.reiniciar();
    exigir(s.retirarPreparado && std::abs(s.cohete().y - 5.22f) < .001f,
           "E retiro preparado salta");
    s.actualizar(2.5f);
    exigir(s.cohete().y < 0 && s.aperturaCompuerta() > .99f, "E retiro sin abrir compuerta");
    captura("E_Retiro_preparado", {224, 19, 41}, {205, 8, 21});
    float t = s.reposicion;
    s.reiniciar();
    exigir(s.reposicion == t, "E T repetida reinicia mecanismo");
    std::cout << "E_COHETE_OK etapas reales 7/11 s; >1200 unidades a 18 s; reposicion y cola "
                 "30/60/144 FPS\n";
    // Audio: objetivo espacial y conteo de inicios verifican condiciones, no una audicion humana.
    s = SistemasPlaneta1();
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
    s = SistemasPlaneta1();
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
    std::cout << "E_AVATAR_OK reposo exacto, jerarquia conservada, escala 1.05, velocidad 15/30\n";
    s = original;
    foco_ = false;
    observandoCohete_ = false;
    actualizarAudio();
    camaras_.alCambiarRegion(Region::Planeta1);
}
