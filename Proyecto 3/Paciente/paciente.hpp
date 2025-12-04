#ifndef PACIENTE_HPP
#define PACIENTE_HPP

#include <cstring>
#include <ctime>
#include <iostream>

class Paciente {
private:
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[15];
    int edad;
    int citas[20];
    int cantidadCitas;
    time_t fechaCreacion;
    time_t fechaModificacion;
    bool activo;

public:
    // Constructores
    Paciente();
    Paciente(int id, const char* nombre, const char* apellido, const char* cedula, int edad);
    Paciente(const Paciente& otro);
    ~Paciente();

    // Getters (const)
    int getId() const;
    const char* getNombre() const;
    const char* getApellido() const;
    const char* getCedula() const;
    int getEdad() const;
    int getCantidadCitas() const;
    bool estaActivo() const;
    time_t getFechaCreacion() const;
    time_t getFechaModificacion() const;

    // Setters con validación
    void setNombre(const char* nuevoNombre);
    void setApellido(const char* nuevoApellido);
    void setCedula(const char* nuevaCedula);
    void setEdad(int nuevaEdad);
    void setActivo(bool valor);

    // Validaciones específicas
    bool validarDatos() const;
    bool esMayorDeEdad() const;
    bool cedulaEsValida() const;

    // Gestión de citas
    bool agregarCitaID(int citaID);
    bool eliminarCitaID(int citaID);
    bool tieneCitas() const;

    // Presentación
    void mostrarInformacionBasica() const;
    void mostrarInformacionCompleta() const;

    // Tamaño binario (para persistencia)
    static size_t obtenerTamano();
};

#endif
