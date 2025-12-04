#include "operacionesHistorial.hpp"
#include <iostream>
#include <limits>

namespace ServicioHistorial {

static void limpiarEntrada() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void registrarConsulta(std::vector<HistorialMedico>& bufferHistorial, int& proximoId) {
    int pacienteId, doctorId;
    std::string diagnostico, tratamiento, fecha;

    std::cout << "=== Registrar nueva consulta médica ===" << std::endl;
    std::cout << "ID Paciente: "; std::cin >> pacienteId; limpiarEntrada();
    std::cout << "ID Doctor: "; std::cin >> doctorId; limpiarEntrada();
    std::cout << "Diagnóstico: "; std::getline(std::cin, diagnostico);
    std::cout << "Tratamiento: "; std::getline(std::cin, tratamiento);
    std::cout << "Fecha (DD/MM/AAAA): "; std::getline(std::cin, fecha);

    HistorialMedico h(proximoId, pacienteId, doctorId, diagnostico, tratamiento, fecha);

    if (!h.validarDatos()) {
        std::cerr << "Datos inválidos. Consulta no registrada." << std::endl;
        return;
    }

    bufferHistorial.push_back(h);
    std::cout << "Consulta registrada con ID: " << proximoId << std::endl;
    proximoId++;
}

    // Declaración completa
const HistorialMedico* buscarConsultaPorID(const std::vector<HistorialMedico>& bufferHistorial, int idBuscado) {
    
    // Iteramos por cada elemento del historial
    for (const auto& consulta : bufferHistorial) {
        // Asumimos que tu clase HistorialMedico tiene un método o atributo getId() o id
        if (consulta.getId() == idBuscado) {
            return &consulta; // Retorna la dirección de memoria de la consulta encontrada
        }
    }
    
    return nullptr; // Retorna nulo si no se encuentra el ID
}