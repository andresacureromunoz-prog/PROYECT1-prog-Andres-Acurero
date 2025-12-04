#ifndef CITA_HPP
#define CITA_HPP

#include <string>
#include <ctime>
#include <iostream>

class Cita {
private:
    int id;
    int pacienteId;
    int doctorId;
    std::string fecha; // formato DD/MM/AAAA
    std::string hora;  // formato HH:MM
    bool atendida;
    bool cancelada;
    time_t fechaCreacion;
    time_t fechaModificacion;

public:
    // Constructores
    Cita();
    Cita(int id, int pacienteId, int doctorId, const std::string& fecha, const std::string& hora);
    Cita(const Cita& otra);
    ~Cita();

    // Getters
    int getId() const;
    int getPacienteId() const;
    int getDoctorId() const;
    std::string getFecha() const;
    std::string getHora() const;
    bool estaAtendida() const;
    bool estaCancelada() const;

    // Setters
    void setFecha(const std::string& nuevaFecha);
    void setHora(const std::string& nuevaHora);
    void setAtendida(bool valor);
    void setCancelada(bool valor);

    // Validaciones
    bool validarDatos() const;
    bool fechaValida() const;
    bool horaValida() const;

    // Gestión de estado
    void marcarComoAtendida();
    void cancelar();

    // Presentación
    void mostrarInformacionBasica() const;
    void mostrarInformacionCompleta() const;

    // Tamaño binario
    static size_t obtenerTamano();
};

#endif
