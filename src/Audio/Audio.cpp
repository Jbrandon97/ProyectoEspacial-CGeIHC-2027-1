#include "Audio.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
Audio::Audio(const std::filesystem::path &r) {
    const char *nombres[] = {"Orbita_ambiental", "Viento_instalaciones", "Puerta_hangar",
                             "Propulsion",       "Motor_nave",           "Inicio_carrera",
                             "Final_carrera",    "Energia_Covenant", "Horizonte_espacial"};
    for (size_t i = 0; i < voces_.size(); ++i) {
        std::ifstream f(r / "Audio" / (std::string(nombres[i]) + ".wav"), std::ios::binary);
        std::vector<char> b((std::istreambuf_iterator<char>(f)), {});
        if (b.size() < 44 || std::memcmp(b.data(), "RIFF", 4))
            throw std::runtime_error("WAV ausente/invalido");
        size_t at = 12;
        while (at + 8 <= b.size()) {
            unsigned n;
            std::memcpy(&n, b.data() + at + 4, 4);
            if (at + 8 + n > b.size())
                throw std::runtime_error("WAV truncado");
            if (!std::memcmp(b.data() + at, "data", 4)) {
                voces_[i].datos.resize(n / 2);
                std::memcpy(voces_[i].datos.data(), b.data() + at + 8, n);
                break;
            }
            at += 8 + n + (n & 1);
        }
        if (voces_[i].datos.empty())
            throw std::runtime_error("WAV sin PCM");
    }
    std::ifstream cfg(r / "Configuracion_C.txt");
    std::string linea;
    while (std::getline(cfg, linea))
        if (linea.rfind("volumen=", 0) == 0)
            try {
                float v = std::stof(linea.substr(8));
                if (std::isfinite(v))
                    volumen_ = std::clamp(v, 0.f, 1.f);
            } catch (...) {
            }
    WAVEFORMATEX fmt{};
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = 2;
    fmt.nSamplesPerSec = 44100;
    fmt.wBitsPerSample = 16;
    fmt.nBlockAlign = 4;
    fmt.nAvgBytesPerSec = 176400;
    if (waveOutOpen(&salida_, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        salida_ = nullptr;
        std::cout << "AUDIO_SIN_DISPOSITIVO WAV cargados, no reproduccion verificada\n";
        return;
    }
    std::cout << "AUDIO_DISPOSITIVO_OK PCM stereo 44100\n";
    hilo_ = std::thread(&Audio::ejecutar, this);
}
Audio::~Audio() {
    salir_ = true;
    if (hilo_.joinable())
        hilo_.join();
    if (salida_)
        waveOutClose(salida_);
    std::cout << "AUDIO_CIERRE bloques=" << bloques_ << "\n";
}
void Audio::bucle(Sonido s, float v) {
    std::lock_guard<std::mutex> l(mutex_);
    auto &x = voces_[s];
    x.bucle = true;
    x.objetivo = std::clamp(v, 0.f, 1.f);
    if (v > 0 && !x.activa) {
        x.activa = true;
        x.cursor = 0;
        ++x.inicios;
    }
}
void Audio::disparar(Sonido s, float v) {
    std::lock_guard<std::mutex> l(mutex_);
    auto &x = voces_[s];
    x.bucle = false;
    x.activa = true;
    x.cursor = 0;
    ++x.inicios;
    x.objetivo = v;
}
void Audio::silenciar() {
    std::lock_guard<std::mutex> l(mutex_);
    silencio_ = !silencio_;
}
void Audio::volumen(float v) {
    std::lock_guard<std::mutex> l(mutex_);
    volumen_ = std::clamp(volumen_ + v, 0.f, 1.f);
}
void Audio::ejecutar() {
    std::array<std::array<short, 2048>, 4> pcm{};
    std::array<WAVEHDR, 4> cab{};
    for (int i = 0; i < 4; ++i) {
        cab[i].lpData = reinterpret_cast<char *>(pcm[i].data());
        cab[i].dwBufferLength = 4096;
        waveOutPrepareHeader(salida_, &cab[i], sizeof(WAVEHDR));
        cab[i].dwFlags |= WHDR_DONE;
    }
    while (!salir_) {
        for (int i = 0; i < 4; ++i) {
            if (!(cab[i].dwFlags & WHDR_DONE))
                continue;
            {
                std::lock_guard<std::mutex> l(mutex_);
                for (int k = 0; k < 2048; ++k) {
                    float muestra = 0;
                    for (auto &v : voces_) {
                        bool musica = &v == &voces_[Musica] || &v == &voces_[MusicaEspacio];
                        v.ganancia += (v.objetivo - v.ganancia) * (musica ? .000045f : .0005f);
                        if (v.objetivo == 0 && v.ganancia < .0001f) {
                            v.activa = false;
                            v.cursor = 0;
                        }
                        if (!v.activa)
                            continue;
                        if (v.cursor >= v.datos.size()) {
                            if (v.bucle)
                                v.cursor = 0;
                            else {
                                v.activa = false;
                                v.objetivo = 0;
                                continue;
                            }
                        }
                        muestra += v.datos[v.cursor++] * v.ganancia;
                    }
                    muestra *= silencio_ ? 0 : volumen_;
                    pcm[i][k] = short(std::clamp(muestra, -32000.f, 32000.f));
                }
            }
            if (waveOutWrite(salida_, &cab[i], sizeof(WAVEHDR)) == MMSYSERR_NOERROR)
                ++bloques_;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
    waveOutReset(salida_);
    for (auto &c : cab)
        waveOutUnprepareHeader(salida_, &c, sizeof(WAVEHDR));
}

void Audio::detenerEfectos() {
    std::lock_guard<std::mutex> l(mutex_);
    for (int i = 2; i < 8; ++i)
        voces_[i].objetivo = 0;
}

float Audio::atenuacionDistancia(float distancia) {
    float d = std::max(0.f, distancia) / 45.f;
    return 1.f / (1.f + d * d);
}
Audio::EstadoVoz Audio::estado(Sonido s) {
    std::lock_guard<std::mutex> l(mutex_);
    const auto &v = voces_[s];
    return {v.objetivo, v.cursor, v.activa, v.inicios, v.ganancia};
}
