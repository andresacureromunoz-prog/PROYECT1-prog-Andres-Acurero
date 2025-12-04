#ifndef OPERACIONES_PACIENTES_HPP
#define OPERACIONES_PACIENTES_HPP

#include "Paciente.hpp"
#include <vector>

// Nota: En la integración real, estas funciones usarán GestorArchivos.
// Por ahora, trabajamos con un contenedor en memoria para validar el flujo.

namespace ServicioPacientes {
    // Registro
    void registrarPaciente(std::vector<Paciente>& bufferPacientes, int& proximoId);

    // Búsqueda
    const Paciente* buscarPacientePorID(const std::vector<Paciente>& bufferPacientes, int id);
    const Paciente* buscarPacientePorCedula(const std::vector<Paciente>& bufferPacientes, const char* cedula);

    // Modificación
    bool modificarPaciente(std::vector<Paciente>& bufferPacientes, int id);

    // Eliminación lógica
    bool eliminarPaciente(std::vector<Paciente>& bufferPacientes, int id);

    // Listado
    void listarTodosPacientes(const std::vector<Paciente>& bufferPacientes);
}

#endif
