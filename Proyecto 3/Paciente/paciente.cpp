#include "Paciente.hpp"
#include <iostream>
#include <cstring>

Paciente::Paciente() {
    id = 0;
    std::strcpy(nombre, "");
    std::strcpy(apellido, "");
    std::strcpy(cedula, "");
    edad = 0;
    cantidadCitas = 0;
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Paciente::Paciente(int id, const char* nombre, const char* apellido, const char* cedula, int edad) {
    this->id = id;
    cantidadCitas = 0;
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
    setNombre(nombre);
    setApellido(apellido);
    setCedula(cedula);
    setEdad(edad);
}

Paciente::Paciente(const Paciente& otro) {
    id = otro.id;
    std::strcpy(nombre, otro.nombre);
    std::strcpy(apellido, otro.apellido);
    std::strcpy(cedula, otro.cedula);
    edad = otro.edad;
    cantidadCitas = otro.cantidadCitas;
    for (int i = 0; i < cantidadCitas; ++i) {
        citas[i] = otro.citas[i];
    }
    activo = otro.activo;
    fechaCreacion = otro.fechaCreacion;
    fechaModificacion = otro.fechaModificacion;
}

Paciente::~Paciente() {}

int Paciente::getId() const { return id; }
const char* Paciente::getNombre() const { return nombre; }
const char* Paciente::getApellido() const { return apellido; }
const char* Paciente::getCedula() const { return cedula; }
int Paciente::getEdad() const { return edad; }
int Paciente::getCantidadCitas() const { return cantidadCitas; }
bool Paciente::estaActivo() const { return activo; }
time_t Paciente::getFechaCreacion() const { return fechaCreacion; }
time_t Paciente::getFechaModificacion() const { return fechaModificacion; }

void Paciente::setNombre(const char* nuevoNombre) {
    if (nuevoNombre && std::strlen(nuevoNombre) > 0 && std::strlen(nuevoNombre) < sizeof(nombre)) {
        std::strcpy(nombre, nuevoNombre);
        fechaModificacion = std::time(nullptr);
    }
}

void Paciente::setApellido(const char* nuevoApellido) {
    if (nuevoApellido && std::strlen(nuevoApellido) > 0 && std::strlen(nuevoApellido) < sizeof(apellido)) {
        std::strcpy(apellido, nuevoApellido);
        fechaModificacion = std::time(nullptr);
    }
}

void Paciente::setCedula(const char* nuevaCedula) {
    if (nuevaCedula && std::strlen(nuevaCedula) >= 7 && std::strlen(nuevaCedula) < sizeof(cedula)) {
        std::strcpy(cedula, nuevaCedula);
        fechaModificacion = std::time(nullptr);
    }
}

void Paciente::setEdad(int nuevaEdad) {
    if (nuevaEdad >= 0 && nuevaEdad <= 120) {
        edad = nuevaEdad;
        fechaModificacion = std::time(nullptr);
    }
}

void Paciente::setActivo(bool valor) {
    activo = valor;
    fechaModificacion = std::time(nullptr);
}

bool Paciente::validarDatos() const {
    if (std::strlen(nombre) == 0) return false;
    if (std::strlen(apellido) == 0) return false;
    if (!cedulaEsValida()) return false;
    if (edad <= 0 || edad > 120) return false;
    return true;
}

bool Paciente::esMayorDeEdad() const { return edad >= 18; }

bool Paciente::cedulaEsValida() const {
    size_t len = std::strlen(cedula);
    return len >= 7 && len <= 12; // rango básico; luego llamaremos a Validaciones::validarCedula()
}

bool Paciente::agregarCitaID(int citaID) {
    if (cantidadCitas >= 20 || citaID <= 0) return false;
    citas[cantidadCitas++] = citaID;
    fechaModificacion = std::time(nullptr);
    return true;
}

bool Paciente::eliminarCitaID(int citaID) {
    for (int i = 0; i < cantidadCitas; ++i) {
        if (citas[i] == citaID) {
            for (int j = i; j < cantidadCitas - 1; ++j) {
                citas[j] = citas[j + 1];
            }
            --cantidadCitas;
            fechaModificacion = std::time(nullptr);
            return true;
        }
    }
    return false;
}

bool Paciente::tieneCitas() const { return cantidadCitas > 0; }

void Paciente::mostrarInformacionBasica() const {
    std::cout << "ID: " << id
              << " | Nombre: " << nombre << " " << apellido
              << " | Edad: " << edad
              << " | Activo: " << (activo ? "Si" : "No")
              << std::endl;
}

void Paciente::mostrarInformacionCompleta() const {
    mostrarInformacionBasica();
    std::cout << "Cédula: " << cedula
              << " | Citas: " << cantidadCitas
              << " | Creado: " << std::asctime(std::localtime(&fechaCreacion))
              << " | Modificado: " << std::asctime(std::localtime(&fechaModificacion));
}

size_t Paciente::obtenerTamano() {
    return sizeof(Paciente);
}
