#include "Formatos.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <limits>

std::string Formatos::formatearFecha(time_t tiempo) {
    std::tm* tmPtr = std::localtime(&tiempo);
    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", tmPtr);
    return std::string(buffer);
}

std::string Formatos::formatearTelefono(const char* telefono) {
    if (!telefono) return "";
    std::string tlf(telefono);
    if (tlf.size() == 11) {
        return "(" + tlf.substr(0, 4) + ") " + tlf.substr(4, 3) + "-" + tlf.substr(7);
    }
    return tlf;
}

void Formatos::convertirAMayusculas(char* texto) {
    if (!texto) return;
    for (int i = 0; texto[i]; i++) {
        texto[i] = std::toupper(texto[i]);
    }
}

void Formatos::convertirAMinusculas(char* texto) {
    if (!texto) return;
    for (int i = 0; texto[i]; i++) {
        texto[i] = std::tolower(texto[i]);
    }
}

void Formatos::limpiarBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Formatos::pausar() {
    std::cout << "Presione Enter para continuar...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
