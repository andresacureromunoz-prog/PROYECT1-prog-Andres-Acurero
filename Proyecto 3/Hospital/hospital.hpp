#ifndef HOSPITAL_HPP
#define HOSPITAL_HPP

#include <string>
#include <ctime>
#include <iostream>

class Hospital {
private:
    std::string nombre;
    std::string direccion;
    std::string telefono;

    // Contadores globales de IDs
    int proximoIdPaciente;
    int proximoIdDoctor;
    int proximoIdCita;
    int proximoIdHistorial;

    // Estadísticas
    int pacientesRegistrados;
    int doctoresRegistrados;
    int citasRegistradas;
    int historialesRegistrados;

    time_t fechaCreacion;
    time_t fechaModificacion;

public:
    // Constructores
    Hospital();
    Hospital(const std::string& nombre, const std::string& direccion, const std::string& telefono);
    ~Hospital();

    // Getters
    std::string getNombre() const;
    std::string getDireccion() const;
    std::string getTelefono() const;

    int getPacientesRegistrados() const;
    int getDoctoresRegistrados() const;
    int getCitasRegistradas() const;
    int getHistorialesRegistrados() const;

    time_t getFechaCreacion() const;
    time_t getFechaModificacion() const;

    // Setters
    void setNombre(const std::string& nuevoNombre);
    void setDireccion(const std::string& nuevaDireccion);
    void setTelefono(const std::string& nuevoTelefono);

    // Generadores de IDs
    int generarNuevoIdPaciente();
    int generarNuevoIdDoctor();
    int generarNuevoIdCita();
    int generarNuevoIdHistorial();

    // Incrementadores de estadísticas
    void incrementarPacientesRegistrados();
    void incrementarDoctoresRegistrados();
    void incrementarCitasRegistradas();
    void incrementarHistorialesRegistrados();

    // Presentación
    void mostrarInformacionBasica() const;
    void mostrarEstadisticas() const;
};

#endif
