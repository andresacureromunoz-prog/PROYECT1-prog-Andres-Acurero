#include "Cita.hpp"
#include <sstream>
#include <iomanip>

Cita::Cita() {
    id = 0;
    pacienteId = 0;
    doctorId = 0;
    fecha = "01/01/2000";
    hora = "08:00";
    atendida = false;
    cancelada = false;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Cita::Cita(int id, int pacienteId, int doctorId, const std::string& fecha, const std::string& hora) {
    this->id = id;
    this->pacienteId = pacienteId;
    this->doctorId = doctorId;
    setFecha(fecha);
    setHora(hora);
    atendida = false;
    cancelada = false;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

Cita::Cita(const Cita& otra) {
    id = otra.id;
    pacienteId = otra.pacienteId;
    doctorId = otra.doctorId;
    fecha = otra.fecha;
    hora = otra.hora;
    atendida = otra.atendida;
    cancelada = otra.cancelada;
    fechaCreacion = otra.fechaCreacion;
    fechaModificacion = otra.fechaModificacion;
}

Cita::~Cita() {}

int Cita::getId() const { return id; }
int Cita::getPacienteId() const { return pacienteId; }
int Cita::getDoctorId() const { return doctorId; }
std::string Cita::getFecha() const { return fecha; }
std::string Cita::getHora() const { return hora; }
bool Cita::estaAtendida() const { return atendida; }
bool Cita::estaCancelada() const { return cancelada; }

void Cita::setFecha(const std::string& nuevaFecha) {
    fecha = nuevaFecha;
    fechaModificacion = std::time(nullptr);
}

void Cita::setHora(const std::string& nuevaHora) {
    hora = nuevaHora;
    fechaModificacion = std::time(nullptr);
}

void Cita::setAtendida(bool valor) {
    atendida = valor;
    fechaModificacion = std::time(nullptr);
}

void Cita::setCancelada(bool valor) {
    cancelada = valor;
    fechaModificacion = std::time(nullptr);
}

bool Cita::fechaValida() const {
    // Validar formato DD/MM/AAAA y que no esté en el pasado
    int d, m, a;
    char sep;
    std::istringstream iss(fecha);
    if (!(iss >> d >> sep >> m >> sep >> a)) return false;

    // Crear objeto tm
    std::tm tmFecha = {};
    tmFecha.tm_mday = d;
    tmFecha.tm_mon = m - 1;
    tmFecha.tm_year = a - 1900;

    time_t fechaCita = std::mktime(&tmFecha);
    if (fechaCita == -1) return false;

    // Comparar con fecha actual
    time_t ahora = std::time(nullptr);
    return difftime(fechaCita, ahora) >= 0;
}

bool Cita::horaValida() const {
    // Validar formato HH:MM y rango laboral (08:00–18:00)
    int h, min;
    char sep;
    std::istringstream iss(hora);
    if (!(iss >> h >> sep >> min)) return false;

    if (h < 0 || h > 23 || min < 0 || min > 59) return false;
    return (h >= 8 && h <= 18);
}

bool Cita::validarDatos() const {
    return pacienteId > 0 && doctorId > 0 && fechaValida() && horaValida();
}

void Cita::marcarComoAtendida() {
    atendida = true;
    fechaModificacion = std::time(nullptr);
}

void Cita::cancelar() {
    cancelada = true;
    fechaModificacion = std::time(nullptr);
}

void Cita::mostrarInformacionBasica() const {
    std::cout << "Cita ID: " << id
              << " | Paciente ID: " << pacienteId
              << " | Doctor ID: " << doctorId
              << " | Fecha: " << fecha
              << " | Hora: " << hora
              << std::endl;
}

void Cita::mostrarInformacionCompleta() const {
    mostrarInformacionBasica();
    std::cout << "Atendida: " << (atendida ? "Si" : "No")
              << " | Cancelada: " << (cancelada ? "Si" : "No")
              << std::endl;
}

size_t Cita::obtenerTamano() {
    return sizeof(Cita);
}
