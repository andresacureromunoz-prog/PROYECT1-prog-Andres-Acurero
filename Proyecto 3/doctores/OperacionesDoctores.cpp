#include "operacionesDoctores.hpp"
#include <iostream>
#include <limits>
#include <cstring>

namespace ServicioDoctores {
// Implementación faltante: buscarDoctorPorID
const Doctor* buscarDoctorPorID(const std::vector<Doctor>& bufferDoctores, int id) {
    for (const auto& d : bufferDoctores) {
        if (d.getId() == id) return &d;
    }
    return nullptr;
}

void listarTodosDoctores(const std::vector<Doctor>& bufferDoctores) {
    std::cout << "=== Listado de Doctores ===" << std::endl;
    for (const auto& d : bufferDoctores) {
        d.mostrarInformacionBasica();
    }
}

static void limpiarEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void registrarDoctor(std::vector<Doctor>& bufferDoctores, int& proximoId) {
    char nombre[50], apellido[50], cedula[20], especialidad[50];
    double costo;

    std::cout << "=== Registrar nuevo doctor ===" << std::endl;
    std::cout << "Nombre: "; std::cin.getline(nombre, sizeof(nombre));
    std::cout << "Apellido: "; std::cin.getline(apellido, sizeof(apellido));
    std::cout << "Cedula Profesional: "; std::cin.getline(cedula, sizeof(cedula));
    std::cout << "Especialidad: "; std::cin.getline(especialidad, sizeof(especialidad));
    std::cout << "Costo Consulta: "; std::cin >> costo; limpiarEntrada();

    Doctor d(proximoId, nombre, apellido, cedula, especialidad, costo);

   if (!d.validarDatos()) {
        std::cout << " Error: Datos ingresados no son válidos." << std::endl;
        return; // Salimos de la función sin guardar nada
    }

    // CORRECCIÓN 2: Guardar el objeto en el vector
    bufferDoctores.push_back(d);

    // CORRECCIÓN 3: Incrementar el ID para el siguiente registro
    proximoId++;

    std::cout << " Doctor registrado exitosamente con ID: " << (proximoId - 1) << std::endl;
}

} // namespace ServicioDoctores