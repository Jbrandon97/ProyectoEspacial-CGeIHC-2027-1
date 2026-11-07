#include "SectorWarframe.h"
// La torre, el NPC Warframe y Railjack comparten la distribucion de SectorRatchet.
void SectorWarframe::cargar(ContextoEscena &) { liberar(); }
void SectorWarframe::actualizar(float) {}
void SectorWarframe::dibujar(ContextoEscena &, const glm::mat4 &) const {}
void SectorWarframe::liberar() noexcept { objetos_.clear(); }
