#include "operacionesPacientes.hpp"
#include <iostream>
#include <limits>

namespace ServicioPacientes {

static void limpiarEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void registrarPaciente(std::vector<Paciente>& bufferPacientes, int& proximoId) {
    char nombre[50], apellido[50], cedula[15];
    int edad;

    std::cout << "=== Registrar nuevo paciente ===" << std::endl;
    std::cout << "Nombre: "; std::cin.getline(nombre, sizeof(nombre));
    std::cout << "Apellido: "; std::cin.getline(apellido, sizeof(apellido));
    std::cout << "Cedula: "; std::cin.getline(cedula, sizeof(cedula));
    std::cout << "Edad: "; std::cin >> edad; limpiarEntrada();

    Paciente p(proximoId, nombre, apellido, cedula, edad);

    if (!p.validarDatos()) {
        std::cerr << "Datos invalidos. Registro cancelado." << std::endl;
        return;
    }

    bufferPacientes.push_back(p);
    std::cout << "Paciente registrado con ID: " << proximoId << std::endl;
    proximoId++; // En integración real: hospital.generarNuevoIDPaciente();
    // En integración real: GestorArchivos::guardarPaciente(p);
}

const Paciente* buscarPacientePorID(const std::vector<Paciente>& bufferPacientes, int id) {
    for (const auto& p : bufferPacientes) {
        if (p.getId() == id && p.estaActivo()) return &p;
    }
    return nullptr;
}

const Paciente* buscarPacientePorCedula(const std::vector<Paciente>& bufferPacientes, const char* cedula) {
    for (const auto& p : bufferPacientes) {
        if (std::strcmp(p.getCedula(), cedula) == 0 && p.estaActivo()) return &p;
    }
    return nullptr;
}

bool modificarPaciente(std::vector<Paciente>& bufferPacientes, int id) {
    for (auto& p : bufferPacientes) {
        if (p.getId() == id && p.estaActivo()) {
            char nombre[50], apellido[50], cedula[15];
            int edad;

            std::cout << "=== Modificar paciente ID " << id << " ===" << std::endl;
            std::cout << "Nuevo nombre (enter para mantener): ";
            std::cin.getline(nombre, sizeof(nombre));
            if (std::strlen(nombre) > 0) p.setNombre(nombre);

            std::cout << "Nuevo apellido (enter para mantener): ";
            std::cin.getline(apellido, sizeof(apellido));
            if (std::strlen(apellido) > 0) p.setApellido(apellido);

            std::cout << "Nueva cedula (enter para mantener): ";
            std::cin.getline(cedula, sizeof(cedula));
            if (std::strlen(cedula) > 0) p.setCedula(cedula);

            std::cout << "Nueva edad (negativa para mantener): ";
            std::cin >> edad; limpiarEntrada();
            if (edad >= 0) p.setEdad(edad);

            if (!p.validarDatos()) {
                std::cerr << "Datos invalidos tras modificacion. Cambios no aplicados completamente." << std::endl;
                return false;
            }
            std::cout << "Paciente modificado." << std::endl;
            return true;
        }
    }
    return false;
}

bool eliminarPaciente(std::vector<Paciente>& bufferPacientes, int id) {
    for (auto& p : bufferPacientes) {
        if (p.getId() == id && p.estaActivo()) {
            p.setActivo(false); // Eliminación lógica
            std::cout << "Paciente ID " << id << " marcado como inactivo." << std::endl;
            return true;
        }
    }
    return false;
}

void listarTodosPacientes(const std::vector<Paciente>& bufferPacientes) {
    std::cout << "=== Lista de pacientes activos ===" << std::endl;
    for (const auto& p : bufferPacientes) {
        if (p.estaActivo()) p.mostrarInformacionBasica();
    }
}

} // namespace ServicioPacientes
