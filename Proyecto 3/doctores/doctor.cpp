#include "Doctor.hpp"
#include <iostream>
#include <cstring>

Doctor::Doctor() {
    id = 0;
    std::strcpy(nombre, "");
    std::strcpy(apellido, "");
    std::strcpy(cedulaProfesional, "");
    std::strcpy(especialidad, "");
    disponible = true;
    costoConsulta = 0.0;
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Doctor::Doctor(int id, const char* nombre, const char* apellido,
               const char* cedulaProfesional, const char* especialidad, double costoConsulta) {
    this->id = id;
    setNombre(nombre);
    setApellido(apellido);
    setCedulaProfesional(cedulaProfesional);
    setEspecialidad(especialidad);
    setCostoConsulta(costoConsulta);
    disponible = true;
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Doctor::Doctor(const Doctor& otro) {
    id = otro.id;
    std::strcpy(nombre, otro.nombre);
    std::strcpy(apellido, otro.apellido);
    std::strcpy(cedulaProfesional, otro.cedulaProfesional);
    std::strcpy(especialidad, otro.especialidad);
    disponible = otro.disponible;
    costoConsulta = otro.costoConsulta;
    activo = otro.activo;
    fechaCreacion = otro.fechaCreacion;
    fechaModificacion = otro.fechaModificacion;
}

Doctor::~Doctor() {}

int Doctor::getId() const { return id; }
const char* Doctor::getNombre() const { return nombre; }
const char* Doctor::getApellido() const { return apellido; }
const char* Doctor::getCedulaProfesional() const { return cedulaProfesional; }
const char* Doctor::getEspecialidad() const { return especialidad; }
bool Doctor::estaDisponible() const { return disponible; }
double Doctor::getCostoConsulta() const { return costoConsulta; }
bool Doctor::estaActivo() const { return activo; }

void Doctor::setNombre(const char* nuevoNombre) {
    if (nuevoNombre && std::strlen(nuevoNombre) > 0) {
        std::strcpy(nombre, nuevoNombre);
        fechaModificacion = std::time(nullptr);
    }
}

void Doctor::setApellido(const char* nuevoApellido) {
    if (nuevoApellido && std::strlen(nuevoApellido) > 0) {
        std::strcpy(apellido, nuevoApellido);
        fechaModificacion = std::time(nullptr);
    }
}

void Doctor::setCedulaProfesional(const char* nuevaCedula) {
    if (nuevaCedula && std::strlen(nuevaCedula) > 5) {
        std::strcpy(cedulaProfesional, nuevaCedula);
        fechaModificacion = std::time(nullptr);
    }
}

void Doctor::setEspecialidad(const char* nuevaEspecialidad) {
    if (nuevaEspecialidad && std::strlen(nuevaEspecialidad) > 0) {
        std::strcpy(especialidad, nuevaEspecialidad);
        fechaModificacion = std::time(nullptr);
    }
}

void Doctor::setDisponible(bool valor) {
    disponible = valor;
    fechaModificacion = std::time(nullptr);
}

void Doctor::setCostoConsulta(double nuevoCosto) {
    if (nuevoCosto >= 0 && nuevoCosto <= 1000) {
        costoConsulta = nuevoCosto;
        fechaModificacion = std::time(nullptr);
    }
}

void Doctor::setActivo(bool valor) {
    activo = valor;
    fechaModificacion = std::time(nullptr);
}

bool Doctor::validarDatos() const {
    return std::strlen(nombre) > 0 &&
           std::strlen(apellido) > 0 &&
           cedulaProfesionalValida() &&
           especialidadEsValida() &&
           costoConsultaValido();
}

bool Doctor::especialidadEsValida() const {
    return std::strlen(especialidad) > 0; // luego se puede validar contra lista
}

bool Doctor::cedulaProfesionalValida() const {
    return std::strlen(cedulaProfesional) >= 6;
}

bool Doctor::costoConsultaValido() const {
    return costoConsulta >= 0 && costoConsulta <= 1000;
}

void Doctor::mostrarInformacionBasica() const {
    std::cout << "ID: " << id
              << " | Nombre: " << nombre << " " << apellido
              << " | Especialidad: " << especialidad
              << " | Disponible: " << (disponible ? "Si" : "No")
              << std::endl;
}

void Doctor::mostrarInformacionCompleta() const {
    mostrarInformacionBasica();
    std::cout << "Cedula Profesional: " << cedulaProfesional
              << " | Costo Consulta: $" << costoConsulta
              << " | Activo: " << (activo ? "Si" : "No")
              << std::endl;
}

size_t Doctor::obtenerTamano() {
    return sizeof(Doctor);
}
