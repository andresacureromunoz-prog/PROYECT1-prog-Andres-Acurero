#ifndef OPERACIONES_DOCTORES_HPP
#define OPERACIONES_DOCTORES_HPP

#include "Doctor.hpp"
#include <vector>

namespace ServicioDoctores {
    void registrarDoctor(std::vector<Doctor>& bufferDoctores, int& proximoId);
    const Doctor* buscarDoctorPorID(const std::vector<Doctor>& bufferDoctores, int id);
    void listarTodosDoctores(const std::vector<Doctor>& bufferDoctores);
    void listarDoctoresPorEspecialidad(const std::vector<Doctor>& bufferDoctores, const char* especialidad);
    bool modificarDoctor(std::vector<Doctor>& bufferDoctores, int id);
    bool eliminarDoctor(std::vector<Doctor>& bufferDoctores, int id);
}

#endif
