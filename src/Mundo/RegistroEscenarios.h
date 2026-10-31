#pragma once
class GestorEscenarios;
// Unico punto de construccion de las tres implementaciones; no carga recursos GPU.
void registrarEscenarios(GestorEscenarios &gestor);
