#include "Validaciones.hpp"
#include <cstring>
#include <regex>
#include <sstream>

bool Validaciones::validarCedula(const char* cedula) {
    if (!cedula) return false;
    int len = std::strlen(cedula);
    if (len < 7 || len > 10) return false;
    for (int i = 0; i < len; i++) {
        if (!isdigit(cedula[i])) return false;
    }
    return true;
}

bool Validaciones::validarEmail(const char* email) {
    if (!email) return false;
    std::regex patron("^[\\w.-]+@[\\w.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, patron);
}

bool Validaciones::validarTelefono(const char* telefono) {
    if (!telefono) return false;
    std::regex patron("^\\+?[0-9]{7,15}$");
    return std::regex_match(telefono, patron);
}

bool Validaciones::validarFecha(const char* fecha) {
    if (!fecha) return false;
    int d, m, a;
    char sep;
    std::istringstream iss(fecha);
    if (!(iss >> d >> sep >> m >> sep >> a)) return false;
    if (d < 1 || d > 31) return false;
    if (m < 1 || m > 12) return false;
    if (a < 1900 || a > 2100) return false;
    return true;
}

bool Validaciones::validarRango(int valor, int min, int max) {
    return valor >= min && valor <= max;
}
