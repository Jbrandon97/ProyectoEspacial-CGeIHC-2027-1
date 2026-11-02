#include "Planeta1.h"
#include "CircuitoCarreras.h"
#include "Distribucion.h"
#include "Graficos/Geometria.h"
#include "Graficos/Renderizador.h"
#include "TerrenoPlaneta1.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
void Planeta1::cargar(ContextoEscena &c) {
    liberar();
    carrera_ = GestorCarrera();
    sistemas_ = SistemasPlaneta1();
    sistemas_.configurar(c.recursos / "Configuracion_C.txt");
    eventoPuerta_ = 0;
    puertaAltura_ = puertaDestino_ = 7;
    for (auto &e : Distribucion::edificios)
        modelos_[e.modelo] = std::make_unique<ModeloGltf>(
            c.recursos / "Modelos" /
            (std::string(e.modelo) +
             (std::string(e.modelo) == "Plataforma" ? "_E.gltf" : ".gltf")));
    for (auto &e : Distribucion::edificios)
        colisiones_.agregar(modelos_.at(e.modelo)->triangulos(), {e.centro.x, 2, e.centro.y});
    colisiones_.agregar(CircuitoCarreras::construir(), CircuitoCarreras::origen());
    colisiones_.agregar(CircuitoCarreras::detalles(), CircuitoCarreras::origen());
    actualizarPuerta();
    std::vector<Vertice> rampa;
    glm::vec3 a(199.2f, 3.03f, 145), b(208, 2.02f, 145), d(199.2f, 3.03f, 151), e(208, 2.02f, 151);
    Geometria::triangulo(rampa, a, d, e, {.48f, .51f, .49f});
    Geometria::triangulo(rampa, a, e, b, {.48f, .51f, .49f});
    // Acceso interior en la misma recta: conserva el trazado y las puertas de carrera.
    glm::vec3 ia(186.8f, 3.03f, 145), ib(178, 2.02f, 145), ic(178, 2.02f, 151),
        id(186.8f, 3.03f, 151);
    Geometria::triangulo(rampa, ia, ib, ic, {.48f, .51f, .49f});
    Geometria::triangulo(rampa, ia, ic, id, {.48f, .51f, .49f});
    colisiones_.agregar(rampa, {0, 0, 0});
    rampaPeatonal_ = std::make_unique<Malla>(rampa);
    for (auto nombre : {"Grunt", "Hunter", "Sentinel", "EnergySword"})
        modelos_[nombre] =
            std::make_unique<ModeloGltf>(c.recursos / "Modelos" / (std::string(nombre) + ".gltf"));
    modelos_["SaturnV"] = std::make_unique<ModeloGltf>(c.recursos / "Modelos/SaturnV_Etapas.gltf");
    colisionCohete_.agregar(modelos_.at("SaturnV")->triangulos(), {205, 5.22f, 21});
    std::vector<Vertice> llama;
    Geometria::cono(llama, {0, 0, 0}, 1.8f, -7, {1, .65f, .18f}, 12);
    propulsion_ = std::make_unique<Malla>(llama);
    std::vector<Vertice> hoja, marco;
    Geometria::caja(hoja, {0, 0, 0}, {5, .18f, 10}, {.23f, .27f, .29f});
    for (int j = -4; j <= 4; j += 2)
        Geometria::caja(hoja, {0, .095f, float(j)}, {4.7f, .012f, .14f}, {.53f, .38f, .17f});
    // Brocal y guardas visibles; la zona central queda reservada al mecanismo.
    for (int signo : {-1, 1}) {
        Geometria::caja(marco, {signo * 5.6f, 5.35f, 0}, {.45f, .3f, 11.6f}, {.45f, .33f, .16f});
        Geometria::caja(marco, {0, 5.35f, signo * 5.6f}, {11.6f, .3f, .45f}, {.45f, .33f, .16f});
        Geometria::caja(marco, {signo * 6.f, 5.9f, 0}, {.16f, 1.4f, 11.8f}, {.25f, .29f, .31f});
        Geometria::caja(marco, {0, 5.9f, signo * 6.f}, {12, .16f, .16f}, {.65f, .45f, .18f});
    }
    Geometria::caja(marco, {0, 2.06f, 0}, {10, .08f, 10}, {.035f, .04f, .045f});
    colisiones_.agregar(marco, {205, 0, 21});
    hojaCompuerta_ = std::make_unique<Malla>(hoja);
    marcoCompuerta_ = std::make_unique<Malla>(marco);
    modelos_["Pino"] = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Pino.gltf");
    suelo_ = std::make_unique<Textura>(c.recursos / "Modelos/Texturas/Terreno_B2.png");
    texturaAgua_ = std::make_unique<Textura>(c.recursos / "Modelos/Texturas/Agua.png");
    agua_ = std::make_unique<Malla>(TerrenoPlaneta1::agua());
    terreno_ = std::make_unique<Malla>(TerrenoPlaneta1::construir());
    paisaje_ = std::make_unique<Malla>(TerrenoPlaneta1::paisaje());
    pista_ = std::make_unique<Malla>(CircuitoCarreras::construir());
    detalles_ = std::make_unique<Malla>(CircuitoCarreras::detalles());
    banshee_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Banshee.gltf");
    ghost_ = std::make_unique<ModeloGltf>(c.recursos / "Modelos/Ghost.gltf");
}
void Planeta1::actualizar(float dt) {
    carrera_.actualizar(dt);
    sistemas_.actualizar(dt);
    // Sensor impide cerrar sobre el avatar; no teletransporta ni aplasta.
    if (puertaDestino_ < puertaAltura_ && std::abs(visitante_.x - 165) < 21 &&
        std::abs(visitante_.z + 77.5f) < 3.5f)
        puertaDestino_ = 7;
    float nueva = puertaAltura_ + glm::clamp(puertaDestino_ - puertaAltura_, -2 * dt, 2 * dt);
    if (nueva != puertaAltura_) {
        puertaAltura_ = nueva;
        actualizarPuerta();
    }
}
void Planeta1::dibujar(ContextoEscena &c, const glm::mat4 &m) const {
    auto &r = c.renderizador;
    r.texturada(*terreno_, m, suelo_->id(), glm::vec3(1));
    r.dibujar(*paisaje_, m);
    r.texturada(*agua_, m, texturaAgua_->id(), glm::vec3(1));
    for (auto &e : Distribucion::edificios) {
        auto modelo = glm::translate(m, {e.centro.x, 2, e.centro.y});
        if (r.visible(glm::vec3(modelo[3]) + glm::vec3(0, 15, 0), 65))
            modelos_.at(e.modelo)->dibujar(
                r, modelo, std::string(e.modelo) == "Hangar" ? puertaAltura_ - 7 : 0);
    }
    r.dibujar(*rampaPeatonal_, m);
    struct Instancia {
        const char *modelo;
        glm::vec3 sitio;
        float giro;
    };
    const Instancia npc[] = {
        {"Grunt", {-77, 0, 153}, 0},           {"Grunt", {-65, 0, 162}, .7f},
        {"Grunt", {-53, 0, 153}, -.7f},        {"Hunter", {-65, 0, 174}, 0},
        {"Sentinel", {-110, 6, -95}, 0},       {"Sentinel", {115, 7, -45}, 1.5f},
        {"EnergySword", {-151, .04f, -61}, 0}, {"EnergySword", {-137, .04f, -61}, .4f}};
    int numero = 0;
    for (auto &n : npc) {
        auto p = n.sitio;
        p.y += TerrenoPlaneta1::altura(p.x, p.z);
        float giro = n.giro;
        if (numero == 4 || numero == 5) {
            p = sistemas_.sentinel(numero - 4);
            giro = sistemas_.giroSentinel(numero - 4);
        }
        auto local = glm::rotate(glm::translate(m, p), giro, glm::vec3(0, 1, 0)) *
                     sistemas_.animacionNPC(numero);

        ++numero;
        if (r.visible(glm::vec3(local[3]), 8))
            modelos_.at(n.modelo)->dibujar(r, local);
    }
    r.dibujar(*marcoCompuerta_, glm::translate(m, {205, 0, 21}));
    for (int signo : {-1, 1}) {
        auto hoja = glm::translate(
            m, {205 + signo * (2.5f + 5.25f * sistemas_.aperturaCompuerta()), 5.12f, 21});
        r.dibujar(*hojaCompuerta_, hoja);
    }
    auto cohete = glm::translate(m, sistemas_.cohete());
    auto etapas = sistemas_.etapas();
    r.recortarDebajo(glm::vec3(m * glm::vec4(0, 5.21f, 0, 1)).y);
    if (sistemas_.visibleCohete())
        modelos_.at("SaturnV")->dibujar(r, cohete, 0, &etapas);
    r.recortarDebajo(-100000.f);
    if (sistemas_.motor()) {
        // Dos conos reutilizados, nucleo caliente y envolvente pulsante; sin particulas.
        float f =
            .9f + .12f * std::sin(sistemas_.tiempo * 29) + .06f * std::sin(sistemas_.tiempo * 47);
        float base = sistemas_.lanzamiento < 7 ? 0.f : sistemas_.lanzamiento < 11 ? 17.95f : 31.53f;
        auto motor = glm::translate(cohete, {0, base, 0});
        r.texturada(*propulsion_, glm::scale(motor, {1.f, f, 1.f}), 0, {1, .55f, .12f},
                    {1, .32f, .04f});
        r.texturada(*propulsion_, glm::scale(motor, {.55f, f * .72f, .55f}), 0, {1, .94f, .7f},
                    {1, .8f, .4f});
    }
    auto luces = sistemas_.luces();
    for (int j = 0; j < 6; ++j) {
        auto &l = luces.puntuales[j];
        float suelo = TerrenoPlaneta1::altura(l.posicion.x, l.posicion.z);
        r.dibujar({{l.posicion.x, (l.posicion.y + suelo) * .5f, l.posicion.z},
                   {.18f, l.posicion.y - suelo, .18f},
                   {.19f, .22f, .23f}},
                  m);
        r.material(.4f, .5f);
        r.dibujar({l.posicion, {.8f, .4f, .8f}, glm::mix(glm::vec3(.18f), l.color, luces.noche)},
                  m);
        r.material(.85f);
    }
    int arbol = 0;
    for (auto q : Distribucion::arboles) {
        auto modelo = glm::translate(m, {q.x, TerrenoPlaneta1::altura(q.x, q.y), q.y});
        modelo = glm::rotate(modelo, float(arbol) * 2.4f, glm::vec3(0, 1, 0));
        modelo = glm::scale(modelo, glm::vec3(.8f + float(arbol++ % 5) * .12f));
        if (r.visible(glm::vec3(modelo[3]) + glm::vec3(0, 8, 0), 17))
            modelos_.at("Pino")->dibujar(r, modelo);
    }
    auto circuito = glm::translate(m, CircuitoCarreras::origen());
    r.dibujar(*pista_, circuito);
    r.dibujar(*detalles_, circuito);
    banshee_->dibujar(r, circuito * carrera_.nave().matriz());
    for (const auto &rival : carrera_.rivales())
        ghost_->dibujar(r, circuito * rival.matriz());
}
void Planeta1::liberar() noexcept {
    colisiones_.limpiar();
    colisionCohete_.limpiar();
    propulsion_.reset();
    hojaCompuerta_.reset();
    marcoCompuerta_.reset();
    colisionPuerta_.limpiar();
    rampaPeatonal_.reset();
    modelos_.clear();
    suelo_.reset();
    agua_.reset();
    texturaAgua_.reset();
    banshee_.reset();
    ghost_.reset();
    detalles_.reset();
    pista_.reset();
    paisaje_.reset();
    terreno_.reset();
}
float Planeta1::altura(float x, float z) const noexcept {
    auto puntual = [&](float px, float pz) {
        auto pr = CircuitoCarreras::proyectar(glm::vec3(px, 0, pz) - CircuitoCarreras::origen());
        return pr.distancia <= CircuitoCarreras::Ancho / 2
                   ? CircuitoCarreras::muestra(pr.s).posicion.y
                   : colisiones_.apoyo(px, pz, TerrenoPlaneta1::altura(px, pz));
    };
    // Apoyo de la huella del avatar: evita que caiga antes de librar un bordillo.
    float h = puntual(x, z);
    for (auto q :
         {glm::vec2(.65f, 0), glm::vec2(-.65f, 0), glm::vec2(0, .65f), glm::vec2(0, -.65f)})
        h = std::max(h, puntual(x + q.x, z + q.y));
    return h;
}
glm::vec3 Planeta1::ajustarMovimiento(glm::vec3 a, glm::vec3 p) const {
    auto soporte = [this](float x, float z) { return altura(x, z); };
    auto q = colisiones_.mover(a, p, soporte);
    q = colisionPuerta_.mover(a, q, soporte);
    return sistemas_.lanzamiento < 5 ? colisionCohete_.mover(a, q, soporte) : q;
}

