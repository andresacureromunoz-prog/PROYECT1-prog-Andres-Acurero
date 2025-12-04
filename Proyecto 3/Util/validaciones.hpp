#ifndef VALIDACIONES_HPP
#define VALIDACIONES_HPP

#include <string>

class Validaciones {
public:
    // Métodos estáticos de validación
    static bool validarCedula(const char* cedula);
    static bool validarEmail(const char* email);
    static bool validarTelefono(const char* telefono);
    static bool validarFecha(const char* fecha); // formato DD/MM/AAAA
    static bool validarRango(int valor, int min, int max);
};

#endif
