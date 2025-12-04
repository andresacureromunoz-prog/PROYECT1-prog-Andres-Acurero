#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <cstdio> // Para rename y remove

using namespace std;

// =========================================================
// 1. ESTRUCTURAS DE DATOS (SEGuN DOCUMENTO ANEXO)
// =========================================================

// Archivos de datos
const char* HOSPITAL_FILE = "hospital.bin";
const char* PACIENTES_FILE = "pacientes.bin";
const char* DOCTORES_FILE = "doctores.bin";
const char* CITAS_FILE = "citas.bin";
const char* HISTORIALES_FILE = "historiales.bin";
const int VERSION_ACTUAL = 1;

// Estructura del Header de Archivo (Metadata para gestion binaria)
struct ArchivoHeader {
    int cantidadRegistros;      
    int proximoID;              
    int registrosActivos;       
    int version;                
};


struct Hospital {
    char nombre[100];
    char direccion[150];
    char telefono[15];
    
    // Contadores de IDs (auto-increment)
    int siguienteIDPaciente;
    int siguienteIDDoctor;
    int siguienteIDCita;
    int siguienteIDConsulta;
    
    // Estadisticas generales
    int totalPacientesRegistrados;
    int totalDoctoresRegistrados;
    int totalCitasAgendadas;
    int totalConsultasRealizadas;
};


struct Paciente {
    int id;
    char nombre[50];
    char apellido[50];
    char cedula[20];
    int edad;
    char sexo; // 'M' o 'F'
    char tipoSangre[5];
    char telefono[15];
    char direccion[100];
    char email[50];
    char alergias[500];
    char observaciones[500];
    bool activo;
    
    // indices para relaciones
    int cantidadConsultas;      
    int primerConsultaID;       
    
    int cantidadCitas;
    int citasIDs[20];           
    
    // Metadata de registro
    bool eliminado;             
    time_t fechaCreacion;
    time_t fechaModificacion;
};


struct Doctor {
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
    
    // Relaciones con arrays fijos
    int cantidadPacientes;
    int pacientesIDs[50];       
    
    int cantidadCitas;
    int citasIDs[30];           
    
    // Metadata
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
};


struct Cita {
    int id;
    int pacienteID;
    int doctorID;
    char fecha[11];             
    char hora[6];               
    char motivo[150];
    char estado[20];            
    char observaciones[200];
    bool atendida;
    
    // Referencia al historial
    int consultaID;            
    
    // Metadata
    bool eliminado;
    time_t fechaCreacion;
    time_t fechaModificacion;
};


struct HistorialMedico {
    int id;
    int pacienteID;             
    char fecha[11];            
    char hora[6];               
    char diagnostico[200];
    char tratamiento[200];
    char medicamentos[150];
    int doctorID;
    float costo;
    
    // Navegacion enlazada
    int siguienteConsultaID;   
    
    // Metadata
    bool eliminado;
    time_t fechaRegistro;
};

// =========================================================
// UTILIDADES
// =========================================================
void mostrarTime(time_t t) {
    if (t == 0) return;
    struct tm* timeinfo = localtime(&t);
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    cout << buffer;
}

// =========================================================
// GESTIoN DE ARCHIVOS BINARIOS (HEADERS Y LECTURA)
// =========================================================

long calcularPosicion(int indice, size_t recordSize) {
    return sizeof(ArchivoHeader) + ((long)indice * recordSize);
}

bool inicializarArchivo(const char* nombreArchivo) {
    ofstream archivo(nombreArchivo, ios::binary | ios::out);
    if (!archivo.is_open()) return false;
    ArchivoHeader header = {0, 1, 0, VERSION_ACTUAL}; 
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool verificarArchivo(const char* nombreArchivo) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo.is_open()) return false;
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    bool valido = (!archivo.fail() && header.version == VERSION_ACTUAL);
    archivo.close();
    return valido;
}

