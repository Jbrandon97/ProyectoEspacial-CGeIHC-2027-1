#pragma once
#include <array>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>
#include <windows.h>
#include <mmsystem.h>
// Mezclador PCM con hilo propio: una voz por evento, sin reinicios por fotograma.
class Audio {
    struct Voz {
        std::vector<short> datos;
        size_t cursor = 0;
        unsigned inicios = 0;
        float objetivo = 0, ganancia = 0;
        bool bucle = false, activa = false;
    };
    std::array<Voz, 8> voces_;
    HWAVEOUT salida_ = nullptr;
    std::thread hilo_;
    std::mutex mutex_;
    std::atomic<bool> salir_{false};
    std::atomic<unsigned> bloques_{0};
    float volumen_ = .35f;
    bool silencio_ = false;
    void ejecutar();

  public:
    enum Sonido { Musica, Ambiente, Puerta, Cohete, Motor, Inicio, Final, Covenant };
    explicit Audio(const std::filesystem::path &);
    ~Audio();
    // Distancia en unidades del mundo; a 45 unidades queda la mitad de energia.
    static float atenuacionDistancia(float distancia);
    struct EstadoVoz {
        float objetivo;
        size_t cursor;
        bool activa;
        unsigned inicios;
    };
    EstadoVoz estado(Sonido);
    void bucle(Sonido, float);
    void disparar(Sonido, float = .6f);
    void detenerEfectos();
    void silenciar();
    void volumen(float incremento);
    bool disponible() const {
        return salida_ != nullptr;
    }
    unsigned bloques() const {
        return bloques_;
    }
};