void Planeta1::actualizarPuerta() {
    colisionPuerta_.limpiar();
    if (modelos_.count("Hangar"))
        colisionPuerta_.agregar(modelos_.at("Hangar")->puerta(),
                                {165, 2 + puertaAltura_ - 7, -100});
}
void Planeta1::controlar(const OrdenActividad &o) {
    visitante_ = o.avatar;
    carrera_.controlar(o);
    if (o.puerta && !carrera_.estado().ocupado &&
        glm::length(glm::vec2(o.avatar.x - 165, o.avatar.z + 70)) < 30) {
        puertaDestino_ = puertaDestino_ > 0 ? 0.0f : 7.0f;
        ++eventoPuerta_;
    }
    if (!carrera_.ocupado()) {
        float d = glm::length(glm::vec2(o.avatar.x - 205, o.avatar.z - 21));
        if (o.lanzar && d < 65 && d > 8)
            sistemas_.lanzar();
        if (o.reiniciarCohete && d < 90 && d > 9)
            sistemas_.reiniciar();
        if (o.reaccion && glm::length(glm::vec2(o.avatar.x + 77, o.avatar.z - 153)) < 10 &&
            sistemas_.reaccion < 0) {
            sistemas_.reaccion = 0;
            ++sistemas_.eventoGrunt;
        }
    }
}
EstadoActividad Planeta1::actividad() const {
    auto e = carrera_.estado();
    if (!e.ocupado && glm::length(glm::vec2(visitante_.x - 165, visitante_.z + 70)) < 30)
        e.mensajes.push_back("E / ABRIR O CERRAR HANGAR / SENSOR DE PASO");
    if (!e.ocupado) {
        if (glm::length(glm::vec2(visitante_.x - 205, visitante_.z - 21)) < 65)
            e.mensajes.push_back("L LANZAR / T REPONER / O OBSERVAR COHETE");
        if (glm::length(glm::vec2(visitante_.x + 77, visitante_.z - 153)) < 10)
            e.mensajes.push_back("G / SALUDAR AL GRUNT");
    }
    return e;
}
