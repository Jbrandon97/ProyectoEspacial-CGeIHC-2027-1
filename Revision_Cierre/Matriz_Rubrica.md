# Matriz resumida de requisitos

| Requisito | Estado en este cierre | Evidencia |
|---|---|---|
| Iluminación solar y ciclo día/noche configurable | Cumplido y probado automáticamente | `Pruebas_Cierre.txt`, `Capturas/planeta2_revision.png` |
| Luces puntuales y dos spotlights en P2 | Cumplido y probado por diferencia de píxeles | `INTEGRAL_P2_OK`, captura P2 |
| Freeport como estación espacial principal | Cumplido y probado en geometría/colisiones | `ESPACIO_B1_GEOMETRIA_OK`, captura Espacio |
| Patrullas NPC y Railjack | Cumplido y probado con muestras temporales | `PLANETA2_OK`, `INTEGRAL_P2_OK` |
| Holocrón, Engineer, carrera y lanzamiento P1 | Cumplido en la rama y cubierto por regresión | `CIERRE_P1_OK`, captura P1 |
| Audio continuo, ambiente y mezcla por región | Cumplido técnicamente; escucha humana pendiente | `MINIAUDIO_*`, `Musica_licencias.md` |
| Cuatro recorridos académicos completos | Parcial: P1/Freeport preparados; recorrido final de equipo pendiente | `Revision_Integral.md` |
| Integración final con ramas de compañeros | Pendiente de merge autorizado | rama `mejoras-teoria-final`, sin push |
| Validación del profesor y autoría de recursos de terceros | Pendiente externa; procedencias conservadas | licencias y atribuciones en `Recursos` |
| HUD sin rótulos técnicos | Cumplido y revisado en capturas de las tres regiones | `Capturas/HUD_Limpio_3_Regiones.jpg` |
| Patrullas P1 y pausas G/K/H | Cumplido y probado por muestreo/captura | `Capturas/P1_Patrullas_Reacciones.jpg`, log `P1_PATRULLA_OK` |
| Apoyo de las seis bases P2 de día/noche | Sin huecos; base índice 4 requiere vistazo manual por pendiente local | `Capturas/P2_Bases_Dia_Noche.jpg`, mediciones en `Pruebas_Cierre.txt` |
| Tres objetos distintos con animación básica por teclado | Cumplido y probado: puerta E, Grunt 1 con G y Grunt 2 con K | `B2_HANGAR_OK`, `P1_PATRULLA_OK`, capturas G/K |
| Dos objetos por keyframes | Cumplido: SaturnV y Holocron, verificados como objetos distintos | `C_ANIMACIONES_OK`, `C_COHETE_OK`, prueba integral Holocron |
