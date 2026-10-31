#include "Pruebas/PruebasCarrera.h"
#include "Camaras/GestorCamaras.h"
#include "Regiones/Planeta1/CircuitoCarreras.h"
#include "Regiones/Planeta1/ControlCarrera.h"
#include "Regiones/Planeta1/Distribucion.h"
#include "Regiones/Planeta1/GestorCarrera.h"
#include "Regiones/Planeta1/TerrenoPlaneta1.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
namespace {
void exigir(bool b, const char *m) {
    if (!b)
        throw std::runtime_error(m);
}
} // namespace
void verificarCarrera() {
    using C = CircuitoCarreras;
    float minx = 1e6f, maxx = -1e6f, minz = 1e6f, maxz = -1e6f;
    for (int i = 0; i < 5000; ++i) {
        float s = C::Longitud * i / 5000;
        auto m = C::muestra(s);
        minx = std::min(minx, m.posicion.x);
        maxx = std::max(maxx, m.posicion.x);
        minz = std::min(minz, m.posicion.z);
        maxz = std::max(maxz, m.posicion.z);
        auto pr = C::proyectar(m.posicion);
        exigir(pr.distancia < .001f, "Proyeccion centro");
        exigir(std::abs(std::remainder(pr.s - s, C::Longitud)) < .002f, "Proyeccion distancia");
        exigir(std::abs(glm::length(C::muestra(s, C::Ancho / 2).posicion -
                                    C::muestra(s, -C::Ancho / 2).posicion) -
                        C::Ancho) < .001f,
               "Ancho no es 14");
        auto g = m.posicion + C::origen();
        exigir(TerrenoPlaneta1::altura(g.x, g.z) < m.posicion.y - .5f,
               "Terreno atraviesa circuito");
    }
    exigir(std::abs(minx + 81) < .001f && std::abs(maxx - 81) < .001f &&
               std::abs(minz + 49.5f) < .001f && std::abs(maxz - 49.5f) < .001f,
           "Dimensiones de pista");
    exigir(C::rampa(0, -55) == 6 && C::rampa(0, 55) == 4 && C::rampa(-30, -55) == 0 &&
               C::rampa(30, 55) == 0,
           "Rampas exactas");
    exigir(std::abs(C::pendiente(-30, -55)) < .00001f && std::abs(C::pendiente(30, 55)) < .00001f,
           "Pendientes extremos");
    const float a = C::Pi * 12.5f;
    for (float s : {0.0f, 30.0f, 30 + a, 160 + a, 160 + 2 * a, 220 + 2 * a, 220 + 3 * a,
                    350 + 3 * a, 350 + 4 * a}) {
        auto antes = C::muestra(s * C::Escala - .0001f),
             despues = C::muestra(s * C::Escala + .0001f);
        exigir(glm::length(antes.posicion - despues.posicion) < .002f &&
                   glm::dot(antes.tangente, despues.tangente) > .9999f,
               "Discontinuidad curva/recta");
    }
    auto terreno = TerrenoPlaneta1::construir();
    exigir(terreno.size() == 240000, "Densidad del terreno");
    glm::vec2 minimo(1e6f), maximo(-1e6f);
    for (auto &v : terreno) {
        minimo = glm::min(minimo, glm::vec2(v.posicion.x, v.posicion.z));
        maximo = glm::max(maximo, glm::vec2(v.posicion.x, v.posicion.z));
    }
    exigir(minimo == glm::vec2(-300) && maximo == glm::vec2(300), "Terreno no mide 600x600");
    exigir(Distribucion::arboles.size() == 28, "Cantidad de arboles");
    for (auto &q : Distribucion::arboles) {
        exigir(!TerrenoPlaneta1::lago(q.x, q.y, 1), "Arbol en lago");
        exigir(!TerrenoPlaneta1::reservaPista(q.x, q.y, 3), "Arbol en pista");
    }
    // Camara suavizada con la altura real: comprueba un recorrido sobre relieve irregular.
    GestorCamaras camTerreno;
    camTerreno.alCambiarRegion(Region::Planeta1);
    for (int i = 0; i < 600; ++i) {
        float x = -230 + i * .75f, z = -190;
        camTerreno.seguir({x, TerrenoPlaneta1::altura(x, z) + 1, z}, {0, 0, -1}, 1.0f / 60,
                          TerrenoPlaneta1::altura);
        auto ojo = camTerreno.ojo();
        exigir(ojo.y >= TerrenoPlaneta1::altura(ojo.x, ojo.z) + 1.2f, "Camara atraviesa terreno");
    }
    std::cout << "BLOQUE_A_OK terreno600x600 celdas200x200 arboles28 camara600pasos\n";
    ControlCarrera reglas;
    reglas.reiniciar();
    for (int i = 0; i < 100; ++i) {
        reglas.actualizar(C::Longitud - .2f, true, 1);
        reglas.actualizar(.2f, true, 1);
    }
    exigir(reglas.vueltas() == 0, "Exploit de linea de meta");
    reglas.reiniciar();
    for (int i = 1; i <= int(std::ceil(C::Longitud * 3 * 10)) + 2; ++i)
        reglas.actualizar(C::envolver(i * .1f), true, .2f);
    exigir(reglas.terminada(), "Secuencia completa de puertas");
    reglas.reiniciar();
    for (int i = 1; i <= 24; ++i)
        reglas.actualizar(C::envolver(i * C::Longitud / 8 + .1f), true, 1);
    exigir(reglas.vueltas() == 0, "Teletransporte acredita vueltas");
    reglas.reiniciar();
    reglas.actualizar(10, false, 12);
    reglas.recuperar();
    exigir(reglas.puntoSeguro() == 0, "Recuperacion incorrecta");
    GestorCamaras cam;
    for (int k = 0; k < 4; ++k) {
        cam.alCambiarRegion(Region::Planeta1);
        cam.orbitar(k * C::Pi / 2);
        auto v = cam.movimientoRelativo({0, 1});
        exigir(glm::length(v - glm::vec2(std::sin(k * C::Pi / 2), -std::cos(k * C::Pi / 2))) <
                   .0001f,
               "W no relativo a camara");
        exigir(glm::length(cam.movimientoRelativo({1, 1})) < 1.0001f, "Diagonal sin normalizar");
    }
    GestorCarrera g;
    OrdenActividad o;
    o.avatar = {0, 0, 0};
    o.interactuar = true;
    g.controlar(o);
    exigir(!g.ocupado(), "Abordaje lejano");
    o.avatar = C::muestra(0).posicion + C::origen();
    g.controlar(o);
    exigir(g.ocupado(), "Abordaje cercano");
    o.interactuar = false;
    o.acelerador = 1;
    g.controlar(o);
    for (int i = 0; i < 120; ++i)
        g.actualizar(1.0f / 60);
    exigir(g.nave().velocidad() == 0, "Salida antes de cuenta regresiva");
    GestorCarrera h;
    h.controlar(OrdenActividad{0, 0, 0, true, false, false, C::muestra(0).posicion + C::origen()});
    for (int i = 0; i < 60; ++i)
        g.actualizar(1.0f / 60);
    for (int i = 0; i < 180; ++i)
        h.actualizar(1.0f / 60);
    o.giro = .15f;
    g.controlar(o);
    h.controlar(o);
    for (int i = 0; i < 120; ++i)
        g.actualizar(1.0f / 60);
    for (int i = 0; i < 60; ++i)
        h.actualizar(1.0f / 30);
    exigir(glm::length(g.nave().posicion() - h.nave().posicion()) < .02f,
           "Dinamica depende de FPS");
    o.recuperar = true;
    g.controlar(o);
    exigir(g.nave().velocidad() == 0 && g.recuperacionRestante() >= 1.99f,
           "Recuperacion sin penalizacion");
    o.recuperar = false;
    o.reiniciar = true;
    g.controlar(o);
    exigir(g.fase() == FaseCarrera::CuentaAtras && g.tiempo() == 0, "Reinicio incorrecto");

    // Los rivales que ya terminaron deben conservar su puesto al llegar el jugador despues.
    GestorCarrera tardia;
    OrdenActividad mando;
    mando.avatar = C::muestra(0).posicion + C::origen();
    mando.interactuar = true;
    tardia.controlar(mando);
    mando.interactuar = false;
    tardia.controlar(mando);
    for (int i = 0; i < 900; ++i)
        tardia.actualizar(.1f);
    for (int i = 0; i < 24000 && tardia.fase() != FaseCarrera::Terminada; ++i) {
        const auto &n = tardia.nave();
        auto pr = C::proyectar(n.posicion());
        auto d = C::muestra(pr.s + 12).posicion - n.posicion();
        float error = std::remainder(
            std::atan2(d.x, -d.z) - std::atan2(n.direccion().x, -n.direccion().z), 2 * C::Pi);
        mando.giro = std::clamp(error * 3.5f, -1.0f, 1.0f);
        float meta = std::abs(error) > .19f ? 24.0f : 34.0f;
        mando.acelerador = n.velocidad() < meta ? 1.0f : 0.0f;
        mando.freno = n.velocidad() > meta + 1 ? .25f : 0.0f;
        tardia.controlar(mando);
        tardia.actualizar(1.0f / 60);
    }
    exigir(tardia.fase() == FaseCarrera::Terminada && tardia.posicion() == 3,
           "Resultado ignora rivales ya finalizados");
    std::cout << "RESULTADO_TERCER_LUGAR_OK llegada posterior a ambos rivales\n";
    auto datos = C::construir();
    for (size_t i = 0; i + 2 < datos.size(); i += 3) {
        auto cruz = glm::cross(datos[i + 1].posicion - datos[i].posicion,
                               datos[i + 2].posicion - datos[i].posicion);
        exigir(glm::dot(cruz, datos[i].normal) > 0, "Normales o winding incorrectos");
    }
    std::cout << "PRUEBAS_CARRERA_OK geometria 5000 muestras, rampas, uniones, normales, "
                 "antitrampa, reinicio, recuperacion, camara y dt\n";
}
