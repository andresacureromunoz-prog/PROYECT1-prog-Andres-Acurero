/**
 * SISTEMA HOSPITALARIO POO - VERSIÓN CORREGIDA Y FINAL
 * Correcciones:
 * 1. Solucionado error de acceso a 'leerHeader' (ahora es public en GestorBase).
 * 2. Historial Médico incluye campo 'costo' y lógica de Lista Enlazada.
 * 3. Captura completa de datos en todos los formularios.
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <cstdio>

using namespace std;

// =========================================================
// 1. UTILIDADES Y CONSTANTES
// =========================================================

const char* HOSPITAL_FILE = "hospital.bin";
const char* PACIENTES_FILE = "pacientes.bin";
const char* DOCTORES_FILE = "doctores.bin";
const char* CITAS_FILE = "citas.bin";
const char* HISTORIALES_FILE = "historiales.bin";
const int VERSION_ACTUAL = 1;

class Util {
public:
    static void limpiarBuffer() {
        cin.ignore(1000, '\n');
    }

    static void pausar() {
        cout << "\nPresione Enter para continuar...";
        cin.get();
    }

    static void encabezado(const char* titulo) {
        cout << "\n========================================" << endl;
        cout << " " << titulo << endl;
        cout << "========================================" << endl;
    }
};

// =========================================================
// 2. ESTRUCTURAS DE DATOS (DTOs - Mapas de Bytes)
// =========================================================

struct ArchivoHeader {
    int cantidadRegistros;
    int proximoID;
    int registrosActivos;
    int version;
};

struct DatosHospital {
    char nombre[100];
    char direccion[150];
    char telefono[15];
    int siguienteIDPaciente;
    int siguienteIDDoctor;
    int siguienteIDCita;
    int siguienteIDConsulta;
    int totalPacientesRegistrados;
    int totalDoctoresRegistrados;
    int totalCitasAgendadas;
    int totalConsultasRealizadas;
};

// Estructura Paciente Completa
struct DatosPaciente {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];
    int edad;
    char sexo;              // M/F
    char tipoSangre[5];     // Ej: O+
    char telefono[15];
    char direccion[100];
    char email[50];
    char alergias[500];
    char observaciones[500];
    bool activo;
    
    // Lista Enlazada de Historial
    int cantidadConsultas;
    int primerConsultaID;   // Puntero al inicio del historial (-1 si vacío)
    
    int cantidadCitas;
    int citasIDs[20];       // Array fijo para citas recientes
    
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
};

// Estructura Doctor Completa
struct DatosDoctor {
    int id;
    char nombre[50];
    char apellido[50];
    char cedulaProfesional[20];
    char especialidad[50];
    int aniosExperiencia;
    float costoConsulta;
    char horarioAtencion[50];
    char telefono[15];
    char email[50];
    bool disponible;
    
    int cantidadPacientes;
    int pacientesIDs[50];
    int cantidadCitas;
    int citasIDs[30];
    
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
};

struct DatosCita {
    int id;
    int pacienteID;
    int doctorID;
    char fecha[11];
    char hora[6];
    char motivo[150];
    char estado[20]; // Agendada, Atendida, Cancelada
    char observaciones[200];
    bool atendida;
    int consultaID;
    
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
};

// Estructura Historial Médico (Nodo de Lista Enlazada)
// INCLUYE COSTO según requerimiento del Word
struct DatosHistorial {
    int id;
    int pacienteID;
    char fecha[11];
    char hora[6];
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    int doctorID;
    float costo;            
    
    // Puntero al siguiente nodo (Lista Enlazada)
    int siguienteConsultaID; // -1 indica fin de la lista
    
    bool eliminado;
    time_t fechaRegistro;
};

// =========================================================
// 3. CLASES DE DOMINIO (Lógica de Negocio)
// =========================================================

class Hospital {
private:
    DatosHospital datos;
public:
    Hospital() {
        strcpy(datos.nombre, "Hospital Central");
        strcpy(datos.direccion, "Av. Principal #123");
        strcpy(datos.telefono, "555-0100");
        datos.siguienteIDPaciente = 1;
        datos.siguienteIDDoctor = 1;
        datos.siguienteIDCita = 1;
        datos.siguienteIDConsulta = 1;
        datos.totalPacientesRegistrados = 0;
        datos.totalDoctoresRegistrados = 0;
        datos.totalCitasAgendadas = 0;
        datos.totalConsultasRealizadas = 0;
    }
    const char* getNombre() const { return datos.nombre; }
    DatosHospital& getDatosRaw() { return datos; }
    void setDatosRaw(const DatosHospital& d) { datos = d; }
    
    void incPacientes() { datos.totalPacientesRegistrados++; datos.siguienteIDPaciente++; }
    void incDoctores() { datos.totalDoctoresRegistrados++; datos.siguienteIDDoctor++; }
    void incCitas() { datos.totalCitasAgendadas++; datos.siguienteIDCita++; }
    void incConsultas() { datos.totalConsultasRealizadas++; datos.siguienteIDConsulta++; }
};

class Paciente {
private:
    DatosPaciente datos;
public:
    Paciente() {
        memset(&datos, 0, sizeof(DatosPaciente));
        datos.eliminado = false;
        datos.activo = true;
        datos.primerConsultaID = -1; // Inicialmente null
        datos.cantidadConsultas = 0;
        for(int i=0; i<20; i++) datos.citasIDs[i] = -1;
        datos.fechaCreacion = time(NULL);
    }
    
    // Setters
    void setId(int v) { datos.id = v; }
    void setNombre(const char* v) { strncpy(datos.nombre, v, 50); }
    void setApellido(const char* v) { strncpy(datos.apellido, v, 50); }
    void setCedula(const char* v) { strncpy(datos.cedula, v, 20); }
    void setEdad(int v) { datos.edad = v; }
    void setSexo(char v) { datos.sexo = v; }
    void setTipoSangre(const char* v) { strncpy(datos.tipoSangre, v, 5); }
    void setTelefono(const char* v) { strncpy(datos.telefono, v, 15); }
    void setDireccion(const char* v) { strncpy(datos.direccion, v, 100); }
    void setEmail(const char* v) { strncpy(datos.email, v, 50); }
    void setAlergias(const char* v) { strncpy(datos.alergias, v, 500); }
    void setObservaciones(const char* v) { strncpy(datos.observaciones, v, 500); }
    
    // Historial Logic
    int getPrimerConsultaID() const { return datos.primerConsultaID; }
    void setPrimerConsultaID(int id) { datos.primerConsultaID = id; }
    void incConsultas() { datos.cantidadConsultas++; }

    // Getters y Utilidad
    int getId() const { return datos.id; }
    const char* getNombre() const { return datos.nombre; }
    const char* getApellido() const { return datos.apellido; }
    const char* getCedula() const { return datos.cedula; }
    bool isEliminado() const { return datos.eliminado; }
    void marcarEliminado() { datos.eliminado = true; datos.activo = false; }
    
    DatosPaciente getDatosRaw() const { return datos; }
    void setDatosRaw(const DatosPaciente& d) { datos = d; }

    void mostrarDetalle() const {
        cout << "\n--- FICHA PACIENTE ID: " << datos.id << " ---" << endl;
        cout << "Nombre: " << datos.nombre << " " << datos.apellido << endl;
        cout << "Cedula: " << datos.cedula << " | Edad: " << datos.edad << " | Sexo: " << datos.sexo << endl;
        cout << "Tipo Sangre: " << datos.tipoSangre << endl;
        cout << "Telefono: " << datos.telefono << " | Email: " << datos.email << endl;
        cout << "Direccion: " << datos.direccion << endl;
        cout << "Alergias: " << datos.alergias << endl;
        cout << "Observaciones: " << datos.observaciones << endl;
        cout << "Consultas en Historial: " << datos.cantidadConsultas << endl;
    }
};

class Doctor {
private:
    DatosDoctor datos;
public:
    Doctor() {
        memset(&datos, 0, sizeof(DatosDoctor));
        datos.eliminado = false;
        datos.disponible = true;
        datos.fechaCreacion = time(NULL);
    }
    
    // Setters
    void setId(int v) { datos.id = v; }
    void setNombre(const char* v) { strncpy(datos.nombre, v, 50); }
    void setApellido(const char* v) { strncpy(datos.apellido, v, 50); }
    void setCedulaProf(const char* v) { strncpy(datos.cedulaProfesional, v, 20); }
    void setEspecialidad(const char* v) { strncpy(datos.especialidad, v, 50); }
    void setAniosExp(int v) { datos.aniosExperiencia = v; }
    void setCosto(float v) { datos.costoConsulta = v; }
    void setHorario(const char* v) { strncpy(datos.horarioAtencion, v, 50); }
    void setTelefono(const char* v) { strncpy(datos.telefono, v, 15); }
    void setEmail(const char* v) { strncpy(datos.email, v, 50); }
    
    // Getters
    int getId() const { return datos.id; }
    const char* getNombre() const { return datos.nombre; }
    bool isEliminado() const { return datos.eliminado; }
    void marcarEliminado() { datos.eliminado = true; }
    
    DatosDoctor getDatosRaw() const { return datos; }
    void setDatosRaw(const DatosDoctor& d) { datos = d; }

    void mostrarDetalle() const {
        cout << "\n--- FICHA DOCTOR ID: " << datos.id << " ---" << endl;
        cout << "Dr/a. " << datos.nombre << " " << datos.apellido << endl;
        cout << "Especialidad: " << datos.especialidad << " | Ced. Prof: " << datos.cedulaProfesional << endl;
        cout << "Experiencia: " << datos.aniosExperiencia << " anios | Costo Base: $" << datos.costoConsulta << endl;
        cout << "Horario: " << datos.horarioAtencion << endl;
        cout << "Contacto: " << datos.telefono << " | " << datos.email << endl;
    }
};

class HistorialMedico {
private:
    DatosHistorial datos;
public:
    HistorialMedico() {
        memset(&datos, 0, sizeof(DatosHistorial));
        datos.eliminado = false;
        datos.siguienteConsultaID = -1; // Fin de lista por defecto
        datos.fechaRegistro = time(NULL);
    }
    
    // Setters
    void setId(int v) { datos.id = v; }
    void setPacienteID(int v) { datos.pacienteID = v; }
    void setDoctorID(int v) { datos.doctorID = v; }
    void setFecha(const char* v) { strncpy(datos.fecha, v, 11); }
    void setHora(const char* v) { strncpy(datos.hora, v, 6); }
    void setDiagnostico(const char* v) { strncpy(datos.diagnostico, v, 200); }
    void setTratamiento(const char* v) { strncpy(datos.tratamiento, v, 200); }
    void setMedicamentos(const char* v) { strncpy(datos.medicamentos, v, 150); }
    void setCosto(float v) { datos.costo = v; }
    void setSiguienteID(int v) { datos.siguienteConsultaID = v; }
    
    // Getters
    int getId() const { return datos.id; }
    int getSiguienteID() const { return datos.siguienteConsultaID; }
    bool isEliminado() const { return datos.eliminado; }
    
    DatosHistorial getDatosRaw() const { return datos; }
    void setDatosRaw(const DatosHistorial& d) { datos = d; }

    void mostrarFicha() const {
        cout << "  [Consulta #" << datos.id << "] " << datos.fecha << " " << datos.hora << endl;
        cout << "   - Dx: " << datos.diagnostico << endl;
        cout << "   - Tx: " << datos.tratamiento << endl;
        cout << "   - Meds: " << datos.medicamentos << endl;
        cout << "   - Costo: $" << datos.costo << " | Atendido por DocID: " << datos.doctorID << endl;
        cout << "   -------------------------------------------------" << endl;
    }
};

class Cita {
private:
    DatosCita datos;
public:
    Cita() {
        memset(&datos, 0, sizeof(DatosCita));
        datos.eliminado = false;
        strcpy(datos.estado, "Agendada");
    }
    
    void setId(int v) { datos.id = v; }
    void setPacienteID(int v) { datos.pacienteID = v; }
    void setDoctorID(int v) { datos.doctorID = v; }
    void setFecha(const char* v) { strncpy(datos.fecha, v, 11); }
    void setHora(const char* v) { strncpy(datos.hora, v, 6); }
    void setMotivo(const char* v) { strncpy(datos.motivo, v, 150); }
    
    bool isEliminado() const { return datos.eliminado; }
    DatosCita getDatosRaw() const { return datos; }
    void setDatosRaw(const DatosCita& d) { datos = d; }

    void mostrarFicha() const {
        cout << "Cita #" << datos.id << " | Paciente ID:" << datos.pacienteID 
             << " | Fecha: " << datos.fecha << " " << datos.hora 
             << " | Estado: " << datos.estado << endl;
    }
};

// =========================================================
// 4. GESTORES (Persistencia y Lógica)
// =========================================================

// CORRECCIÓN PRINCIPAL: Métodos 'public' para evitar error de acceso
class GestorBase {
public: 
    bool inicializarArchivo(const char* nombre) {
        ofstream archivo(nombre, ios::binary | ios::out);
        if (!archivo.is_open()) return false;
        ArchivoHeader header = {0, 1, 0, VERSION_ACTUAL}; 
        archivo.write((char*)&header, sizeof(ArchivoHeader));
        archivo.close();
        return true;
    }
    bool verificarArchivo(const char* nombre) {
        ifstream archivo(nombre, ios::binary);
        if (!archivo.is_open()) return false;
        ArchivoHeader header;
        archivo.read((char*)&header, sizeof(ArchivoHeader));
        bool ok = (!archivo.fail() && header.version == VERSION_ACTUAL);
        archivo.close();
        return ok;
    }
    ArchivoHeader leerHeader(const char* nombre) {
        ArchivoHeader header = {0, 0, 0, 0};
        ifstream archivo(nombre, ios::binary);
        if (archivo.is_open()) {
            archivo.read((char*)&header, sizeof(ArchivoHeader));
            archivo.close();
        }
        return header;
    }
    void actualizarHeader(const char* nombre, ArchivoHeader header) {
        fstream archivo(nombre, ios::binary | ios::in | ios::out);
        if (archivo.is_open()) {
            archivo.seekp(0);
            archivo.write((char*)&header, sizeof(ArchivoHeader));
            archivo.close();
        }
    }
    long calcularPos(int indice, size_t tam) {
        return sizeof(ArchivoHeader) + ((long)indice * tam);
    }
};

class GestorHospital : public GestorBase {
private:
    Hospital hospital;
public:
    void cargar() {
        if (!verificarArchivo(HOSPITAL_FILE)) {
            ofstream archivo(HOSPITAL_FILE, ios::binary | ios::out);
            DatosHospital raw = hospital.getDatosRaw();
            archivo.write((char*)&raw, sizeof(DatosHospital));
            archivo.close();
            if(!verificarArchivo(PACIENTES_FILE)) inicializarArchivo(PACIENTES_FILE);
            if(!verificarArchivo(DOCTORES_FILE)) inicializarArchivo(DOCTORES_FILE);
            if(!verificarArchivo(CITAS_FILE)) inicializarArchivo(CITAS_FILE);
            if(!verificarArchivo(HISTORIALES_FILE)) inicializarArchivo(HISTORIALES_FILE);
        } else {
            ifstream archivo(HOSPITAL_FILE, ios::binary);
            DatosHospital raw;
            archivo.read((char*)&raw, sizeof(DatosHospital));
            hospital.setDatosRaw(raw);
            archivo.close();
        }
    }
    void guardar() {
        fstream archivo(HOSPITAL_FILE, ios::binary | ios::in | ios::out);
        DatosHospital raw = hospital.getDatosRaw();
        archivo.seekp(0);
        archivo.write((char*)&raw, sizeof(DatosHospital));
        archivo.close();
    }
    Hospital& getHospital() { return hospital; }
};

class GestorPacientes : public GestorBase {
public:
    void agregar(Paciente& nuevo, Hospital& hRef) {
        ArchivoHeader header = leerHeader(PACIENTES_FILE);
        nuevo.setId(header.proximoID);
        ofstream archivo(PACIENTES_FILE, ios::binary | ios::app);
        DatosPaciente raw = nuevo.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosPaciente));
        archivo.close();
        
        header.cantidadRegistros++; header.proximoID++; header.registrosActivos++;
        actualizarHeader(PACIENTES_FILE, header);
        hRef.incPacientes();
    }

    bool buscarPorID(int id, Paciente& resultado) {
        ifstream archivo(PACIENTES_FILE, ios::binary);
        if(!archivo.is_open()) return false;
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosPaciente temp;
        for(int i=0; i<h.cantidadRegistros; i++){
            archivo.read((char*)&temp, sizeof(DatosPaciente));
            if(temp.id == id) {
                resultado.setDatosRaw(temp);
                archivo.close();
                return true;
            }
        }
        archivo.close();
        return false;
    }
    
    bool buscarPorCedula(const char* ced, Paciente& resultado) {
        ifstream archivo(PACIENTES_FILE, ios::binary);
        if(!archivo.is_open()) return false;
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosPaciente temp;
        for(int i=0; i<h.cantidadRegistros; i++){
            archivo.read((char*)&temp, sizeof(DatosPaciente));
            if(!temp.eliminado && strcmp(temp.cedula, ced) == 0) {
                resultado.setDatosRaw(temp);
                archivo.close();
                return true;
            }
        }
        archivo.close();
        return false;
    }

    bool actualizar(Paciente& p) {
        ifstream lector(PACIENTES_FILE, ios::binary);
        ArchivoHeader h; lector.read((char*)&h, sizeof(ArchivoHeader));
        int idx = -1;
        DatosPaciente temp;
        for(int i=0; i<h.cantidadRegistros; i++){
            lector.read((char*)&temp, sizeof(DatosPaciente));
            if(temp.id == p.getId()) { idx = i; break; }
        }
        lector.close();
        if(idx == -1) return false;

        fstream archivo(PACIENTES_FILE, ios::binary | ios::in | ios::out);
        archivo.seekp(calcularPos(idx, sizeof(DatosPaciente)));
        DatosPaciente raw = p.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosPaciente));
        archivo.close();
        return true;
    }
    
    bool eliminar(int id, Hospital& hRef) {
        Paciente p;
        if(buscarPorID(id, p) && !p.isEliminado()) {
            p.marcarEliminado();
            actualizar(p);
            ArchivoHeader h = leerHeader(PACIENTES_FILE);
            h.registrosActivos--;
            actualizarHeader(PACIENTES_FILE, h);
            hRef.getDatosRaw().totalPacientesRegistrados--;
            return true;
        }
        return false;
    }

    void listar() {
        ifstream archivo(PACIENTES_FILE, ios::binary);
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosPaciente p;
        cout << left << setw(5) << "ID" << setw(15) << "CEDULA" << setw(25) << "NOMBRE" << setw(15) << "TELEFONO" << endl;
        for(int i=0; i<h.cantidadRegistros; i++){
            archivo.read((char*)&p, sizeof(DatosPaciente));
            if(!p.eliminado) {
                cout << left << setw(5) << p.id << setw(15) << p.cedula 
                     << setw(25) << (string(p.nombre) + " " + p.apellido) 
                     << setw(15) << p.telefono << endl;
            }
        }
        archivo.close();
    }
    
    void compactar() {
        ifstream orig(PACIENTES_FILE, ios::binary);
        ofstream temp("temp_pac.bin", ios::binary | ios::out);
        ArchivoHeader hOrig; orig.read((char*)&hOrig, sizeof(ArchivoHeader));
        ArchivoHeader hNew = hOrig; hNew.cantidadRegistros = 0; hNew.registrosActivos = 0; hNew.proximoID = 1;
        temp.write((char*)&hNew, sizeof(ArchivoHeader));
        DatosPaciente p;
        while(orig.read((char*)&p, sizeof(DatosPaciente))) {
            if(!p.eliminado) {
                p.id = hNew.proximoID++;
                temp.write((char*)&p, sizeof(DatosPaciente));
                hNew.cantidadRegistros++; hNew.registrosActivos++;
            }
        }
        orig.close(); temp.close();
        actualizarHeader("temp_pac.bin", hNew);
        remove(PACIENTES_FILE); rename("temp_pac.bin", PACIENTES_FILE);
        cout << "Pacientes compactados.\n";
    }
};

class GestorDoctores : public GestorBase {
public:
    void agregar(Doctor& nuevo, Hospital& hRef) {
        ArchivoHeader header = leerHeader(DOCTORES_FILE);
        nuevo.setId(header.proximoID);
        ofstream archivo(DOCTORES_FILE, ios::binary | ios::app);
        DatosDoctor raw = nuevo.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosDoctor));
        archivo.close();
        header.cantidadRegistros++; header.proximoID++; header.registrosActivos++;
        actualizarHeader(DOCTORES_FILE, header);
        hRef.incDoctores();
    }
    
    bool buscarPorID(int id, Doctor& res) {
        ifstream archivo(DOCTORES_FILE, ios::binary);
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosDoctor temp;
        while(archivo.read((char*)&temp, sizeof(DatosDoctor))) {
            if(temp.id == id) {
                res.setDatosRaw(temp);
                archivo.close();
                return true;
            }
        }
        archivo.close();
        return false;
    }
    
    bool actualizar(Doctor& d) {
        ifstream lector(DOCTORES_FILE, ios::binary);
        ArchivoHeader h; lector.read((char*)&h, sizeof(ArchivoHeader));
        int idx = -1;
        DatosDoctor temp;
        for(int i=0; i<h.cantidadRegistros; i++){
            lector.read((char*)&temp, sizeof(DatosDoctor));
            if(temp.id == d.getId()) { idx = i; break; }
        }
        lector.close();
        if(idx == -1) return false;

        fstream archivo(DOCTORES_FILE, ios::binary | ios::in | ios::out);
        archivo.seekp(calcularPos(idx, sizeof(DatosDoctor)));
        DatosDoctor raw = d.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosDoctor));
        archivo.close();
        return true;
    }

    bool eliminar(int id, Hospital& hRef) {
        Doctor d;
        if(buscarPorID(id, d) && !d.isEliminado()) {
            d.marcarEliminado();
            actualizar(d);
            ArchivoHeader h = leerHeader(DOCTORES_FILE);
            h.registrosActivos--;
            actualizarHeader(DOCTORES_FILE, h);
            hRef.getDatosRaw().totalDoctoresRegistrados--;
            return true;
        }
        return false;
    }

    void listar() {
        ifstream archivo(DOCTORES_FILE, ios::binary);
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosDoctor d;
        cout << left << setw(5) << "ID" << setw(25) << "NOMBRE" << setw(20) << "ESPECIALIDAD" << setw(10) << "COSTO" << endl;
        while(archivo.read((char*)&d, sizeof(DatosDoctor))) {
            if(!d.eliminado) {
                cout << left << setw(5) << d.id << setw(25) << (string(d.nombre) + " " + d.apellido) 
                     << setw(20) << d.especialidad << setw(10) << d.costoConsulta << endl;
            }
        }
        archivo.close();
    }
    
    void compactar() {
        ifstream orig(DOCTORES_FILE, ios::binary);
        ofstream temp("temp_doc.bin", ios::binary | ios::out);
        ArchivoHeader hOrig; orig.read((char*)&hOrig, sizeof(ArchivoHeader));
        ArchivoHeader hNew = hOrig; hNew.cantidadRegistros = 0; hNew.registrosActivos = 0; hNew.proximoID = 1;
        temp.write((char*)&hNew, sizeof(ArchivoHeader));
        DatosDoctor d;
        while(orig.read((char*)&d, sizeof(DatosDoctor))) {
            if(!d.eliminado) {
                d.id = hNew.proximoID++;
                temp.write((char*)&d, sizeof(DatosDoctor));
                hNew.cantidadRegistros++; hNew.registrosActivos++;
            }
        }
        orig.close(); temp.close();
        actualizarHeader("temp_doc.bin", hNew);
        remove(DOCTORES_FILE); rename("temp_doc.bin", DOCTORES_FILE);
        cout << "Doctores compactados.\n";
    }
};

class GestorHistorial : public GestorBase {
public:
    bool buscarPorID(int id, HistorialMedico& res) {
        ifstream archivo(HISTORIALES_FILE, ios::binary);
        if(!archivo.is_open()) return false;
        ArchivoHeader h; archivo.read((char*)&h, sizeof(ArchivoHeader));
        DatosHistorial temp;
        while(archivo.read((char*)&temp, sizeof(DatosHistorial))) {
            if(temp.id == id) {
                res.setDatosRaw(temp);
                archivo.close();
                return true;
            }
        }
        archivo.close();
        return false;
    }

    // Método crucial para persistir los enlaces de la lista
    bool actualizar(HistorialMedico& hm) {
        ifstream lector(HISTORIALES_FILE, ios::binary);
        ArchivoHeader h; lector.read((char*)&h, sizeof(ArchivoHeader));
        int idx = -1;
        DatosHistorial temp;
        for(int i=0; i<h.cantidadRegistros; i++){
            lector.read((char*)&temp, sizeof(DatosHistorial));
            if(temp.id == hm.getId()) { idx = i; break; }
        }
        lector.close();
        if(idx == -1) return false;

        fstream archivo(HISTORIALES_FILE, ios::binary | ios::in | ios::out);
        archivo.seekp(calcularPos(idx, sizeof(DatosHistorial)));
        DatosHistorial raw = hm.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosHistorial));
        archivo.close();
        return true;
    }

    void agregar(HistorialMedico& nueva, int pid, GestorPacientes& gp, Hospital& hRef) {
        Paciente p;
        if(!gp.buscarPorID(pid, p)) { cout << "Paciente no encontrado." << endl; return; }

        // 1. Crear nuevo registro
        ArchivoHeader hHist = leerHeader(HISTORIALES_FILE);
        int nuevoID = hHist.proximoID;
        nueva.setId(nuevoID);
        nueva.setPacienteID(pid);
        nueva.setSiguienteID(-1); // Es el último

        ofstream archivo(HISTORIALES_FILE, ios::binary | ios::app);
        DatosHistorial raw = nueva.getDatosRaw();
        archivo.write((char*)&raw, sizeof(DatosHistorial));
        archivo.close();

        hHist.cantidadRegistros++; hHist.proximoID++; hHist.registrosActivos++;
        actualizarHeader(HISTORIALES_FILE, hHist);
        hRef.incConsultas();

        // 2. Enlazar (Linked List Logic)
        if(p.getPrimerConsultaID() == -1) {
            // Caso A: Primera consulta
            p.setPrimerConsultaID(nuevoID);
        } else {
            // Caso B: Recorrer hasta el final y enlazar
            int currID = p.getPrimerConsultaID();
            HistorialMedico currHist;
            while(true) {
                if(!buscarPorID(currID, currHist)) break; 
                
                if(currHist.getSiguienteID() == -1) {
                    // Encontrado el último. Enlazar al nuevo.
                    currHist.setSiguienteID(nuevoID);
                    actualizar(currHist); // GUARDAR EL ENLACE EN DISCO
                    break;
                }
                currID = currHist.getSiguienteID();
            }
        }
        
        // 3. Actualizar Paciente
        p.incConsultas();
        gp.actualizar(p);
        cout << "Consulta agregada y enlazada (ID: " << nuevoID << ")." << endl;
    }

    void listarPorPaciente(int pid, GestorPacientes& gp) {
        Paciente p;
        if(!gp.buscarPorID(pid, p)) { cout << "Paciente inexistente." << endl; return; }
        
        cout << "\n--- HISTORIAL MEDICO: " << p.getNombre() << " " << p.getApellido() << " ---" << endl;
        
        int currID = p.getPrimerConsultaID();
        if(currID == -1) {
            cout << "No hay consultas registradas para este paciente." << endl;
            return;
        }

        HistorialMedico h;
        while(currID != -1) {
            if(buscarPorID(currID, h)) {
                if(!h.isEliminado()) {
                    h.mostrarFicha();
                }
                currID = h.getSiguienteID();
            } else {
                cout << "Error de integridad en historial ID " << currID << endl;
                break;
            }
        }
    }
};

class GestorCitas : public GestorBase {
public:
    void agendar(Cita& nueva, Hospital& hRef) {
        ArchivoHeader h = leerHeader(CITAS_FILE);
        nueva.setId(h.proximoID);
        ofstream f(CITAS_FILE, ios::binary | ios::app);
        DatosCita raw = nueva.getDatosRaw();
        f.write((char*)&raw, sizeof(DatosCita));
        f.close();
        h.cantidadRegistros++; h.proximoID++; h.registrosActivos++;
        actualizarHeader(CITAS_FILE, h);
        hRef.incCitas();
    }
    
    void listar() {
        ifstream f(CITAS_FILE, ios::binary);
        ArchivoHeader h; f.read((char*)&h, sizeof(ArchivoHeader));
        DatosCita raw;
        Cita cObj;
        cout << "--- LISTADO DE CITAS ---" << endl;
        while(f.read((char*)&raw, sizeof(DatosCita))) {
            if(!raw.eliminado) {
                cObj.setDatosRaw(raw);
                cObj.mostrarFicha();
            }
        }
        f.close();
    }
    
    void compactar() {
        ifstream orig(CITAS_FILE, ios::binary);
        ofstream temp("temp_cit.bin", ios::binary | ios::out);
        ArchivoHeader hOrig; orig.read((char*)&hOrig, sizeof(ArchivoHeader));
        ArchivoHeader hNew = hOrig; hNew.cantidadRegistros = 0; hNew.registrosActivos = 0; hNew.proximoID = 1;
        temp.write((char*)&hNew, sizeof(ArchivoHeader));
        DatosCita c;
        while(orig.read((char*)&c, sizeof(DatosCita))) {
            if(!c.eliminado) {
                c.id = hNew.proximoID++;
                temp.write((char*)&c, sizeof(DatosCita));
                hNew.cantidadRegistros++; hNew.registrosActivos++;
            }
        }
        orig.close(); temp.close();
        actualizarHeader("temp_cit.bin", hNew);
        remove(CITAS_FILE); rename("temp_cit.bin", CITAS_FILE);
        cout << "Citas compactadas.\n";
    }
};

// =========================================================
// 5. MENÚS DE INTERFAZ (Captura Detallada)
// =========================================================

void menuPacientes(GestorPacientes& gp, Hospital& hosp) {
    int op;
    do {
        Util::encabezado("GESTION DE PACIENTES");
        cout << "1. Registrar Nuevo\n2. Listar Todos\n3. Buscar y Ver Detalle (ID)\n4. Buscar y Ver Detalle (Cedula)\n5. Eliminar\n0. Volver\nOpcion: ";
        cin >> op; Util::limpiarBuffer();
        switch(op) {
            case 1: {
                Paciente p; char buff[500]; int n; char c;
                cout << "\n--- REGISTRO DE PACIENTE ---" << endl;
                cout << "Nombre: "; cin.getline(buff, 50); p.setNombre(buff);
                cout << "Apellido: "; cin.getline(buff, 50); p.setApellido(buff);
                cout << "Cedula: "; cin.getline(buff, 20); p.setCedula(buff);
                cout << "Edad: "; cin >> n; p.setEdad(n);
                cout << "Sexo (M/F): "; cin >> c; p.setSexo(c); Util::limpiarBuffer();
                cout << "Tipo Sangre: "; cin.getline(buff, 5); p.setTipoSangre(buff);
                cout << "Telefono: "; cin.getline(buff, 15); p.setTelefono(buff);
                cout << "Direccion: "; cin.getline(buff, 100); p.setDireccion(buff);
                cout << "Email: "; cin.getline(buff, 50); p.setEmail(buff);
                cout << "Alergias: "; cin.getline(buff, 500); p.setAlergias(buff);
                cout << "Observaciones: "; cin.getline(buff, 500); p.setObservaciones(buff);
                
                gp.agregar(p, hosp);
                cout << ">> Paciente registrado con ID: " << gp.leerHeader(PACIENTES_FILE).proximoID - 1 << endl;
                Util::pausar();
                break;
            }
            case 2: gp.listar(); Util::pausar(); break;
            case 3: {
                int id; cout << "ID: "; cin >> id;
                Paciente p;
                if(gp.buscarPorID(id, p)) p.mostrarDetalle();
                else cout << "No encontrado." << endl;
                Util::pausar();
                break;
            }
            case 4: {
                char ced[20]; cout << "Cedula: "; cin.getline(ced, 20);
                Paciente p;
                if(gp.buscarPorCedula(ced, p)) p.mostrarDetalle();
                else cout << "No encontrado." << endl;
                Util::pausar();
                break;
            }
            case 5: {
                int id; cout << "ID a eliminar: "; cin >> id;
                if(gp.eliminar(id, hosp)) cout << "Eliminado." << endl;
                else cout << "Error o no encontrado." << endl;
                Util::pausar();
                break;
            }
        }
    } while(op != 0);
}

void menuDoctores(GestorDoctores& gd, Hospital& hosp) {
    int op;
    do {
        Util::encabezado("GESTION DE DOCTORES");
        cout << "1. Registrar Nuevo\n2. Listar Todos\n3. Ver Detalle (ID)\n4. Eliminar\n0. Volver\nOpcion: ";
        cin >> op; Util::limpiarBuffer();
        switch(op) {
            case 1: {
                Doctor d; char buff[100]; float f; int n;
                cout << "\n--- REGISTRO DE DOCTOR ---" << endl;
                cout << "Nombre: "; cin.getline(buff, 50); d.setNombre(buff);
                cout << "Apellido: "; cin.getline(buff, 50); d.setApellido(buff);
                cout << "Cedula Prof: "; cin.getline(buff, 20); d.setCedulaProf(buff);
                cout << "Especialidad: "; cin.getline(buff, 50); d.setEspecialidad(buff);
                cout << "Anios Experiencia: "; cin >> n; d.setAniosExp(n);
                cout << "Costo Consulta: "; cin >> f; d.setCosto(f); Util::limpiarBuffer();
                cout << "Horario Atencion: "; cin.getline(buff, 50); d.setHorario(buff);
                cout << "Telefono: "; cin.getline(buff, 15); d.setTelefono(buff);
                cout << "Email: "; cin.getline(buff, 50); d.setEmail(buff);
                
                gd.agregar(d, hosp);
                cout << ">> Doctor registrado." << endl;
                Util::pausar();
                break;
            }
            case 2: gd.listar(); Util::pausar(); break;
            case 3: {
                int id; cout << "ID: "; cin >> id;
                Doctor d;
                if(gd.buscarPorID(id, d)) d.mostrarDetalle();
                else cout << "No encontrado." << endl;
                Util::pausar();
                break;
            }
            case 4: {
                int id; cout << "ID a eliminar: "; cin >> id;
                if(gd.eliminar(id, hosp)) cout << "Eliminado." << endl;
                else cout << "Error." << endl;
                Util::pausar();
                break;
            }
        }
    } while(op != 0);
}

void menuCitas(GestorCitas& gc, GestorPacientes& gp, GestorDoctores& gd, Hospital& hosp) {
    int op;
    do {
        Util::encabezado("GESTION DE CITAS");
        cout << "1. Agendar Cita\n2. Listar Citas\n0. Volver\nOpcion: ";
        cin >> op; Util::limpiarBuffer();
        switch(op) {
            case 1: {
                int pid, did; char buff[150];
                cout << "\n--- AGENDAR CITA ---" << endl;
                cout << "ID Paciente: "; cin >> pid;
                cout << "ID Doctor: "; cin >> did; Util::limpiarBuffer();
                
                Paciente p; Doctor d;
                if(!gp.buscarPorID(pid, p) || !gd.buscarPorID(did, d)) {
                    cout << "Error: ID de paciente o doctor incorrecto." << endl;
                    Util::pausar(); break;
                }
                
                Cita c;
                c.setPacienteID(pid); c.setDoctorID(did);
                cout << "Fecha (YYYY-MM-DD): "; cin.getline(buff, 11); c.setFecha(buff);
                cout << "Hora (HH:MM): "; cin.getline(buff, 6); c.setHora(buff);
                cout << "Motivo: "; cin.getline(buff, 150); c.setMotivo(buff);
                
                gc.agendar(c, hosp);
                cout << ">> Cita agendada." << endl;
                Util::pausar();
                break;
            }
            case 2: gc.listar(); Util::pausar(); break;
        }
    } while(op != 0);
}

void menuHistorial(GestorHistorial& gh, GestorPacientes& gp, GestorDoctores& gd, Hospital& hosp) {
    int op;
    do {
        Util::encabezado("HISTORIAL MEDICO (LISTA ENLAZADA)");
        cout << "1. Registrar Consulta\n2. Ver Historial (Buscar por ID Paciente)\n3. Ver Historial (Buscar por Cedula)\n0. Volver\nOpcion: ";
        cin >> op; Util::limpiarBuffer();
        switch(op) {
            case 1: {
                int pid, did; char buff[200]; float costo;
                cout << "\n--- NUEVA CONSULTA ---" << endl;
                cout << "ID Paciente: "; cin >> pid;
                cout << "ID Doctor: "; cin >> did; Util::limpiarBuffer();
                
                HistorialMedico h;
                // Captura detallada
                cout << "Fecha (YYYY-MM-DD): "; cin.getline(buff, 11); h.setFecha(buff);
                cout << "Hora (HH:MM): "; cin.getline(buff, 6); h.setHora(buff);
                cout << "Diagnostico: "; cin.getline(buff, 200); h.setDiagnostico(buff);
                cout << "Tratamiento: "; cin.getline(buff, 200); h.setTratamiento(buff);
                cout << "Medicamentos: "; cin.getline(buff, 150); h.setMedicamentos(buff);
                cout << "Costo Consulta: "; cin >> costo; h.setCosto(costo);
                h.setDoctorID(did);
                
                gh.agregar(h, pid, gp, hosp);
                Util::pausar();
                break;
            }
            case 2: {
                int pid; cout << "ID Paciente: "; cin >> pid;
                gh.listarPorPaciente(pid, gp);
                Util::pausar();
                break;
            }
            case 3: {
                char ced[20]; cout << "Cedula Paciente: "; cin.getline(ced, 20);
                Paciente p;
                if(gp.buscarPorCedula(ced, p)) {
                    gh.listarPorPaciente(p.getId(), gp);
                } else {
                    cout << "Paciente no encontrado." << endl;
                }
                Util::pausar();
                break;
            }
        }
    } while(op != 0);
}

void menuMantenimiento(GestorPacientes& gp, GestorDoctores& gd, GestorCitas& gc) {
    Util::encabezado("MANTENIMIENTO Y COMPACTACION");
    cout << "1. Compactar Pacientes\n2. Compactar Doctores\n3. Compactar Citas\n0. Volver\nOpcion: ";
    int op; cin >> op;
    switch(op) {
        case 1: gp.compactar(); break;
        case 2: gd.compactar(); break;
        case 3: gc.compactar(); break;
    }
    if(op!=0) Util::pausar();
}

// =========================================================
// MAIN
// =========================================================

int main() {
    GestorHospital gHospital;
    GestorPacientes gPacientes;
    GestorDoctores gDoctores;
    GestorCitas gCitas;
    GestorHistorial gHistorial;
    
    gHospital.cargar();
    Hospital& hosp = gHospital.getHospital();

    int op;
    do {
        cout << "\n========================================" << endl;
        cout << " SISTEMA HOSPITALARIO INTEGRAL POO " << endl;
        cout << " Institucion: " << hosp.getNombre() << endl;
        cout << "========================================" << endl;
        cout << "1. Gestion Pacientes" << endl;
        cout << "2. Gestion Doctores" << endl;
        cout << "3. Gestion Citas" << endl;
        cout << "4. Mantenimiento" << endl;
        cout << "5. Gestion Historial Medico" << endl;
        cout << "6. Salir" << endl;
        cout << "Opcion: ";
        cin >> op;

        switch(op) {
            case 1: menuPacientes(gPacientes, hosp); break;
            case 2: menuDoctores(gDoctores, hosp); break;
            case 3: menuCitas(gCitas, gPacientes, gDoctores, hosp); break;
            case 4: menuMantenimiento(gPacientes, gDoctores, gCitas); break;
            case 5: menuHistorial(gHistorial, gPacientes, gDoctores, hosp); break;
            case 6: gHospital.guardar(); cout << "Datos guardados. Cerrando..." << endl; break;
        }
    } while(op != 6);

    return 0;
}