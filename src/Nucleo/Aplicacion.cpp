#include "Nucleo/Aplicacion.h"
#include "Mundo/RegistroEscenarios.h"
#include "Pruebas/Verificacion.h"
#include "Regiones/Planeta1/CircuitoCarreras.h"
#include "Regiones/Planeta1/Planeta1.h"
#include "Regiones/Espacio/Espacio.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
#endif
namespace {
// Resuelve los recursos junto al ejecutable, independientemente del directorio de trabajo.
std::filesystem::path recursosJuntoAlEjecutable(const char *argumento) {
#ifdef _WIN32
    wchar_t ruta[32768];
    auto n = GetModuleFileNameW(nullptr, ruta, 32768);
    if (!n || n >= 32768)
        throw std::runtime_error("Ruta de ejecutable no disponible");
    (void)argumento;
    return std::filesystem::path(std::wstring(ruta, n)).parent_path() / "Recursos";
#else
    return std::filesystem::absolute(argumento).parent_path() / "Recursos";
#endif
}
} // namespace
Aplicacion::Aplicacion(const std::filesystem::path &r) : recursos_(r), renderizador_(r), audio_(r) {
    registrarEscenarios(escenarios_);
    cambiarRegion(Region::Planeta1);
}
void Aplicacion::cambiarRegion(Region r) {
    ContextoEscena c{renderizador_, recursos_};
    if (regionPreparada_) {
        if (escenarios_.actual().id() == r)
            return;
        memoriaRegiones_[escenarios_.actual().id()] = {avatar_.guardar(), camaras_, foco_,
                                                       observandoCohete_};
    }
    if (escenarios_.activar(r, c)) {
        foco_ = false;
        observandoCohete_ = false;
        audio_.detenerEfectos();
        sonidoPuerta_ = 0;
        faseSonora_ = -1;
        avatar_.aparecer(escenarios_.actual());
        camaras_.alCambiarRegion(r);
        if (memoriaRegiones_.count(r)) {
            auto &prev = memoriaRegiones_.at(r);
            avatar_.restaurar(prev.avatar);
            camaras_ = prev.camara;
            foco_ = prev.foco;
            observandoCohete_ = prev.cohete;
        }
        // Siempre se aborda la nave al entrar a Espacio; V alterna la vista exterior.
        if (r == Region::Espacio) {
            camaras_.solicitar(ModoCamara::Viaje, r);
            if (!camaras_.cabina()) camaras_.alternarCabina();
        }
        regionPreparada_ = true;
        aviso_ = "Region preparada";
    }
}

