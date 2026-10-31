#include "PruebasB1.h"
#include "Personajes/Avatar.h"
#include "Regiones/Planeta1/CircuitoCarreras.h"
#include "Regiones/Planeta1/GestorCarrera.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
namespace {
void exigir(bool b, const char *s) {
    if (!b)
        throw std::runtime_error(s);
}
void andar(Avatar &a, glm::vec2 d, float segundos, const Escenario &e) {
    for (int i = 0; i < int(segundos * 120); ++i)
        a.mover(d, 1.0f / 120, e);
}
} // namespace
void verificarB1(const Escenario &e) {
    Avatar a;
    a.colocar({205, 0, -35}, e);
    andar(a, {0, 1}, 47.0f / 15, e);
    std::cout << "PLATAFORMA_RECORRIDO x=" << a.posicion().x << " y=" << a.posicion().y
              << " z=" << a.posicion().z << "\n";
    exigir(a.posicion().z > 10 && a.posicion().y > 6 && a.posicion().y < 6.4f,
           "No sube caminando a plataforma");
    andar(a, {1, 0}, .6f, e);
    exigir(a.posicion().y > 6, "No permanece sobre plataforma");
    andar(a, {-1, 0}, .6f, e);
    andar(a, {0, -1}, 47.0f / 15, e);
    std::cout << "DESCENSO x=" << a.posicion().x << " y=" << a.posicion().y
              << " z=" << a.posicion().z << "\n";
    exigir(a.posicion().z < -34 &&
               std::abs(a.posicion().y - e.altura(a.posicion().x, a.posicion().z) - 1) < .01f,
           "No desciende por rampa");
    a.colocar({205, 0, 12}, e);
    andar(a, {1, 0}, 3, e);
    exigir(a.posicion().x < 225, "Atraviesa baranda plataforma");
    a.colocar({165, 0, -65}, e);
    andar(a, {0, -1}, 3, e);
    exigir(a.posicion().z < -99, "No entra al hangar abierto");
    a.colocar({240, 0, 21}, e);
    andar(a, {-1, 0}, 3, e);
    exigir(a.posicion().x > 226, "Atraviesa lateral plataforma");
    for (float x : {5.0f, 220.0f}) {
        a.colocar({x, 0, 80}, e);
        andar(a, {0, 1}, 10, e);
        exigir(a.posicion().z > 199, "Barrera invisible alrededor de pista");
    }
    a.colocar({40, 0, -90}, e);
    andar(a, {0, -1}, 3, e);
    std::cout << "EDIFICIO x=" << a.posicion().x << " y=" << a.posicion().y
              << " z=" << a.posicion().z << "\n";
    exigir(a.posicion().z > -104.2f, "Atraviesa edificio");
    a.colocar({40, 0, -90}, e);
    andar(a, {1, -1}, 3, e);
    exigir(a.posicion().x > 60, "Esquina bloquea deslizamiento");
    a.colocar({-125, 0, 85}, e);
    andar(a, {0, -1}, 4, e);
    exigir(!TerrenoPlaneta1::lago(a.posicion().x, a.posicion().z, 1.8f), "Entra al lago");
    bool pendiente = false;
    for (int x = 240; x < 290 && !pendiente; ++x) {
        if (TerrenoPlaneta1::normal(float(x), 0).y < .6f) {
            glm::vec3 previo(float(x) - .1f, 0, 0), destino(float(x), 0, 0);
            exigir(TerrenoPlaneta1::permitir(previo, destino) == previo,
                   "Escala montana pronunciada");
            pendiente = true;
        }
    }
    exigir(pendiente, "Falta caso de pendiente");
    std::cout << "COLISIONES_B1_OK ascenso permanencia descenso laterales edificios esquinas lagos "
                 "montanas y exteriores pista\n";
    GestorCarrera g;
    OrdenActividad o;
    o.avatar = CircuitoCarreras::muestra(0).posicion + CircuitoCarreras::origen();
    o.interactuar = true;
    g.controlar(o);
    o.interactuar = false;
    for (int i = 0; i < 400; ++i)
        g.actualizar(1.0f / 120);
    o.recuperar = true;
    g.controlar(o);
    float inicio = g.tiempo();
    exigir(std::abs(g.recuperacionRestante() - 2) < .001f, "Castigo no es dos segundos");
    for (int i = 0; i < 60; ++i)
        g.actualizar(1.0f / 120);
    float restante = g.recuperacionRestante();
    g.controlar(o);
    exigir(std::abs(g.recuperacionRestante() - restante) < .001f,
           "Recuperacion duplicada extiende castigo");
    o.recuperar = false;
    g.controlar(o);
    for (int i = 0; i < 181; ++i)
        g.actualizar(1.0f / 120);
    exigir(g.recuperacionRestante() == 0 && std::abs(g.tiempo() - inicio - 241.0f / 120) < .02f,
           "Penalizacion altera reloj dos veces");
    GestorCarrera automatica;
    OrdenActividad autoOrden;
    autoOrden.avatar = CircuitoCarreras::muestra(0).posicion + CircuitoCarreras::origen();
    autoOrden.interactuar = true;
    automatica.controlar(autoOrden);
    autoOrden.interactuar = false;
    autoOrden.acelerador = 1;
    automatica.controlar(autoOrden);
    for (int i = 0; i < 3000 && automatica.recuperacionRestante() == 0; ++i)
        automatica.actualizar(1.0f / 120);
    exigir(automatica.recuperacionRestante() > 1.98f, "Recuperacion automatica no aplica 2 s");
    exigir(CircuitoCarreras::proyectar(automatica.nave().posicion()).distancia < .01f,
           "Recuperacion fuera de punto seguro");
    for (int i = 0; i < 48; ++i)
        automatica.actualizar(1.0f / 120);
    float castigoAuto = automatica.recuperacionRestante();
    autoOrden.recuperar = true;
    automatica.controlar(autoOrden);
    exigir(std::abs(automatica.recuperacionRestante() - castigoAuto) < .001f,
           "Manual duplica penalizacion automatica");
    std::cout << "PENALIZACION_B1_OK 2 segundos sin reinicio ni suma doble\n";
    for (float maxima : {37.0f, 38.0f}) {
        RivalCarrera r(maxima, 3);
        float tiempo = 0;
        for (int i = 0; i < 20000 && r.avance() < 3 * CircuitoCarreras::Longitud; ++i) {
            float antes = r.velocidad();
            r.actualizar(1.0f / 120);
            exigir(std::abs(r.velocidad() - antes) <= 18.0f / 120 + .001f,
                   "Rival cambia velocidad instantaneamente");
            tiempo += 1.0f / 120;
        }
        exigir(r.avance() >= 3 * CircuitoCarreras::Longitud, "Rival no termina");
        std::cout << "RIVAL_B1 maxima=" << maxima << " tiempo=" << tiempo << "\n";
        r.reiniciar();
        exigir(r.velocidad() == 0 && r.avance() == 9, "Rival no reinicia");
    }
    // Dos perfiles conducen con las mismas ordenes de aceleracion/giro que el jugador.
    for (int perfil = 0; perfil < 2; ++perfil) {
        GestorCarrera carrera;
        OrdenActividad orden;
        orden.avatar = CircuitoCarreras::muestra(0).posicion + CircuitoCarreras::origen();
        orden.interactuar = true;
        carrera.controlar(orden);
        orden.interactuar = false;
        for (int i = 0; i < 24000 && carrera.fase() != FaseCarrera::Terminada; ++i) {
            auto &n = carrera.nave();
            auto pro = CircuitoCarreras::proyectar(n.posicion());
            auto d = CircuitoCarreras::muestra(pro.s + 12).posicion - n.posicion();
            float error = std::remainder(std::atan2(d.x, -d.z) -
                                             std::atan2(n.direccion().x, -n.direccion().z),
                                         2 * CircuitoCarreras::Pi);
            orden.giro = std::clamp(error * 3.5f, -1.0f, 1.0f);
            float meta = std::abs(error) > .19f ? (perfil == 0 ? 27.0f : 23.0f)
                                                : (perfil == 0 ? 37.0f : 31.0f);
            orden.acelerador = n.velocidad() < meta ? 1.0f : 0.0f;
            orden.freno = n.velocidad() > meta + 1 ? .25f : 0.0f;
            carrera.controlar(orden);
            carrera.actualizar(1.0f / 60);
        }
        exigir(carrera.fase() == FaseCarrera::Terminada, "Perfil no completa carrera");
        std::cout << "EQUILIBRIO_B1 perfil=" << (perfil == 0 ? "rapido" : "conservador")
                  << " tiempo=" << carrera.tiempo() << " puesto=" << carrera.posicion() << "\n";
        exigir(carrera.posicion() == (perfil == 0 ? 1 : 3),
               "Rivales sin equilibrio entre perfiles");
    }
}
