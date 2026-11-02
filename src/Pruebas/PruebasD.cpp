#include "Nucleo/Aplicacion.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Planeta1/CircuitoCarreras.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
void Aplicacion::probarD(const std::filesystem::path &dir) {
    auto exigir = [](bool ok, const char *texto) {
        if (!ok)
            throw std::runtime_error(texto);
    };
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    auto &s = p.sistemas();
    auto original = s;
    exigir(s.ciclo == 120.f, "D: configuracion distinta de 120 s");
    s.faseInicial = 0;
    int dia = 0, noche = 0;
    for (int k = 0; k < 12000; ++k) {
        s.tiempo = (float(k) + .5f) * .01f;
        auto l = s.luces();
        if (l.sol.y > 0)
            ++dia;
        else
            ++noche;
        for (int i = 0; i < 2; ++i) {
            auto &q = l.puntuales[6 + i];
            exigir(glm::length(q.posicion - (s.sentinel(i) + glm::vec3(0, -.3f, 0))) < .001f,
                   "D: luz separada de Sentinel");
            exigir(q.intensidad / (q.atenuacion.x + q.atenuacion.y * 50 + q.atenuacion.z * 2500) <
                       .03f,
                   "D: luz Sentinel demasiado extensa");
        }
    }
    exigir(dia == 8000 && noche == 4000, "D: duraciones solares incorrectas");
    for (float f : {1.f / 3, 5.f / 6}) {
        s.tiempo = 0;
        s.faseInicial = f;
        auto l = s.luces();
        for (auto &q : l.puntuales)
            exigir(q.encendida == (f > .7f), "D: encendido nocturno incorrecto");
    }
    std::cout << "D_CICLO_OK dia=80 noche=40 total=120; 12000 muestras y 8 luces sincronizadas\n";
    ModeloGltf banshee(recursos_ / "Modelos/Banshee.gltf"), ghost(recursos_ / "Modelos/Ghost.gltf");
    for (auto *modelo : {&banshee, &ghost}) {
        auto &v = modelo->triangulos();
        exigir(v.size() > 3000, "D: modelo de carrera incompleto");
        for (float carril : {-3.f, 0.f, 3.f})
            for (int i = 0; i < 400; ++i) {
                auto muestra =
                    CircuitoCarreras::muestra(CircuitoCarreras::Longitud * i / 400, carril);
                auto m = NaveCarreras::orientar(muestra.posicion, muestra.tangente, muestra.normal);
                for (auto q : v) {
                    auto t = glm::vec3(m * glm::vec4(q, 1));
                    auto pr = CircuitoCarreras::proyectar(t);
                    exigir(pr.distancia < CircuitoCarreras::Ancho / 2 + .04f,
                           "D: vehiculo fuera del ancho");
                    exigir(t.y > CircuitoCarreras::muestra(pr.s).posicion.y - .35f,
                           "D: vehiculo enterrado");
                }
            }
    }
    std::cout << "D_MODELOS_OK Banshee y Ghost: ancho y apoyo, 1200 posiciones por modelo\n";
    GestorCarrera carrera;
    OrdenActividad orden;
    orden.avatar = carrera.nave().posicion() + CircuitoCarreras::origen();
    orden.interactuar = true;
    carrera.controlar(orden);
    auto mensajes = carrera.estado().mensajes;
    exigir(mensajes.size() == 3, "D: HUD de inicio excesivo");
    for (auto &l : mensajes)
        exigir(l.find("VELOCIDAD") == std::string::npos &&
                   l.find("POSICION") == std::string::npos &&
                   l.find("CHECKPOINT") == std::string::npos,
               "D: HUD sin simplificar");
    for (int i = 0; i < 600; ++i)
        carrera.actualizar(1.f / 120);
    exigir(carrera.estado().mensajes.size() == 2, "D: HUD de carrera excesivo");
    s = original;
    s.tiempo = 0;
    s.faseInicial = 1.f / 3;
    avatar_.colocar({-95, 0, -30}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    for (int j = 0; j < 120; ++j) {
        avatar_.mover({0, 0}, 1.f / 60, p);
        actualizarCamara(1.f / 60);
    }
    dibujar(dir / "D_Engineer.ppm");
    foco_ = true;
    dibujar(dir / "D_Foco_dia.ppm");
    s.faseInicial = 5.f / 6;
    dibujar(dir / "D_Foco_noche.ppm");
    foco_ = false;
    camaras_.mirar({0, -1000});
    for (int j = 0; j < 120; ++j)
        actualizarCamara(1.f / 60);
    auto ojo = camaras_.ojo();
    auto vista = camaras_.vista({0, 0, 0});
    auto frente = -glm::vec3(glm::inverse(vista)[2]);
    exigir(frente.y > .3f, "D: camara no mira arriba realmente");
    exigir(ojo.y >= p.altura(ojo.x, ojo.z) + 1.24f, "D: camara bajo suelo");
    int w, h;
    ventana_.dimensiones(w, h);
    auto punto = camaras_.proyeccion(float(w) / h) * vista *
                 glm::vec4(avatar_.posicion() + glm::vec3(0, .8f, 0), 1);
    punto /= punto.w;
    exigir(std::abs(punto.x) < 1 && std::abs(punto.y) < 1, "D: avatar perdido al mirar arriba");
    dibujar(dir / "D_Cielo_noche.ppm");
    s.faseInicial = 1.f / 3;
    dibujar(dir / "D_Cielo_dia.ppm");
    std::cout << "D_CAMARA_OK frente_y=" << frente.y << " avatar_ndc_y=" << punto.y
              << " suelo respetado\n";
    s.tiempo = 0;
    s.faseInicial = 5.f / 6;
    auto objetivo = s.sentinel(0);
    auto ojoSent = objetivo + glm::vec3(6, -2, 10);
    auto vp =
        camaras_.proyeccion(float(w) / h) * glm::lookAt(ojoSent, objetivo, glm::vec3(0, 1, 0));
    ContextoEscena contexto{renderizador_, recursos_};
    for (int encendida = 0; encendida < 2; ++encendida) {
        auto amb = p.ambiente();
        if (!encendida) {
            amb.iluminacion.puntuales[6].encendida = false;
            amb.iluminacion.puntuales[7].encendida = false;
        }
        renderizador_.iniciar(w, h, vp, amb, ojoSent);
        p.dibujar(contexto, glm::mat4(1));
        renderizador_.capturar(dir / (encendida ? "D_Sentinel_luz.ppm" : "D_Sentinel_sin_luz.ppm"),
                               w, h);
        ventana_.presentar();
    }
    s = original;
    camaras_.alCambiarRegion(Region::Planeta1);
    std::cout << "D_HUD_OK vuelta tiempo inicio fin; mecanismos carrera intactos\n";
}