void Aplicacion::solicitarCamara(ModoCamara m) {
    foco_ = false;
    observandoCohete_ = false;
    auto estado = escenarios_.actual().actividad();
    if (escenarios_.actual().id() == Region::Planeta1 && estado.ocupado &&
        m != ModoCamara::Carrera) {
        aviso_ = "Desciende con X antes de cambiar camara";
        return;
    }
    if (m == ModoCamara::Carrera && !estado.ocupado) {
        aviso_ = "Acercate a la nave y aborda con X";
        return;
    }
    aviso_ =
        camaras_.solicitar(m, escenarios_.actual().id()) ? "Modo actualizado" : "Modo incompatible";
}
void Aplicacion::aplicarOrden(const OrdenActividad &orden) {
    auto &e = escenarios_.actual();
    auto antes = e.actividad();
    e.controlar(orden);
    auto despues = e.actividad();
    if (despues.ocupado) {
        foco_ = false;
        observandoCohete_ = false;
    }
    if (e.id() == Region::Espacio) {
        auto destino = dynamic_cast<Espacio &>(e).consumirDestino();
        if (destino)
            cambiarRegion(*destino);
        return;
    }
    if (!antes.ocupado && despues.ocupado)
        camaras_.solicitar(ModoCamara::Carrera, e.id());
    if (antes.ocupado && !despues.ocupado) {
        auto derecha = glm::vec3(-antes.direccion.z, 0, antes.direccion.x);
        avatar_.colocar(antes.objetivo + derecha * 4.5f, e);
        camaras_.solicitar(ModoCamara::Exploracion, e.id());
    }
}
void Aplicacion::procesarEntradas(float dt) {
    entradas_.actualizar(ventana_.nativa());
    if (entradas_.pulsada(GLFW_KEY_ESCAPE))
        ventana_.cerrar();
    for (int i = 0; i < 3; ++i)
        if (entradas_.pulsada(GLFW_KEY_1 + i))
            cambiarRegion(Region(i));
    if (entradas_.pulsada(GLFW_KEY_M))
        audio_.silenciar();
    if (entradas_.pulsada(GLFW_KEY_MINUS))
        audio_.volumen(-.1f);
    if (entradas_.pulsada(GLFW_KEY_EQUAL))
        audio_.volumen(.1f);
    bool pilotando = escenarios_.actual().id() == Region::Espacio &&
                      camaras_.modo() == ModoCamara::Viaje;
    if (pilotando && entradas_.pulsada(GLFW_KEY_V)) camaras_.alternarCabina();
    if (!pilotando) camaras_.mirar(entradas_.raton());
    camaras_.orbitar(entradas_.orbita() * dt);
    OrdenActividad o;
    o.avatar = avatar_.posicion();
    o.acelerador = entradas_.mantenida(GLFW_KEY_W) ? 1.0f : 0.0f;
    o.freno = entradas_.mantenida(GLFW_KEY_S) ? 1.0f : 0.0f;
    o.giro = float(entradas_.mantenida(GLFW_KEY_D)) - float(entradas_.mantenida(GLFW_KEY_A));
    o.vertical = float(entradas_.mantenida(GLFW_KEY_Q)) - float(entradas_.mantenida(GLFW_KEY_Z));
    o.confirmar = entradas_.pulsada(GLFW_KEY_ENTER);
    if (pilotando) o.mirada = entradas_.raton();
    if (escenarios_.actual().id() == Region::Espacio && !pilotando)
        o.acelerador = o.freno = o.giro = o.vertical = 0;
    o.interactuar = entradas_.pulsada(GLFW_KEY_X);
    o.reiniciar = entradas_.pulsada(GLFW_KEY_R);
    o.recuperar = entradas_.pulsada(GLFW_KEY_C);
    o.puerta = entradas_.pulsada(GLFW_KEY_E);
    o.lanzar = entradas_.pulsada(GLFW_KEY_L);
    o.reiniciarCohete = entradas_.pulsada(GLFW_KEY_T);
    o.reaccion = entradas_.pulsada(GLFW_KEY_G);
    o.titan = entradas_.pulsada(GLFW_KEY_B);
    o.hunter = entradas_.pulsada(GLFW_KEY_H);
    o.saludo = entradas_.pulsada(GLFW_KEY_J);
    o.reflector = entradas_.pulsada(GLFW_KEY_P);
    o.inspeccion = entradas_.pulsada(GLFW_KEY_K);
    bool explorar = (escenarios_.actual().id() == Region::Planeta1 ||
                     escenarios_.actual().id() == Region::Planeta2) &&
                    !escenarios_.actual().actividad().ocupado &&
                    camaras_.modo() == ModoCamara::Exploracion;
    if (entradas_.pulsada(GLFW_KEY_O) && explorar &&
        escenarios_.actual().id() == Region::Planeta1) {
        observandoCohete_ = !observandoCohete_;
        foco_ = false;
    }
    if (entradas_.pulsada(GLFW_KEY_F) && explorar && !observandoCohete_)
        foco_ = !foco_;
    if (!explorar)
        foco_ = false;
    aplicarOrden(o);
    if (!observandoCohete_ && !escenarios_.actual().actividad().ocupado &&
        camaras_.modo() == ModoCamara::Exploracion)
        avatar_.mover(camaras_.movimientoRelativo(entradas_.movimiento()), dt, escenarios_.actual(),
                      entradas_.mantenida(GLFW_KEY_LEFT_SHIFT) ||
                          entradas_.mantenida(GLFW_KEY_RIGHT_SHIFT));
}
void Aplicacion::actualizarCamara(float dt) {
    auto &e = escenarios_.actual();
    auto r = e.id();
    auto actividad = e.actividad();
    if (r == Region::Espacio) {
        dynamic_cast<Espacio &>(e).mostrarNave(!(camaras_.modo() == ModoCamara::Viaje &&
                                                camaras_.cabina()));
        camaras_.seguirEspacio(mundo_.aGlobal(r, actividad.objetivo), actividad.direccion,
                               mundo_.aGlobal(r, {0, 0, 0}), dt);
        return;
    }
    if (observandoCohete_ && r == Region::Planeta1 && !actividad.ocupado) {
        auto &p = dynamic_cast<Planeta1 &>(e);
        camaras_.encuadrarEvento(mundo_.aGlobal(r, p.sistemas().objetivoObservacion()));
        return;
    }
    glm::vec3 objetivo = actividad.ocupado ? actividad.objetivo : avatar_.posicion();
    glm::vec3 direccion = actividad.direccion;
    if (camaras_.modo() == ModoCamara::Viaje)
        objetivo = {-11, .3f, 9};
    if (camaras_.modo() == ModoCamara::Observacion)
        objetivo = {0, 4, 0};
    auto alturaGlobal = [&](float x, float z) {
        auto p = mundo_.aLocal(r, {x, 0, z});
        return mundo_.aGlobal(r, {p.x, e.altura(p.x, p.z), p.z}).y;
    };
    auto direccionGlobal =
        glm::normalize(glm::vec3(mundo_.transformacion(r) * glm::vec4(direccion, 0)));
    camaras_.seguir(mundo_.aGlobal(r, objetivo), direccionGlobal, dt, alturaGlobal);
}
void Aplicacion::dibujar(const std::filesystem::path &captura) {
    actualizarAudio();
    int a, b;
    ventana_.dimensiones(a, b);
    if (a <= 0 || b <= 0)
        return;
    auto &e = escenarios_.actual();
    auto r = e.id();
    auto actividad = e.actividad();
    actualizarCamara(deltaCuadro_);
    auto vp = camaras_.proyeccion(float(a) / b) * camaras_.vista(glm::vec3(0));
    auto ambiente = e.ambiente();
    auto &il = ambiente.iluminacion;
    // Las regiones trabajan en coordenadas locales; pasar luces a mundo para el shader.
    if (r != Region::Planeta1) {
        for (auto &p : il.puntuales)
            if (p.encendida) p.posicion = mundo_.aGlobal(r, p.posicion);
        if (il.foco) il.focoPos = mundo_.aGlobal(r, il.focoPos);
    }
    // Spotlight fijo del hangar; el Engineer solo usa la puntual corporal.
    auto &energia = il.puntuales[8];
    energia.encendida = foco_ && !observandoCohete_ && !actividad.ocupado &&
                        (r == Region::Planeta1 || r == Region::Planeta2) &&
                        camaras_.modo() == ModoCamara::Exploracion;
    energia.posicion = mundo_.aGlobal(r, avatar_.posicion() + glm::vec3(0, 1.1f, 0));
    energia.color = {.12f, .65f, 1.f};
    energia.intensidad = r == Region::Planeta2 ? .75f : 1.65f;
    energia.atenuacion = {1, .09f, .025f};
    renderizador_.iniciar(a, b, vp, ambiente, camaras_.ojo());
    ContextoEscena c{renderizador_, recursos_};
    e.dibujar(c, mundo_.transformacion(r));
    if (r != Region::Espacio && !actividad.ocupado)
        avatar_.dibujar(c, mundo_.transformacion(r), energia.encendida);
    auto lineas = actividad.mensajes;
    if (r == Region::Planeta1 && !actividad.ocupado)
        lineas.push_back(observandoCohete_
                             ? "O / VOLVER A EXPLORACION"
                             : "F ENERGIA CORPORAL / O COHETE / M SILENCIO / - + VOLUMEN");
    if (lineas.empty() && r == Region::Planeta2)
        lineas = {"PLANETA 2 / KERWAN + WARFRAME / APHELION Y RAILJACK",
                  "WASD MOVER / SHIFT CORRER / M SILENCIO / 1-3 REGIONES"};
    if (lineas.empty())
        lineas = {std::string(e.nombre()) + " / " + camaras_.nombre(),
                  "WASD MOVER / 1-3 REGIONES"};
    interfaz_.preparar(lineas, a, b, !actividad.ocupado);
    renderizador_.interfaz(interfaz_.malla(), a, b);
    if (!captura.empty())
        renderizador_.capturar(captura, a, b);
    Renderizador::comprobarErrores();
    ventana_.titulo("PROYECTO ESPACIAL");
    ventana_.presentar();
}
int Aplicacion::ciclo(bool pruebas, const std::filesystem::path &evidencias, Region inicial) {
    cambiarRegion(inicial);
    if (pruebas) {
        ContextoEscena c{renderizador_, recursos_};
        verificarContratos(c);
        auto *ventanaOriginal = ventana_.nativa();
        // Nueve transiciones y cuatro solicitudes de modo por region; renderizado real.
        for (int vuelta = 0; vuelta < 3; ++vuelta)
            for (int i = 0; i < 3; ++i) {
                cambiarRegion(Region(i));
                if (ventana_.nativa() != ventanaOriginal)
                    throw std::runtime_error("Se reemplazo ventana");
                if (vuelta == 0) {
                    ventana_.eventos();
                    dibujar(evidencias / (std::string(escenarios_.actual().nombre()) + ".ppm"));
                }
                for (int m = 0; m < 4; ++m) {
                    solicitarCamara(ModoCamara(m));
                    escenarios_.actual().actualizar(1.0f / 60);
                    ventana_.eventos();
                    dibujar();
                    if (!renderizador_.dibujos())
                        throw std::runtime_error("Region sin geometria");
                }
            }
        cambiarRegion(Region::Planeta1);
        dibujar();
        std::cout << "INTEGRACION_OK 9 transiciones, 36 intentos camara, misma ventana, shaders "
                     "y GL sin errores\n";
        probarCarrera(evidencias);
        probarEspacio(evidencias);
        escenarios_.liberar();
        Renderizador::comprobarErrores();
        return 0;
    }
    double antes = glfwGetTime();
    while (ventana_.abierta()) {
        ventana_.eventos();
        double ahora = glfwGetTime();
        float dt = std::clamp(float(ahora - antes), 0.0f, .05f);
        antes = ahora;
        deltaCuadro_ = dt;
        procesarEntradas(dt);
        escenarios_.actual().actualizar(dt);
        dibujar();
    }
    return 0;
}
int Aplicacion::ejecutar(int argc, char **argv) noexcept {
    try {
        bool pruebas = false, capturas = true;
        Region inicial = Region::Planeta1;
        std::filesystem::path evidencias = "EvidenciasPrueba";
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            if (a == "--pruebas")
                pruebas = true;
            else if (a == "--sin-capturas")
                capturas = false;
            else if (a == "--evidencias" && i + 1 < argc)
                evidencias = argv[++i];
            else if (a == "--region" && i + 1 < argc) {
                std::string r = argv[++i];
                if (r == "planeta1")
                    inicial = Region::Planeta1;
                else if (r == "planeta2")
                    inicial = Region::Planeta2;
                else if (r == "espacio")
                    inicial = Region::Espacio;
                else
                    throw std::invalid_argument("Region desconocida");
            } else
                throw std::invalid_argument(
                    "Opciones: --region planeta1|planeta2|espacio --pruebas --sin-capturas --evidencias carpeta");
        }
        {
            Aplicacion app(recursosJuntoAlEjecutable(argv[0]));
            app.renderizador_.permitirCapturas(capturas);
            int estado = app.ciclo(pruebas, evidencias, inicial);
            if (estado)
                return estado;
        }
        std::cout << "CIERRE CORRECTO\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}

void Aplicacion::probarCarrera(const std::filesystem::path &evidencias) {
    cambiarRegion(Region::Planeta1);
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    probarB2(evidencias);
    probarC(evidencias);
    probarD(evidencias);
    probarE(evidencias);
    // Prueba de conduccion: el controlador de prueba produce W/S/A/D analogicos, nunca fija
    // posiciones de la nave. No existe piloto automatico del jugador en el modo interactivo.
    avatar_.colocar({205, 0, -35}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    camaras_.orbitar(CircuitoCarreras::Pi);
    for (int i = 0; i < 376; ++i) {
        avatar_.mover(camaras_.movimientoRelativo({0, 1}), 1.0f / 120, p);
        actualizarCamara(1.0f / 120);
        if (i % 60 == 0) {
            ventana_.eventos();
            dibujar();
        }
        if (i == 256)
            dibujar(evidencias / "Acceso_plataforma.ppm");
    }
    for (int i = 0; i < 40; ++i)
        dibujar();
    dibujar(evidencias / "Avatar_sobre_plataforma.ppm");
    if (avatar_.posicion().z < 10 || avatar_.posicion().y < 6)
        throw std::runtime_error("Evidencia no llego a plataforma");
    avatar_.colocar({220, 0, 80}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    camaras_.orbitar(CircuitoCarreras::Pi);
    for (int i = 0; i < 1000; ++i) {
        avatar_.mover(camaras_.movimientoRelativo({0, 1}), 1.0f / 120, p);
        actualizarCamara(1.0f / 120);
    }
    for (int i = 0; i < 40; ++i)
        dibujar();
    dibujar(evidencias / "Exterior_pista.ppm");
    if (avatar_.posicion().z < 179)
        throw std::runtime_error("Recorrido exterior bloqueado");
    camaras_.alCambiarRegion(Region::Planeta1);
    avatar_.colocar({198, 3, 139}, p);
    OrdenActividad o;
    o.avatar = avatar_.posicion();
    o.interactuar = true;
    aplicarOrden(o);
    if (!p.actividad().ocupado || camaras_.modo() != ModoCamara::Carrera)
        throw std::runtime_error("Abordaje/camara fallo");
    dibujar(evidencias / "Cuenta_regresiva.ppm");
    o.interactuar = false;
    aplicarOrden(o);
    for (int i = 0; i < 240; ++i)
        p.actualizar(1.0f / 60);
    o.recuperar = true;
    aplicarOrden(o);
    for (int i = 0; i < 40; ++i)
        dibujar();
    dibujar(evidencias / "Penalizacion_2s.ppm");
    o.recuperar = false;
    o.reiniciar = true;
    aplicarOrden(o);
    o.reiniciar = false;
    bool rampa6 = false, rampa4 = false;
    for (int cuadro = 0; cuadro < 24000 && p.carrera().fase() != FaseCarrera::Terminada; ++cuadro) {
        auto &n = p.carrera().nave();
        auto pr = CircuitoCarreras::proyectar(n.posicion());
        auto objetivo = CircuitoCarreras::muestra(pr.s + 12).posicion;
        auto d = objetivo - n.posicion();
        float deseado = std::atan2(d.x, -d.z),
              actual = std::atan2(n.direccion().x, -n.direccion().z);
        float error = std::remainder(deseado - actual, 2 * CircuitoCarreras::Pi);
        o.giro = std::clamp(error * 3.5f, -1.0f, 1.0f);
        float meta = std::abs(error) > .19f ? 24.0f : 34.0f;
        o.acelerador = n.velocidad() < meta ? 1.0f : 0.0f;
        o.freno = n.velocidad() > meta + 1 ? 0.25f : 0.0f;
        aplicarOrden(o);
        p.actualizar(1.0f / 60);
        deltaCuadro_ = 1.0f / 60;
        actualizarCamara(deltaCuadro_);
        if (cuadro % 60 == 0) {
            ventana_.eventos();
            dibujar();
        }
        if (!rampa6 && n.posicion().y > 8.8f) {
            for (int j = 0; j < 30; ++j)
                dibujar();
            dibujar(evidencias / "Rampa_6_unidades.ppm");
            rampa6 = true;
        }
        if (!rampa4 && n.posicion().z > 48 && n.posicion().y > 6.8f) {
            for (int j = 0; j < 30; ++j)
                dibujar();
            dibujar(evidencias / "Rampa_4_unidades.ppm");
            rampa4 = true;
        }
    }
    if (p.carrera().fase() != FaseCarrera::Terminada)
        throw std::runtime_error("La conduccion de prueba no completo tres vueltas: puertas " +
                                 std::to_string(p.carrera().reglas().vueltas()) + "/" +
                                 std::to_string(p.carrera().reglas().siguiente()));
    if (!rampa6 || !rampa4)
        throw std::runtime_error("No se recorrieron ambas rampas");
    for (int j = 0; j < 40; ++j)
        dibujar();
    dibujar(evidencias / "Resultado_carrera.ppm");
    std::cout << "CARRERA_COMPLETA_OK vueltas=3 checkpoints=24 rampas=2 tiempo="
              << p.carrera().tiempo() << " posicion=" << p.carrera().posicion() << "\n";
    o = {};
    o.avatar = avatar_.posicion();
    o.interactuar = true;
    aplicarOrden(o);
    if (p.actividad().ocupado || camaras_.modo() != ModoCamara::Exploracion)
        throw std::runtime_error("Descenso fallo");
    for (int j = 0; j < 40; ++j)
        dibujar();
    dibujar(evidencias / "Regreso_avatar.ppm");
    avatar_.colocar({-100, 0, -30}, p);
    camaras_.alCambiarRegion(Region::Planeta1);
    OrdenActividad vacia;
    vacia.avatar = avatar_.posicion();
    aplicarOrden(vacia);
    for (int j = 0; j < 40; ++j)
        dibujar();
    dibujar(evidencias / "Terreno_y_montanas.ppm");
    // Camaras de evidencia, solamente en --pruebas. No alteran el recorrido interactivo.
    struct Vista {
        const char *nombre;
        glm::vec3 ojo, objetivo;
    };
    const Vista vistas[] = {{"Plano_general", {430, 460, 480}, {0, 0, 0}},
                            {"Covenant", {-108, 18, -39}, {-145, 19, -100}},
                            {"Colonia_Titanfall", {25, 27, -47}, {68, 10, -119}},
                            {"Hangar", {196, 15, -45}, {165, 13, -100}},
                            {"Lago_y_bosque", {-77, 15, 99}, {-140, 6, 18}},
                            {"Recinto_NPC", {-15, 19, 207}, {-65, 3, 158}},
                            {"Plataforma", {244, 17, 58}, {205, 5, 15}},
                            {"Pista_reducida", {235, 85, 241}, {112, 0, 135}},
                            {"Montanas_desde_sendero", {-20, 8, -15}, {-20, 30, -290}}};
    ContextoEscena contexto{renderizador_, recursos_};
    int ancho, alto;
    ventana_.dimensiones(ancho, alto);
    for (auto &v : vistas) {
        auto vp = glm::perspective(glm::radians(60.0f), float(ancho) / alto, .15f, 1400.0f) *
                  glm::lookAt(v.ojo, v.objetivo, glm::vec3(0, 1, 0));
        ventana_.eventos();
        double inicioVista = glfwGetTime();
        renderizador_.iniciar(ancho, alto, vp, p.ambiente());
        p.dibujar(contexto, mundo_.transformacion(Region::Planeta1));
        std::string titulo = v.nombre;
        for (auto &letra : titulo)
            letra = letra == '_' ? ' ' : char(std::toupper(static_cast<unsigned char>(letra)));
        interfaz_.preparar(
            {std::string("BLOQUE B2 / ") + titulo, "CAPTURA REAL OPENGL / VISTA DE VERIFICACION"},
            ancho, alto);
        renderizador_.interfaz(interfaz_.malla(), ancho, alto);
        glFinish();
        std::cout << "VISTA_OK " << v.nombre << " dibujos=" << renderizador_.dibujos()
                  << " ms_muestra=" << (glfwGetTime() - inicioVista) * 1000 << "\n";
        renderizador_.capturar(evidencias / (std::string(v.nombre) + ".ppm"), ancho, alto);
        Renderizador::comprobarErrores();
        ventana_.presentar();
    }
    {
        int w, h;
        ventana_.dimensiones(w, h);
        auto vista = glm::perspective(glm::radians(48.0f), float(w) / h, .1f, 100.0f) *
                     glm::lookAt(glm::vec3(10, 12, 21), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        auto anterior = p.sistemas();
        p.sistemas().tiempo = 0;
        p.sistemas().faseInicial = 1.f / 3;
        renderizador_.iniciar(w, h, vista, p.ambiente(), {10, 12, 21});
        ModeloGltf banshee(recursos_ / "Modelos/Banshee.gltf"),
            ghost(recursos_ / "Modelos/Ghost.gltf");
        for (int i = 0; i < 3; ++i) {
            auto m = glm::translate(glm::mat4(1), {float(i - 1) * 7.f, 0, 0});
            (i == 0 ? banshee : ghost).dibujar(renderizador_, m);
        }
        renderizador_.capturar(evidencias / "Vehiculos_D.ppm", w, h);
        p.sistemas() = anterior;
        Renderizador::comprobarErrores();
        ventana_.presentar();
    }
}

void Aplicacion::actualizarAudio() {
    bool activo = escenarios_.actual().id() == Region::Planeta1;
    bool espacio = escenarios_.actual().id() == Region::Espacio;
    bool planeta2 = escenarios_.actual().id() == Region::Planeta2;
    audio_.bucle(Audio::Musica, activo ? .65f : 0);
    audio_.bucle(Audio::MusicaEspacio, espacio ? .80f : 0);
    audio_.bucle(Audio::MusicaPlaneta2, planeta2 ? .68f : 0);
    audio_.bucle(Audio::Ambiente, activo ? .7f : 0);
    if (!activo) {
        audio_.bucle(Audio::Motor, espacio ? .10f * dynamic_cast<Espacio &>(escenarios_.actual()).velocidad() / 40.f : 0);
        audio_.bucle(Audio::Cohete, 0);
        audio_.bucle(Audio::Covenant, 0);
        return;
    }
    auto &p = dynamic_cast<Planeta1 &>(escenarios_.actual());
    float distanciaCovenant = glm::length(avatar_.posicion() - glm::vec3(-145, 5, -63));
    audio_.bucle(Audio::Covenant, .65f / (1 + distanciaCovenant * distanciaCovenant / 225.f));
    audio_.bucle(Audio::Cohete, p.sistemas().motor()
                                    ? .8f * Audio::atenuacionDistancia(glm::length(
                                                avatar_.posicion() - p.sistemas().fuenteMotor()))
                                    : 0);
    audio_.bucle(Audio::Motor,
                 p.carrera().fase() == FaseCarrera::Corriendo
                     ? .2f + .5f * std::min(1.f, std::abs(p.carrera().nave().velocidad()) / 46)
                     : 0);
    if (p.eventoPuerta() != sonidoPuerta_) {
        sonidoPuerta_ = p.eventoPuerta();
        audio_.disparar(Audio::Puerta);
    }
    int fase = int(p.carrera().fase());
    if (fase != faseSonora_) {
        if (p.carrera().fase() == FaseCarrera::Corriendo)
            audio_.disparar(Audio::Inicio);
        if (p.carrera().fase() == FaseCarrera::Terminada)
            audio_.disparar(Audio::Final);
        faseSonora_ = fase;
    }
}
