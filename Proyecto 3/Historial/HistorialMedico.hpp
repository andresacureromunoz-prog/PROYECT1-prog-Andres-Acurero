#ifndef HISTORIAL_MEDICO_HPP
#define HISTORIAL_MEDICO_HPP

#include <string>
#include <ctime>
#include <iostream>

class HistorialMedico {
private:
    int id;
    int pacienteId;
    int doctorId;
    std::string diagnostico;
    std::string tratamiento;
    std::string fecha; // formato DD/MM/AAAA
    int siguienteConsultaId; // ID de la siguiente consulta en la lista enlazada
    time_t fechaCreacion;
    time_t fechaModificacion;
    bool activo;

public:
    // Constructores
    HistorialMedico();
    HistorialMedico(int id, int pacienteId, int doctorId,
                    const std::string& diagnostico,
                    const std::string& tratamiento,
                    const std::string& fecha);
    HistorialMedico(const HistorialMedico& otro);
    ~HistorialMedico();

    // Getters
    int getId() const;
    int getPacienteId() const;
    int getDoctorId() const;
    std::string getDiagnostico() const;
    std::string getTratamiento() const;
    std::string getFecha() const;
    int getSiguienteConsultaId() const;
    bool estaActivo() const;

    // Setters
    void setDiagnostico(const std::string& nuevoDiagnostico);
    void setTratamiento(const std::string& nuevoTratamiento);
    void setFecha(const std::string& nuevaFecha);
    void setSiguienteConsultaId(int id);
    void setActivo(bool valor);

    // Validaciones
    bool validarDatos() const;
    bool fechaValida() const;

    // Presentación
    void mostrarInformacionBasica() const;
    void mostrarInformacionCompleta() const;

    // Tamaño binario
    static size_t obtenerTamano();
};

#endif
