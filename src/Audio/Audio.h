#pragma once
#include <filesystem>
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
class Audio {
    struct MotorAudio;
    std::unique_ptr<MotorAudio> motor_;

  public:
    enum Sonido {
        Musica,
        Ambiente,
        Puerta,
        Cohete,
        Motor,
        Inicio,
        Final,
        Covenant,
        MusicaEspacio,
        MusicaPlaneta2,
        AmbienteEspacio,
        AmbientePlaneta2,
        Trafico,
        Boost,
        Cantidad
    };
    struct EstadoVoz {
        float objetivo;
        size_t cursor;
        bool activa;
        unsigned inicios;
        float ganancia;
    };
    explicit Audio(const std::filesystem::path &, bool sinDispositivo = false);
    ~Audio();
    static float atenuacionDistancia(float distancia);
    EstadoVoz estado(Sonido);
    void bucle(Sonido, float);
    void disparar(Sonido, float = .6f);
    void detenerEfectos();
    void silenciar();
    void volumen(float incremento);
    void siguienteCanal();
    std::string controlVolumen() const;
    void oyente(glm::vec3 posicion, glm::vec3 frente);
    void fuente(Sonido, glm::vec3 posicion, bool espacial = true);
    bool disponible() const;
    unsigned bloques() const;
    // Sólo el motor sin dispositivo permite extraer PCM para pruebas reproducibles.
    std::vector<float> leerPCM(unsigned cuadros);
};
