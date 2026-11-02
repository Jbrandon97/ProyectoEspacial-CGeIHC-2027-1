cmake_minimum_required(VERSION 3.24)
project(ProyectoEspacial VERSION 0.2.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configuraciones del equipo" FORCE)
# Runtime estatico: el Release no requiere redistribuir DLL de MSVC/GLFW.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(Dependencias/glfw EXCLUDE_FROM_ALL)
add_library(glad STATIC Dependencias/glad/src/glad.c)
target_include_directories(glad PUBLIC Dependencias/glad/include)
add_library(glm INTERFACE)
target_include_directories(glm SYSTEM INTERFACE Dependencias/glm)
add_executable(ProyectoEspacial src/main.cpp)
target_sources(ProyectoEspacial PRIVATE
 src/Audio/Audio.cpp src/Nucleo/Aplicacion.cpp src/Nucleo/Ventana.cpp src/Nucleo/Entradas.cpp
 src/Graficos/ModeloGltf.cpp src/Graficos/Geometria.cpp src/Graficos/Interfaz.cpp src/Graficos/Shader.cpp src/Graficos/Malla.cpp src/Graficos/Renderizador.cpp
 src/Camaras/GestorCamaras.cpp src/Mundo/MundoEspacial.cpp
 src/Mundo/GestorEscenarios.cpp src/Mundo/RegistroEscenarios.cpp
 src/Pruebas/PruebasE.cpp src/Pruebas/PruebasD.cpp src/Pruebas/PruebasC.cpp src/Pruebas/PruebasB2.cpp src/Pruebas/PruebasB1.cpp src/Pruebas/PruebasCarrera.cpp src/Personajes/Engineer.cpp src/Personajes/Avatar.cpp src/Pruebas/Verificacion.cpp)
add_subdirectory(src/Regiones)
target_include_directories(ProyectoEspacial PRIVATE src Dependencias/importacion)
target_compile_definitions(ProyectoEspacial PRIVATE GLFW_INCLUDE_NONE NOMINMAX)
target_link_libraries(ProyectoEspacial PRIVATE glad glfw glm winmm)
if(MSVC)
 target_compile_options(ProyectoEspacial PRIVATE /W4 /utf-8 /permissive-)
endif()
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ProyectoEspacial)
add_custom_command(TARGET ProyectoEspacial POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Recursos" "$<TARGET_FILE_DIR:ProyectoEspacial>/Recursos")
enable_testing()
add_test(NAME IntegracionPlanetaCarrera COMMAND ProyectoEspacial --pruebas)
set_tests_properties(IntegracionPlanetaCarrera PROPERTIES TIMEOUT 240)
install(TARGETS ProyectoEspacial RUNTIME DESTINATION .)
install(DIRECTORY Recursos DESTINATION .)
