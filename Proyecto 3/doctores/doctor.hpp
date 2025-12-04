#ifndef DOCTOR_HPP
#define DOCTOR_HPP

#include <cstring>
#include <ctime>
#include <iostream>

class Doctor {
private:
    int id;
    char nombre[50];
    char apellido[50];
    char cedulaProfesional[20];
    char especialidad[50];
    bool disponible;
    double costoConsulta;
    time_t fechaCreacion;
    time_t fechaModificacion;
    bool activo;

public:
    // Constructores
    Doctor();
    Doctor(int id, const char* nombre, const char* apellido,
           const char* cedulaProfesional, const char* especialidad, double costoConsulta);
    Doctor(const Doctor& otro);
    ~Doctor();

    // Getters
    int getId() const;
    const char* getNombre() const;
    const char* getApellido() const;
    const char* getCedulaProfesional() const;
    const char* getEspecialidad() const;
    bool estaDisponible() const;
    double getCostoConsulta() const;
    bool estaActivo() const;

    // Setters
    void setNombre(const char* nuevoNombre);
    void setApellido(const char* nuevoApellido);
    void setCedulaProfesional(const char* nuevaCedula);
    void setEspecialidad(const char* nuevaEspecialidad);
    void setDisponible(bool valor);
    void setCostoConsulta(double nuevoCosto);
    void setActivo(bool valor);

    // Validaciones específicas
    bool validarDatos() const;
    bool especialidadEsValida() const;
    bool cedulaProfesionalValida() const;
    bool costoConsultaValido() const;

    // Presentación
    void mostrarInformacionBasica() const;
    void mostrarInformacionCompleta() const;

    // Tamaño binario
    static size_t obtenerTamano();
};

#endif
