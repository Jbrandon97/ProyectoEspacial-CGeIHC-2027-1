#include "Audio.h"
#define MA_NO_WINMM
#define MINIAUDIO_IMPLEMENTATION
#include "../../Dependencias/miniaudio/miniaudio.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
struct Audio::MotorAudio {
    ma_engine engine{};
    std::array<ma_sound_group, 3> grupos{};
    std::array<ma_sound, Cantidad> voces{};
    std::array<float, Cantidad> objetivos{};
    std::array<unsigned, Cantidad> inicios{};
    std::array<float, 4> volumenes{.35f, .8f, .65f, .8f};
    std::atomic<unsigned> bloques{0};
    std::mt19937 azar{std::random_device{}()};
    Sonido hangar = Cantidad;
    int cargadas = 0, gruposListos = 0, canal = 0;
    bool listo = false, offline = false, silencio = false;
    static int grupo(int i) {
        if (i == Musica || i == MusicaEspacio || i == MusicaPlaneta2)
            return 0;
        if (i == Ambiente || i == AmbienteEspacio || i == AmbientePlaneta2)
            return 1;
        return 2;
    }
    ~MotorAudio() {
        if (listo)
            ma_engine_stop(&engine);
        for (int i = 0; i < cargadas; ++i)
            ma_sound_uninit(&voces[i]);
        for (int i = 0; i < gruposListos; ++i)
            ma_sound_group_uninit(&grupos[i]);
        if (listo)
            ma_engine_uninit(&engine);
    }
};
Audio::Audio(const std::filesystem::path &r, bool offline)
    : motor_(std::make_unique<MotorAudio>()) {
    auto &m = *motor_;
    m.offline = offline;
    auto cfg = ma_engine_config_init();
    cfg.channels = 2;
    cfg.sampleRate = 44100;
    cfg.noDevice = offline;
    cfg.onProcess = [](void *usuario, float *, ma_uint64) {
        ++static_cast<MotorAudio *>(usuario)->bloques;
    };
    cfg.pProcessUserData = &m;
    if (ma_engine_init(&cfg, &m.engine) != MA_SUCCESS)
        throw std::runtime_error("miniaudio: no se pudo iniciar el motor/dispositivo");
    m.listo = true;
    if (!offline)
        std::cout << "MINIAUDIO_BACKEND "
                  << ma_get_backend_name(ma_engine_get_device(&m.engine)->pContext->backend)
                  << "\n";
    for (auto &g : m.grupos) {
        if (ma_sound_group_init(&m.engine, 0, nullptr, &g) != MA_SUCCESS)
            throw std::runtime_error("miniaudio: grupo no disponible");
        ++m.gruposListos;
    }
    const char *archivos[] = {"Exploration_Theme.wav", "Viento_instalaciones.wav",
                              "Hangar_Ahora.mp3", "Propulsion.wav",
                              "Motor_nave.wav", "Inicio_carrera.wav",
                              "Final_carrera.wav", "Energia_Covenant.wav",
                              "Airy.wav", "Our_Expanse_Chill.wav",
                              "Cabina_espacial.wav", "Instalaciones_Planeta2.wav",
                              "Motor_trafico.wav", "Boost.wav", "Hangar_Homero.mp3"};
    static_assert(sizeof archivos / sizeof *archivos == Cantidad);
    for (int i = 0; i < Cantidad; ++i) {
        auto path = (r / "Audio" / archivos[i]).wstring();
        if (ma_sound_init_from_file_w(&m.engine, path.c_str(), MA_SOUND_FLAG_DECODE,
                                      &m.grupos[MotorAudio::grupo(i)], nullptr,
                                      &m.voces[i]) != MA_SUCCESS)
            throw std::runtime_error(std::string("miniaudio: audio no disponible: ") + archivos[i]);
        ++m.cargadas;
        ma_sound_set_spatialization_enabled(&m.voces[i], MA_FALSE);
        ma_sound_set_volume(&m.voces[i], 1);
        ma_sound_set_fade_in_milliseconds(&m.voces[i], 0, 0, 0);
    }
    std::ifstream config(r / "Configuracion_C.txt");
    std::string linea;
    while (std::getline(config, linea))
        if (linea.rfind("volumen=", 0) == 0)
            try {
                float v = std::stof(linea.substr(8));
                if (std::isfinite(v))
                    m.volumenes[0] = std::clamp(v, 0.f, 1.f);
            } catch (...) {
            }
    ma_engine_set_volume(&m.engine, m.volumenes[0]);
    for (int i = 0; i < 3; ++i)
        ma_sound_group_set_volume(&m.grupos[i], m.volumenes[i + 1]);
    std::cout << "MINIAUDIO_OK 0.11.23 stereo44100 "
              << (offline ? "PCM sin dispositivo" : "dispositivo activo") << "\n";
}
Audio::~Audio() {
    std::cout << "MINIAUDIO_CIERRE bloques=" << motor_->bloques << "\n";
}
void Audio::fundirVoz(Sonido s, float v, unsigned ms) {
    auto &m = *motor_;
    if (std::abs(m.objetivos[s] - v) > 1e-5f) {
        ma_sound_set_fade_in_milliseconds(&m.voces[s], -1, v, ms);
        m.objetivos[s] = v;
    }
}
void Audio::bucle(Sonido s, float v) {
    auto &m = *motor_;
    auto &voz = m.voces[s];
    v = std::clamp(v, 0.f, 1.f);
    ma_sound_set_looping(&voz, MA_TRUE);
    if (v > 0 && !ma_sound_is_playing(&voz)) {
        ma_sound_seek_to_pcm_frame(&voz, 0);
        ma_sound_start(&voz);
        ++m.inicios[s];
    }
    fundirVoz(s, v, MotorAudio::grupo(s) == 0 ? 1200 : 80);
    if (v == 0 && ma_sound_get_current_fade_volume(&voz) < .0001f)
        ma_sound_stop(&voz);
}
void Audio::disparar(Sonido s, float v) {
    auto &m = *motor_;
    auto &voz = m.voces[s];
    m.objetivos[s] = std::clamp(v, 0.f, 1.f);
    ma_sound_stop(&voz);
    ma_sound_seek_to_pcm_frame(&voz, 0);
    ma_sound_set_looping(&voz, MA_FALSE);
    ma_sound_set_fade_in_milliseconds(&voz, 0, m.objetivos[s], 8);
    ma_sound_start(&voz);
    ++m.inicios[s];
}
namespace {
float volumenHangar(float distancia) {
    return .85f * Audio::atenuacionDistancia(distancia - 12.f, 22.f);
}
}
Audio::Sonido Audio::dispararHangar(float distancia) {
    detenerHangar();
    auto &m = *motor_;
    m.hangar = std::bernoulli_distribution(.5)(m.azar) ? HangarAhora : HangarHomero;
    disparar(m.hangar, volumenHangar(distancia));
    return m.hangar;
}
void Audio::actualizarHangar(float distancia) {
    auto &m = *motor_;
    if (m.hangar == Cantidad)
        return;
    if (ma_sound_at_end(&m.voces[m.hangar]))
        detenerHangar();
    else
        fundirVoz(m.hangar, volumenHangar(distancia), 100);
}
void Audio::detenerHangar() {
    auto &m = *motor_;
    if (m.hangar != Cantidad) {
        ma_sound_stop(&m.voces[m.hangar]);
        m.objetivos[m.hangar] = 0;
        m.hangar = Cantidad;
    }
}
void Audio::detenerEfectos() {
    detenerHangar();
    for (int i = 0; i < Cantidad; ++i)
        if (i != HangarAhora && i != HangarHomero && MotorAudio::grupo(i) == 2)
            bucle(Sonido(i), 0);
}
void Audio::silenciar() {
    auto &m = *motor_;
    m.silencio = !m.silencio;
    ma_engine_set_volume(&m.engine, m.silencio ? 0 : m.volumenes[0]);
}
void Audio::volumen(float v) {
    auto &m = *motor_;
    auto &a = m.volumenes[m.canal];
    a = std::clamp(a + v, 0.f, 1.f);
    if (m.canal)
        ma_sound_group_set_volume(&m.grupos[m.canal - 1], a);
    else
        ma_engine_set_volume(&m.engine, m.silencio ? 0 : a);
}
void Audio::siguienteCanal() {
    motor_->canal = (motor_->canal + 1) % 4;
}
std::string Audio::controlVolumen() const {
    const char *n[] = {"GENERAL", "MUSICA", "AMBIENTE", "EFECTOS"};
    auto &m = *motor_;
    return std::string(n[m.canal]) + " " + std::to_string(int(m.volumenes[m.canal] * 100)) +
           (m.silencio ? " / SILENCIO" : "");
}
float Audio::atenuacionDistancia(float distancia, float alcance) {
    float d = std::max(0.f, distancia) / alcance;
    return 1 / (1 + d * d);
}
Audio::EstadoVoz Audio::estado(Sonido s) {
    auto &m = *motor_;
    ma_uint64 cursor = 0;
    ma_sound_get_cursor_in_pcm_frames(&m.voces[s], &cursor);
    return {m.objetivos[s], size_t(cursor), bool(ma_sound_is_playing(&m.voces[s])), m.inicios[s],
            ma_sound_get_current_fade_volume(&m.voces[s])};
}
void Audio::oyente(glm::vec3 p, glm::vec3 f) {
    auto &e = motor_->engine;
    ma_engine_listener_set_position(&e, 0, p.x, p.y, p.z);
    ma_engine_listener_set_direction(&e, 0, f.x, f.y, f.z);
    ma_engine_listener_set_world_up(&e, 0, 0, 1, 0);
}
void Audio::fuente(Sonido s, glm::vec3 p, bool espacial) {
    auto &v = motor_->voces[s];
    ma_sound_set_spatialization_enabled(&v, espacial);
    ma_sound_set_position(&v, p.x, p.y, p.z);
    ma_sound_set_attenuation_model(&v, ma_attenuation_model_inverse);
    ma_sound_set_min_distance(&v, 8);
    ma_sound_set_max_distance(&v, 250);
    ma_sound_set_rolloff(&v, 1);
}
bool Audio::disponible() const {
    return motor_->listo && !motor_->offline;
}
unsigned Audio::bloques() const {
    return motor_->bloques;
}
std::vector<float> Audio::leerPCM(unsigned n) {
    if (!motor_->offline)
        throw std::logic_error("Lectura PCM requiere motor sin dispositivo");
    std::vector<float> pcm(size_t(n) * 2);
    if (ma_engine_read_pcm_frames(&motor_->engine, pcm.data(), n, nullptr) != MA_SUCCESS)
        throw std::runtime_error("Lectura PCM fallo");
    return pcm;
}
