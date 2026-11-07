#include "SectorWarframe.h"
// Las figuras provisionales del sector se retiran.
// La torre, el Warframe y el Railjack se cargan como modelos reales en SectorRatchet.
void SectorWarframe::cargar(ContextoEscena &) { liberar(); }
void SectorWarframe::actualizar(float) {}
void SectorWarframe::dibujar(ContextoEscena &, const glm::mat4 &) const {}
void SectorWarframe::liberar() noexcept { objetos_.clear(); }