ArchivoHeader leerHeader(const char* nombreArchivo) {
    ArchivoHeader header = {0, 0, 0, 0};
    ifstream archivo(nombreArchivo, ios::binary);
    if (archivo.is_open()) {
        archivo.read((char*)&header, sizeof(ArchivoHeader));
        archivo.close();
    }
    return header;
}

bool actualizarHeader(const char* nombreArchivo, ArchivoHeader header) {
    fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    archivo.seekp(0);
    archivo.write((char*)&header, sizeof(ArchivoHeader));
    archivo.close();
    return true;
}


int buscarIndiceDeID(const char* nombreArchivo, int id, size_t recordSize) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo.is_open()) return -1;

    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));
    
    char* buffer = new char[recordSize];
    int currentID = -1;
    
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read(buffer, recordSize);
        if (archivo.eof()) break;
        // Asumimos que el ID es el primer campo (int) de todas las estructuras
        memcpy(&currentID, buffer, sizeof(int)); 
        
        if (currentID == id) {
            delete[] buffer;
            archivo.close();
            return i;
        }
    }
    delete[] buffer;
    archivo.close();
    return -1;
}

bool leerRegistroPorIndice(const char* nombreArchivo, int indice, size_t recordSize, void* registro) {
    ifstream archivo(nombreArchivo, ios::binary);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicion(indice, recordSize);
    archivo.seekg(posicion);
    archivo.read((char*)registro, recordSize);
    
    bool leido = !archivo.fail();
    archivo.close();
    return leido;
}

bool actualizarRegistroPorIndice(const char* nombreArchivo, int indice, size_t recordSize, const void* registro) {
    fstream archivo(nombreArchivo, ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    
    long posicion = calcularPosicion(indice, recordSize);
    archivo.seekp(posicion);
    archivo.write((const char*)registro, recordSize);
    
    bool escrito = !archivo.fail();
    archivo.close();
    return escrito;
}

// =========================================================
// GESTIoN DEL HOSPITAL
// =========================================================

Hospital* cargarDatosHospital() {
    Hospital* hospital = new Hospital();
    
    if (!verificarArchivo(HOSPITAL_FILE)) {
        // Inicializar si no existe
        ofstream archivo(HOSPITAL_FILE, ios::binary | ios::out);
        strcpy(hospital->nombre, "Hospital Central");
        strcpy(hospital->direccion, "Av. Principal #123");
        strcpy(hospital->telefono, "555-0000");
        hospital->siguienteIDPaciente = 1;
        hospital->siguienteIDDoctor = 1;
        hospital->siguienteIDCita = 1;
        hospital->siguienteIDConsulta = 1;
        hospital->totalPacientesRegistrados = 0;
        hospital->totalDoctoresRegistrados = 0;
        hospital->totalCitasAgendadas = 0;
        hospital->totalConsultasRealizadas = 0;
        archivo.write((char*)hospital, sizeof(Hospital));
        archivo.close();
    } else {
        ifstream archivo(HOSPITAL_FILE, ios::binary);
        archivo.read((char*)hospital, sizeof(Hospital));
        archivo.close();
    }

    // Asegurar existencia de los otros archivos
    const char* archivos[] = {PACIENTES_FILE, DOCTORES_FILE, CITAS_FILE, HISTORIALES_FILE};
    for (int i = 0; i < 4; ++i) {
        if (!verificarArchivo(archivos[i])) inicializarArchivo(archivos[i]);
    }
    return hospital;
}

bool guardarDatosHospital(Hospital* hospital) {
    fstream archivo(HOSPITAL_FILE, ios::binary | ios::in | ios::out);
    if (!archivo.is_open()) return false;
    archivo.seekp(0);
    archivo.write((char*)hospital, sizeof(Hospital));
    archivo.close();
    return true;
}

// =========================================================
// MoDULO PACIENTES
// =========================================================

// Busqueda por Cedula (Iteracion Secuencial)
int buscarIndicePorCedulaPaciente(const char* cedulaBuscada) {
    ifstream archivo(PACIENTES_FILE, ios::binary);
    if (!archivo.is_open()) return -1;

    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));

    Paciente temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Paciente));
        if (!temp.eliminado && strcmp(temp.cedula, cedulaBuscada) == 0) {
            archivo.close();
            return i; // Retorna indice fisico
        }
    }
    archivo.close();
    return -1;
}

