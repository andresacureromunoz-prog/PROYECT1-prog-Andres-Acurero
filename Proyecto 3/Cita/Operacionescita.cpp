#include "Operacionescita.hpp"
#include <iostream>
#include <limits>

namespace ServicioCitas {

static void limpiarEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void registrarCita(std::vector<Cita>& bufferCitas, int& proximoId) {
    int pacienteId, doctorId;
    std::string fecha, hora;

    std::cout << "=== Registrar nueva cita ===" << std::endl;
    std::cout << "ID Paciente: "; std::cin >> pacienteId; limpiarEntrada();
    std::cout << "ID Doctor: "; std::cin >> doctorId; limpiarEntrada();
    std::cout << "Fecha (DD/MM/AAAA): "; std::getline(std::cin, fecha);
    std::cout << "Hora (HH:MM): "; std::getline(std::cin, hora);

    Cita c(proximoId, pacienteId, doctorId, fecha, hora);

    if (!c.validarDatos()) {
        std::cerr << "Datos inválidos. La cita no fue registrada." << std::endl;
        return;
    }

    bufferCitas.push_back(c);
    std::cout << "Cita registrada con ID: " << proximoId << std::endl;
    proximoId++;
}

Cita* buscarCitaPorID(std::vector<Cita>& bufferCitas, int id) {
    // El bucle también necesita un 'auto&' en lugar de 'const auto&'
    for (auto& c : bufferCitas) { 
        if (c.getId() == id) return &c;
    }
    return nullptr;
}
void listarTodasCitas(const std::vector<Cita>& bufferCitas) {
    std::cout << "=== Listado de todas las citas ===" << std::endl;
    for (const auto& c : bufferCitas) {
        c.mostrarInformacionBasica();
    }
}

} // namespace ServicioCitas
