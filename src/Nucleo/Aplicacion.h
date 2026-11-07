#pragma once
#include "Audio/Audio.h"
#include "Camaras/GestorCamaras.h"
#include "Graficos/Interfaz.h"
#include "Graficos/Renderizador.h"
#include "Mundo/GestorEscenarios.h"
#include "Mundo/MundoEspacial.h"
#include "Nucleo/Entradas.h"
#include "Nucleo/Ventana.h"
#include "Personajes/Avatar.h"
// Coordina sistemas. El orden de miembros asegura destruir GPU antes del contexto GLFW.
class Aplicacion {
    Ventana ventana_;
    std::filesystem::path recursos_;
    Renderizador renderizador_;
    MundoEspacial mundo_;
    GestorEscenarios escenarios_;
    GestorCamaras camaras_;
    Avatar avatar_;
    struct MemoriaRegion {Avatar::Estado avatar;GestorCamaras camara;bool foco,cohete;};
    std::map<Region,MemoriaRegion> memoriaRegiones_;
    bool regionPreparada_=false;
    Entradas entradas_;
    Interfaz interfaz_;
    Audio audio_;
    unsigned sonidoPuerta_ = 0;
    int faseSonora_ = -1;
    void actualizarAudio();
    bool foco_ = false, observandoCohete_ = false;
    float deltaCuadro_ = 1.0f / 60;
    std::string aviso_;
    explicit Aplicacion(const std::filesystem::path &recursos);
    void cambiarRegion(Region region);
    void solicitarCamara(ModoCamara modo);
    void procesarEntradas(float dt);
    void aplicarOrden(const OrdenActividad &orden);
    void probarFinalPlaneta1(const std::filesystem::path &dir);
    void probarE(const std::filesystem::path &dir);
    void probarD(const std::filesystem::path &dir);
    void probarC(const std::filesystem::path &evidencias);
    void probarB2(const std::filesystem::path &evidencias);
    void probarCarrera(const std::filesystem::path &evidencias);
    void actualizarCamara(float dt);
    void dibujar(const std::filesystem::path &captura = {});
    int ciclo(bool pruebas, const std::filesystem::path &evidencias, Region inicial);

  public:
    // Analiza opciones, resuelve recursos junto al EXE y devuelve 0 en cierre limpio, 1 en error.
    static int ejecutar(int argc, char **argv) noexcept;
};