bool agregarPaciente(Hospital* hospital, Paciente nuevo) {
    ArchivoHeader header = leerHeader(PACIENTES_FILE);
    
    nuevo.id = header.proximoID;
    nuevo.eliminado = false;
    nuevo.activo = true;
    nuevo.fechaCreacion = time(NULL);
    nuevo.fechaModificacion = time(NULL);
    
    // Inicializar relaciones vacias
    nuevo.cantidadConsultas = 0;
    nuevo.primerConsultaID = -1;
    nuevo.cantidadCitas = 0;
    for(int i=0; i<20; i++) nuevo.citasIDs[i] = -1;

    ofstream archivo(PACIENTES_FILE, ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    archivo.write((char*)&nuevo, sizeof(Paciente));
    archivo.close();

    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeader(PACIENTES_FILE, header);

    hospital->siguienteIDPaciente = header.proximoID;
    hospital->totalPacientesRegistrados++;
    return true;
}

bool actualizarPaciente(Paciente modificado) {
    int indice = buscarIndiceDeID(PACIENTES_FILE, modificado.id, sizeof(Paciente));
    if (indice == -1) return false;
    modificado.fechaModificacion = time(NULL);
    return actualizarRegistroPorIndice(PACIENTES_FILE, indice, sizeof(Paciente), &modificado);
}

bool eliminarPaciente(int id, Hospital* hospital) {
    int indice = buscarIndiceDeID(PACIENTES_FILE, id, sizeof(Paciente));
    if (indice == -1) return false;

    Paciente p;
    leerRegistroPorIndice(PACIENTES_FILE, indice, sizeof(Paciente), &p);
    p.eliminado = true;
    p.activo = false;
    p.fechaModificacion = time(NULL);
    
    if (actualizarRegistroPorIndice(PACIENTES_FILE, indice, sizeof(Paciente), &p)) {
        ArchivoHeader header = leerHeader(PACIENTES_FILE);
        header.registrosActivos--;
        actualizarHeader(PACIENTES_FILE, header);
        hospital->totalPacientesRegistrados--;
        return true;
    }
    return false;
}

void mostrarFichaPaciente(Paciente p) {
    cout << "\n--- FICHA PACIENTE ID: " << p.id << " ---" << endl;
    cout << "Nombre: " << p.nombre << " " << p.apellido << endl;
    cout << "Cedula: " << p.cedula << endl;
    cout << "Edad: " << p.edad << " | Sexo: " << p.sexo << " | Sangre: " << p.tipoSangre << endl;
    cout << "Telefono: " << p.telefono << endl;
    cout << "Email: " << p.email << endl;
    cout << "Direccion: " << p.direccion << endl;
    cout << "Alergias: " << p.alergias << endl;
    cout << "Observaciones: " << p.observaciones << endl;
    cout << "Estado: " << (p.activo ? "Activo" : "Inactivo") << endl;
}

void menuPacientes(Hospital* hospital) {
    int opcion;
    do {
        cout << "\n--- GESTIoN DE PACIENTES ---" << endl;
        cout << "1. Agregar Paciente" << endl;
        cout << "2. Buscar por Cedula" << endl;
        cout << "3. Buscar por ID" << endl;
        cout << "4. Actualizar Paciente" << endl;
        cout << "5. Eliminar Paciente" << endl;
        cout << "6. Listar Todos" << endl;
        cout << "0. Volver" << endl;
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();

        switch (opcion) {
            case 1: {
                Paciente p;
                cout << "Nombre: "; cin.getline(p.nombre, 50);
                cout << "Apellido: "; cin.getline(p.apellido, 50);
                cout << "Cedula: "; cin.getline(p.cedula, 20);
                cout << "Edad: "; cin >> p.edad; cin.ignore();
                cout << "Sexo (M/F): "; cin >> p.sexo; cin.ignore();
                cout << "Tipo Sangre: "; cin.getline(p.tipoSangre, 5);
                cout << "Telefono: "; cin.getline(p.telefono, 15);
                cout << "Direccion: "; cin.getline(p.direccion, 100);
                cout << "Email: "; cin.getline(p.email, 50);
                cout << "Alergias: "; cin.getline(p.alergias, 500);
                cout << "Observaciones: "; cin.getline(p.observaciones, 500);
                
                if(agregarPaciente(hospital, p)) cout << "Paciente registrado exitosamente.\n";
                else cout << "Error al registrar.\n";
                break;
            }
            case 2: { 
                char ced[20];
                cout << "Ingrese Cedula: "; cin.getline(ced, 20);
                int idx = buscarIndicePorCedulaPaciente(ced);
                if (idx != -1) {
                    Paciente p;
                    leerRegistroPorIndice(PACIENTES_FILE, idx, sizeof(Paciente), &p);
                    mostrarFichaPaciente(p);
                } else {
                    cout << "Paciente no encontrado.\n";
                }
                break;
            }
            case 3: { 
                int id; cout << "Ingrese ID: "; cin >> id;
                int idx = buscarIndiceDeID(PACIENTES_FILE, id, sizeof(Paciente));
                if (idx != -1) {
                    Paciente p;
                    leerRegistroPorIndice(PACIENTES_FILE, idx, sizeof(Paciente), &p);
                    if(!p.eliminado) mostrarFichaPaciente(p);
                    else cout << "El registro fue eliminado.\n";
                } else cout << "ID no existe.\n";
                break;
            }
            case 4: { 
                char ced[20];
                cout << "Ingrese Cedula del Paciente a modificar: "; cin.getline(ced, 20);
                int idx = buscarIndicePorCedulaPaciente(ced);
                if (idx != -1) {
                    Paciente p;
                    leerRegistroPorIndice(PACIENTES_FILE, idx, sizeof(Paciente), &p);
                    mostrarFichaPaciente(p);
                    cout << "\n--- Ingrese nuevos datos (o presione ENTER para mantener) ---\n";
                    // Simplificacion: solo actualizamos algunos campos claves para el ejemplo
                    cout << "Nuevo telefono: "; 
                    char buff[100]; cin.getline(buff, 100);
                    if(strlen(buff) > 0) strcpy(p.telefono, buff);
                    
                    cout << "Nueva direccion: "; cin.getline(buff, 100);
                    if(strlen(buff) > 0) strcpy(p.direccion, buff);
                    
                    cout << "Nuevas Observaciones: "; cin.getline(buff, 500);
                    if(strlen(buff) > 0) strcpy(p.observaciones, buff);
                    
                    if(actualizarPaciente(p)) cout << "Actualizacion correcta.\n";
                } else cout << "Paciente no encontrado.\n";
                break;
            }
            case 5: { // Eliminar
                int id; cout << "Ingrese ID a eliminar: "; cin >> id;
                if (eliminarPaciente(id, hospital)) cout << "Paciente eliminado.\n";
                else cout << "Error o no encontrado.\n";
                break;
            }
            case 6: {
                ifstream archivo(PACIENTES_FILE, ios::binary);
                ArchivoHeader h = leerHeader(PACIENTES_FILE);
                Paciente p;
                cout << left << setw(5) << "ID" << setw(15) << "Cedula" << setw(20) << "Nombre" << setw(20) << "Apellido" << endl;
                archivo.seekg(sizeof(ArchivoHeader));
                for(int i=0; i<h.cantidadRegistros; i++) {
                    archivo.read((char*)&p, sizeof(Paciente));
                    if(!p.eliminado) {
                        cout << left << setw(5) << p.id << setw(15) << p.cedula << setw(20) << p.nombre << setw(20) << p.apellido << endl;
                    }
                }
                archivo.close();
                break;
            }
        }
    } while (opcion != 0);
}

// =========================================================
// MoDULO DOCTORES
// =========================================================

int buscarIndicePorCedulaDoctor(const char* cedulaBuscada) {
    ifstream archivo(DOCTORES_FILE, ios::binary);
    if (!archivo.is_open()) return -1;
    ArchivoHeader header;
    archivo.read((char*)&header, sizeof(ArchivoHeader));

    Doctor temp;
    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.read((char*)&temp, sizeof(Doctor));
        if (!temp.eliminado && strcmp(temp.cedulaProfesional, cedulaBuscada) == 0) {
            archivo.close();
            return i;
        }
    }
    archivo.close();
    return -1;
}

