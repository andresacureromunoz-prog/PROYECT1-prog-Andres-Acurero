#ifndef OPERACIONES_HISTORIAL_HPP
#define OPERACIONES_HISTORIAL_HPP

#include "HistorialMedico.hpp"
#include <vector>

namespace ServicioHistorial {
    void registrarConsulta(std::vector<HistorialMedico>& bufferHistorial, int& proximoId);
    const HistorialMedico* buscarConsultaPorID(const std::vector<HistorialMedico>& bufferHistorial, int id);
    bool enlazarConsultas(std::vector<HistorialMedico>& bufferHistorial, int idActual, int idSiguiente);
    void listarHistorialPaciente(const std::vector<HistorialMedico>& bufferHistorial, int pacienteId);
}

#endif
