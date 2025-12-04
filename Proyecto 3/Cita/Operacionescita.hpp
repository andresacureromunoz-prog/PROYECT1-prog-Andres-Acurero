#ifndef OPERACIONES_CITAS_HPP
#define OPERACIONES_CITAS_HPP

#include "Cita.hpp"
#include <vector>

namespace ServicioCitas {
    void registrarCita(std::vector<Cita>& bufferCitas, int& proximoId);
    const Cita* buscarCitaPorID(const std::vector<Cita>& bufferCitas, int id);
    bool modificarCita(std::vector<Cita>& bufferCitas, int id);
    bool cancelarCita(std::vector<Cita>& bufferCitas, int id);
    void listarTodasCitas(const std::vector<Cita>& bufferCitas);
}

#endif
