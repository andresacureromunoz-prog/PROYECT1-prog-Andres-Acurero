#ifndef FORMATOS_HPP
#define FORMATOS_HPP

#include <string>
#include <ctime>

class Formatos {
public:
    // Métodos estáticos de formateo
    static std::string formatearFecha(time_t tiempo);
    static std::string formatearTelefono(const char* telefono);
    static void convertirAMayusculas(char* texto);
    static void convertirAMinusculas(char* texto);
    static void limpiarBuffer();
    static void pausar();
};

#endif
