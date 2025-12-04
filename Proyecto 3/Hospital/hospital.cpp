#include "Hospital.hpp"

Hospital::Hospital() {
    nombre = "Hospital sin nombre";
    direccion = "Dirección no definida";
    telefono = "000-0000";

    proximoIdPaciente = 1;
    proximoIdDoctor = 1;
    proximoIdCita = 1;
    proximoIdHistorial = 1;

    pacientesRegistrados = 0;
    doctoresRegistrados = 0;
    citasRegistradas = 0;
    historialesRegistrados = 0;

    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Hospital::Hospital(const std::string& nombre, const std::string& direccion, const std::string& telefono) {
    this->nombre = nombre;
    this->direccion = direccion;
    this->telefono = telefono;

    proximoIdPaciente = 1;
    proximoIdDoctor = 1;
    proximoIdCita = 1;
    proximoIdHistorial = 1;

    pacientesRegistrados = 0;
    doctoresRegistrados = 0;
    citasRegistradas = 0;
    historialesRegistrados = 0;

    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Hospital::~Hospital() {}

std::string Hospital::getNombre() const { return nombre; }
std::string Hospital::getDireccion() const { return direccion; }
std::string Hospital::getTelefono() const { return telefono; }

int Hospital::getPacientesRegistrados() const { return pacientesRegistrados; }
int Hospital::getDoctoresRegistrados() const { return doctoresRegistrados; }
int Hospital::getCitasRegistradas() const { return citasRegistradas; }
int Hospital::getHistorialesRegistrados() const { return historialesRegistrados; }

time_t Hospital::getFechaCreacion() const { return fechaCreacion; }
time_t Hospital::getFechaModificacion() const { return fechaModificacion; }

void Hospital::setNombre(const std::string& nuevoNombre) {
    if (!nuevoNombre.empty()) {
        nombre = nuevoNombre;
        fechaModificacion = std::time(nullptr);
    }
}

void Hospital::setDireccion(const std::string& nuevaDireccion) {
    if (!nuevaDireccion.empty()) {
        direccion = nuevaDireccion;
        fechaModificacion = std::time(nullptr);
    }
}

void Hospital::setTelefono(const std::string& nuevoTelefono) {
    if (!nuevoTelefono.empty()) {
        telefono = nuevoTelefono;
        fechaModificacion = std::time(nullptr);
    }
}

// Generadores de IDs
int Hospital::generarNuevoIdPaciente() { return proximoIdPaciente++; }
int Hospital::generarNuevoIdDoctor() { return proximoIdDoctor++; }
int Hospital::generarNuevoIdCita() { return proximoIdCita++; }
int Hospital::generarNuevoIdHistorial() { return proximoIdHistorial++; }

// Incrementadores
void Hospital::incrementarPacientesRegistrados() { pacientesRegistrados++; fechaModificacion = std::time(nullptr); }
void Hospital::incrementarDoctoresRegistrados() { doctoresRegistrados++; fechaModificacion = std::time(nullptr); }
void Hospital::incrementarCitasRegistradas() { citasRegistradas++; fechaModificacion = std::time(nullptr); }
void Hospital::incrementarHistorialesRegistrados() { historialesRegistrados++; fechaModificacion = std::time(nullptr); }

// Presentación
void Hospital::mostrarInformacionBasica() const {
    std::cout << "Hospital: " << nombre << "\nDireccion: " << direccion << "\nTelefono: " << telefono << std::endl;
}

void Hospital::mostrarEstadisticas() const {
    std::cout << "=== Estadísticas del sistema ===" << std::endl;
    std::cout << "Pacientes registrados: " << pacientesRegistrados << std::endl;
    std::cout << "Doctores registrados: " << doctoresRegistrados << std::endl;
    std::cout << "Citas registradas: " << citasRegistradas << std::endl;
    std::cout << "Historiales registrados: " << historialesRegistrados << std::endl;
}
