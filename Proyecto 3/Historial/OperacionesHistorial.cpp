#include "OperacionesHistorial.hpp"
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

const HistorialMedico* buscarConsultaPorID(const std::vector<HistorialMedico>& bufferHistorial, int idBuscado) {
    for (const auto& consulta : bufferHistorial) {
        if (consulta.getId() == idBuscado) {
            return &consulta;
        }
    }
    return nullptr;
}

bool enlazarConsultas(std::vector<HistorialMedico>& bufferHistorial, int idActual, int idSiguiente) {
    HistorialMedico* actual = nullptr;
    HistorialMedico* siguiente = nullptr;

    for (auto& c : bufferHistorial) {
        if (c.getId() == idActual) actual = &c;
        if (c.getId() == idSiguiente) siguiente = &c;
    }

    if (!actual || !siguiente) return false;
    actual->setSiguienteConsultaId(idSiguiente);
    return true;
}

void listarHistorialPaciente(const std::vector<HistorialMedico>& bufferHistorial, int pacienteId) {
    std::cout << "=== Historial del paciente ID: " << pacienteId << " ===" << std::endl;
    for (const auto& c : bufferHistorial) {
        if (c.getPacienteId() == pacienteId) {
            c.mostrarInformacionBasica();
        }
    }
}

} // namespace ServicioHistorial