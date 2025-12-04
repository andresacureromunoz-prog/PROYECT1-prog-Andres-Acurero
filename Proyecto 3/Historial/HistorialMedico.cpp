#include "HistorialMedico.hpp"
#include <sstream>

HistorialMedico::HistorialMedico() {
    id = 0;
    pacienteId = 0;
    doctorId = 0;
    diagnostico = "";
    tratamiento = "";
    fecha = "01/01/2000";
    siguienteConsultaId = -1; // -1 indica que no hay siguiente
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

HistorialMedico::HistorialMedico(int id, int pacienteId, int doctorId,
                                 const std::string& diagnostico,
                                 const std::string& tratamiento,
                                 const std::string& fecha) {
    this->id = id;
    this->pacienteId = pacienteId;
    this->doctorId = doctorId;
    setDiagnostico(diagnostico);
    setTratamiento(tratamiento);
    setFecha(fecha);
    siguienteConsultaId = -1;
    activo = true;
    fechaCreacion = std::time(nullptr);
    fechaModificacion = fechaCreacion;
}

HistorialMedico::HistorialMedico(const HistorialMedico& otro) {
    id = otro.id;
    pacienteId = otro.pacienteId;
    doctorId = otro.doctorId;
    diagnostico = otro.diagnostico;
    tratamiento = otro.tratamiento;
    fecha = otro.fecha;
    siguienteConsultaId = otro.siguienteConsultaId;
    activo = otro.activo;
    fechaCreacion = otro.fechaCreacion;
    fechaModificacion = otro.fechaModificacion;
}

HistorialMedico::~HistorialMedico() {}

int HistorialMedico::getId() const { return id; }
int HistorialMedico::getPacienteId() const { return pacienteId; }
int HistorialMedico::getDoctorId() const { return doctorId; }
std::string HistorialMedico::getDiagnostico() const { return diagnostico; }
std::string HistorialMedico::getTratamiento() const { return tratamiento; }
std::string HistorialMedico::getFecha() const { return fecha; }
int HistorialMedico::getSiguienteConsultaId() const { return siguienteConsultaId; }
bool HistorialMedico::estaActivo() const { return activo; }

void HistorialMedico::setDiagnostico(const std::string& nuevoDiagnostico) {
    if (!nuevoDiagnostico.empty()) {
        diagnostico = nuevoDiagnostico;
        fechaModificacion = std::time(nullptr);
    }
}

void HistorialMedico::setTratamiento(const std::string& nuevoTratamiento) {
    if (!nuevoTratamiento.empty()) {
        tratamiento = nuevoTratamiento;
        fechaModificacion = std::time(nullptr);
    }
}

void HistorialMedico::setFecha(const std::string& nuevaFecha) {
    if (!nuevaFecha.empty()) {
        fecha = nuevaFecha;
        fechaModificacion = std::time(nullptr);
    }
}

void HistorialMedico::setSiguienteConsultaId(int id) {
    siguienteConsultaId = id;
    fechaModificacion = std::time(nullptr);
}

void HistorialMedico::setActivo(bool valor) {
    activo = valor;
    fechaModificacion = std::time(nullptr);
}

bool HistorialMedico::fechaValida() const {
    int d, m, a;
    char sep;
    std::istringstream iss(fecha);
    if (!(iss >> d >> sep >> m >> sep >> a)) return false;
    return (d > 0 && d <= 31 && m > 0 && m <= 12 && a >= 1900);
}

bool HistorialMedico::validarDatos() const {
    return pacienteId > 0 && doctorId > 0 &&
           !diagnostico.empty() && !tratamiento.empty() &&
           fechaValida();
}

void HistorialMedico::mostrarInformacionBasica() const {
    std::cout << "Historial ID: " << id
              << " | Paciente ID: " << pacienteId
              << " | Doctor ID: " << doctorId
              << " | Fecha: " << fecha
              << std::endl;
}

void HistorialMedico::mostrarInformacionCompleta() const {
    mostrarInformacionBasica();
    std::cout << "Diagnóstico: " << diagnostico
              << " | Tratamiento: " << tratamiento
              << " | Siguiente Consulta ID: " << siguienteConsultaId
              << " | Activo: " << (activo ? "Si" : "No")
              << std::endl;
}

size_t HistorialMedico::obtenerTamano() {
    return sizeof(HistorialMedico);
}