bool agregarDoctor(Hospital* hospital, Doctor nuevo) {
    ArchivoHeader header = leerHeader(DOCTORES_FILE);
    
    nuevo.id = header.proximoID;
    nuevo.eliminado = false;
    nuevo.disponible = true;
    nuevo.fechaCreacion = time(NULL);
    nuevo.fechaModificacion = time(NULL);
    
    nuevo.cantidadPacientes = 0;
    for(int i=0; i<50; i++) nuevo.pacientesIDs[i] = -1;
    nuevo.cantidadCitas = 0;
    for(int i=0; i<30; i++) nuevo.citasIDs[i] = -1;

    ofstream archivo(DOCTORES_FILE, ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    archivo.write((char*)&nuevo, sizeof(Doctor));
    archivo.close();

    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeader(DOCTORES_FILE, header);

    hospital->siguienteIDDoctor = header.proximoID;
    hospital->totalDoctoresRegistrados++;
    return true;
}

bool eliminarDoctor(int id, Hospital* hospital) {
    int indice = buscarIndiceDeID(DOCTORES_FILE, id, sizeof(Doctor));
    if (indice == -1) return false;

    Doctor d;
    leerRegistroPorIndice(DOCTORES_FILE, indice, sizeof(Doctor), &d);
    d.eliminado = true;
    d.fechaModificacion = time(NULL);

    if (actualizarRegistroPorIndice(DOCTORES_FILE, indice, sizeof(Doctor), &d)) {
        ArchivoHeader header = leerHeader(DOCTORES_FILE);
        header.registrosActivos--;
        actualizarHeader(DOCTORES_FILE, header);
        hospital->totalDoctoresRegistrados--;
        return true;
    }
    return false;
}

void mostrarFichaDoctor(Doctor d) {
    cout << "\n--- FICHA DOCTOR ID: " << d.id << " ---" << endl;
    cout << "Dr/a. " << d.nombre << " " << d.apellido << endl;
    cout << "Cedula Prof: " << d.cedulaProfesional << endl;
    cout << "Especialidad: " << d.especialidad << endl;
    cout << "Experiencia: " << d.aniosExperiencia << " años" << endl;
    cout << "Costo Consulta: " << d.costoConsulta << endl;
    cout << "Horario: " << d.horarioAtencion << endl;
    cout << "Contacto: " << d.telefono << " | " << d.email << endl;
}

void menuDoctores(Hospital* hospital) {
    int opcion;
    do {
        cout << "\n--- GESTIoN DE DOCTORES ---" << endl;
        cout << "1. Agregar Doctor" << endl;
        cout << "2. Buscar por Cedula" << endl;
        cout << "3. Eliminar Doctor" << endl;
        cout << "4. Listar Doctores" << endl;
        cout << "0. Volver" << endl;
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();

        switch (opcion) {
            case 1: {
                Doctor d;
                cout << "Nombre: "; cin.getline(d.nombre, 50);
                cout << "Apellido: "; cin.getline(d.apellido, 50);
                cout << "Cedula Profesional: "; cin.getline(d.cedulaProfesional, 20);
                cout << "Especialidad: "; cin.getline(d.especialidad, 50);
                cout << "Años Experiencia: "; cin >> d.aniosExperiencia;
                cout << "Costo Consulta: "; cin >> d.costoConsulta; cin.ignore();
                cout << "Horario: "; cin.getline(d.horarioAtencion, 50);
                cout << "Telefono: "; cin.getline(d.telefono, 15);
                cout << "Email: "; cin.getline(d.email, 50);
                
                if(agregarDoctor(hospital, d)) cout << "Doctor registrado.\n";
                else cout << "Error.\n";
                break;
            }
            case 2: {
                char ced[20];
                cout << "Ingrese Cedula Profesional: "; cin.getline(ced, 20);
                int idx = buscarIndicePorCedulaDoctor(ced);
                if(idx != -1) {
                    Doctor d;
                    leerRegistroPorIndice(DOCTORES_FILE, idx, sizeof(Doctor), &d);
                    mostrarFichaDoctor(d);
                } else cout << "No encontrado.\n";
                break;
            }
            case 3: {
                int id; cout << "ID a eliminar: "; cin >> id;
                if(eliminarDoctor(id, hospital)) cout << "Eliminado.\n";
                else cout << "Error.\n";
                break;
            }
            case 4: {
                ifstream archivo(DOCTORES_FILE, ios::binary);
                ArchivoHeader h = leerHeader(DOCTORES_FILE);
                Doctor d;
                cout << left << setw(5) << "ID" << setw(20) << "Nombre" << setw(20) << "Especialidad" << endl;
                archivo.seekg(sizeof(ArchivoHeader));
                for(int i=0; i<h.cantidadRegistros; i++) {
                    archivo.read((char*)&d, sizeof(Doctor));
                    if(!d.eliminado) {
                        cout << left << setw(5) << d.id << setw(20) << d.nombre << setw(20) << d.especialidad << endl;
                    }
                }
                archivo.close();
                break;
            }
        }
    } while (opcion != 0);
}

// =========================================================
// MoDULO CITAS
// =========================================================

bool agregarCita(Hospital* hospital, Cita nueva) {
    ArchivoHeader header = leerHeader(CITAS_FILE);
    
    nueva.id = header.proximoID;
    nueva.eliminado = false;
    nueva.atendida = false;
    nueva.consultaID = -1;
    strcpy(nueva.estado, "Agendada");
    nueva.fechaCreacion = time(NULL);
    nueva.fechaModificacion = time(NULL);

    ofstream archivo(CITAS_FILE, ios::binary | ios::app);
    if (!archivo.is_open()) return false;
    archivo.write((char*)&nueva, sizeof(Cita));
    archivo.close();

    header.cantidadRegistros++;
    header.proximoID++;
    header.registrosActivos++;
    actualizarHeader(CITAS_FILE, header);

    hospital->siguienteIDCita = header.proximoID;
    hospital->totalCitasAgendadas++;
    return true;
}

void menuCitas(Hospital* hospital) {
    int opcion;
    do {
        cout << "\n--- GESTIoN DE CITAS ---" << endl;
        cout << "1. Agendar Cita" << endl;
        cout << "2. Ver Citas" << endl;
        cout << "0. Volver" << endl;
        cout << "Opcion: ";
        cin >> opcion;
        cin.ignore();

        switch(opcion) {
            case 1: {
                Cita c;
                cout << "ID Paciente: "; cin >> c.pacienteID;
                cout << "ID Doctor: "; cin >> c.doctorID; cin.ignore();
                cout << "Fecha (YYYY-MM-DD): "; cin.getline(c.fecha, 11);
                cout << "Hora (HH:MM): "; cin.getline(c.hora, 6);
                cout << "Motivo: "; cin.getline(c.motivo, 150);
                cout << "Observaciones: "; cin.getline(c.observaciones, 200);
                
                if(agregarCita(hospital, c)) cout << "Cita agendada. ID: " << hospital->siguienteIDCita -1 << endl;
                else cout << "Error.\n";
                break;
            }
            case 2: {
                ifstream archivo(CITAS_FILE, ios::binary);
                ArchivoHeader h = leerHeader(CITAS_FILE);
                Cita c;
                cout << left << setw(5) << "ID" << setw(12) << "Fecha" << setw(15) << "Estado" << setw(20) << "Motivo" << endl;
                archivo.seekg(sizeof(ArchivoHeader));
                for(int i=0; i<h.cantidadRegistros; i++) {
                    archivo.read((char*)&c, sizeof(Cita));
                    if(!c.eliminado) {
                        cout << left << setw(5) << c.id << setw(12) << c.fecha << setw(15) << c.estado << setw(20) << c.motivo << endl;
                    }
                }
                archivo.close();
                break;
            }
        }
    } while (opcion != 0);
}

// =========================================================
// MANTENIMIENTO: COMPACTACIoN
// =========================================================

template <typename T>
bool compactarArchivoGenerico(const char* nombreOriginal, const char* nombreTemp) {
    ifstream original(nombreOriginal, ios::binary);
    if (!original.is_open()) return false;

    ofstream temp(nombreTemp, ios::binary | ios::out);
    if (!temp.is_open()) return false;

    ArchivoHeader headerOriginal;
    original.read((char*)&headerOriginal, sizeof(ArchivoHeader));

    ArchivoHeader headerNuevo = headerOriginal;
    headerNuevo.cantidadRegistros = 0;
    headerNuevo.registrosActivos = 0;
    headerNuevo.proximoID = 1; 

    temp.write((char*)&headerNuevo, sizeof(ArchivoHeader));

    T registro;
    while (original.read((char*)&registro, sizeof(T))) {
        if (!registro.eliminado) {
            registro.id = headerNuevo.proximoID; 
            temp.write((char*)&registro, sizeof(T));
            
            headerNuevo.cantidadRegistros++;
            headerNuevo.registrosActivos++;
            headerNuevo.proximoID++;
        }
    }

    original.close();
    temp.close();

    actualizarHeader(nombreTemp, headerNuevo);
    if (remove(nombreOriginal) != 0) return false;
    if (rename(nombreTemp, nombreOriginal) != 0) return false;
    
    cout << "Compactado: " << nombreOriginal << ". Registros activos: " << headerNuevo.registrosActivos << endl;
    return true;
}

void menuMantenimiento() {
    cout << "\n--- MANTENIMIENTO ---" << endl;
    cout << "1. Compactar Pacientes\n2. Compactar Doctores\n3. Compactar Citas\n0. Salir\nOpcion: ";
    int op; cin >> op;
    switch(op) {
        case 1: compactarArchivoGenerico<Paciente>(PACIENTES_FILE, "pac_tmp.bin"); break;
        case 2: compactarArchivoGenerico<Doctor>(DOCTORES_FILE, "doc_tmp.bin"); break;
        case 3: compactarArchivoGenerico<Cita>(CITAS_FILE, "cit_tmp.bin"); break;
    }
}

// =========================================================
// MAIN
// =========================================================

int main() {
    Hospital* hospital = cargarDatosHospital();
    if (!hospital) {
        cerr << "Error critico iniciando sistema." << endl;
        return 1;
    }
    cout << "\n=================================================" << endl;
    cout << " Sistema de Gestion Hospitalaria Iniciado "  << endl;
    cout << "=================================================" << endl;

    int opcion;
    do {
        cout << "\n========================================" << endl;
    cout << "            MENU PRINCIPAL" << endl;
    cout << "========================================" << endl;
    cout << "1. Gestion de Pacientes" << endl;
    cout << "2. Gestion de Doctores" << endl;
    cout << "3. Gestion de Citas" << endl;
    cout << "4. Consultas y Reportes" << endl;
    cout << "5. Mantenimiento de Archivos" << endl;
    cout << "6. Guardar y Salir" << endl;
    cout << "Opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1: menuPacientes(hospital); break;
            case 2: menuDoctores(hospital); break;
            case 3: menuCitas(hospital); break;
            case 4: menuMantenimiento(); break;
            case 5: guardarDatosHospital(hospital); break;
            case 6: return 0;
        }
    } while (opcion != 5);

    delete hospital;
    return 0;
}