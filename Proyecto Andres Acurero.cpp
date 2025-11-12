#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <cctype>
#include <cstdlib> // Para std::abs
#include <limits>

using namespace std;


// Constantes
const char* ARCHIVO_HOSPITAL = "hospital.bin";
const char* ARCHIVO_PACIENTES = "pacientes.bin";
const char* ARCHIVO_DOCTORES = "doctores.bin";
const char* ARCHIVO_CITAS = "citas.bin";

// =================================================================
// 1. MODELO DE DATOS - ESTRUCTURAS
// =================================================================
const int MAX_HISTORIAL_MEDICO = 100;
const int MAX_CITAS_PACIENTE = 50;
const int MAX_PACIENTES_DOCTOR = 100;
const int MAX_CITAS_DOCTOR = 100;
const int MAX_CITAS_HOSPITAL = 500;

struct HistorialMedico {
    int idConsulta;
    char fecha[11];      // YYYY-MM-DD\0
    char hora[6];        // HH:MM\0
    char diagnostico[201];
    char tratamiento[201];
    char medicamentos[151];
    int idDoctor;
    float costoConsulta;
    bool activo; 
};


struct Paciente {
    // Datos originales (sin cambios)
    int idPaciente;
    char nombre[51];
    char apellido[51];
    char cedula[11]; // Cédula como string
    int edad;
    char sexo;       // 'M', 'F', 'O'
    // --- NUEVOS CAMPOS REQUERIDOS ---
    
    HistorialMedico historial[MAX_HISTORIAL_MEDICO];
    int numHistorial; // Contador de cuántas entradas tiene
    
    // Citas: Ahora es un array fijo
    int idCitas[MAX_CITAS_PACIENTE];
    int numCitas; // Contador de cuántas citas tiene
    
    bool activo; // Para eliminación lógica
    // Relleno para asegurar el tamaño fijo del struct
    char _padding[3]; 
};


struct Doctor {
    // Identificación y datos personales
     int idDoctor;
    char nombre[51];
    char apellido[51];
    char cedulaProfesional[11];
    char especialidad[51];
    int experienciaAnios;
    float costoConsulta;

    // Pacientes Asignados: Array de IDs de pacientes
    int idPacientesAsignados[MAX_PACIENTES_DOCTOR];
    int numPacientesAsignados;

    // Citas: Array de IDs de citas
    int idCitas[MAX_CITAS_DOCTOR];
    int numCitas;

    bool activo; // Para eliminación lógica
    // Relleno para asegurar el tamaño fijo del struct
    char _padding[3]; 
};


struct Cita {
     int idCita;
    int idPaciente;
    int idDoctor;
    char fecha[11];      // YYYY-MM-DD\0
    char hora[6];        // HH:MM\0
    char motivo[101];
    // Estado: 0: Agendada, 1: Atendida, 2: Cancelada
    int estado; 
    
    bool activo; // Para eliminación lógica
    // Relleno para asegurar el tamaño fijo del struct
    char _padding[3]; 
};


struct Hospital {
   char nombre[101];
    char direccion[101];
    int ultimoIdPaciente;
    int ultimoIdDoctor;
    int ultimoIdCita;
    // Contadores reales de registros activos (para reportes/mantenimiento)
    int totalPacientes;
    int totalDoctores;
    int totalCitas;
};

// void buscarPacientePorId(Hospital* hospital, int id);
// =================================================================
// 5. FUNCIONES DE UTILIDADES
// =================================================================

// 5.3 Funciones de copia (Deep Copy)


long calcularPosicion(int index, size_t structSize) {
    // Los IDs son base 1, el índice del array es base 0, por eso index - 1.
    return (index - 1) * structSize;
}

void guardarDatosHospital(Hospital* h) {
    // Usamos trunc para sobrescribir completamente (solo es 1 registro)
    fstream archivo(ARCHIVO_HOSPITAL, ios::binary | ios::out | ios::trunc);
    if (archivo.is_open()) {
        archivo.write(reinterpret_cast<const char*>(h), sizeof(Hospital));
        archivo.close();
    } else {
        cerr << "ERROR: No se pudo abrir/crear el archivo de Hospital para guardar." << endl;
    }
}

Hospital* cargarDatosHospital() {
    Hospital* h = new Hospital();
    fstream archivo(ARCHIVO_HOSPITAL, ios::binary | ios::in | ios::out);

    if (archivo.is_open()) {
        // Archivo existe, cargamos los datos
        archivo.read(reinterpret_cast<char*>(h), sizeof(Hospital));
        archivo.close();
    } else {
        // Archivo no existe o no se puede abrir, inicializamos
        cout << "Inicializando nuevo archivo de Hospital..." << endl;
        strncpy(h->nombre, "Hospital Central (v2 - Binario)", 100);
        strncpy(h->direccion, "Av. Principal, Ciudad", 100);
        h->ultimoIdPaciente = 0;
        h->ultimoIdDoctor = 0;
        h->ultimoIdCita = 0;
        h->totalPacientes = 0;
        h->totalDoctores = 0;
        h->totalCitas = 0;
        
        // Guardamos la inicialización
        guardarDatosHospital(h);
    }
    return h;
}

bool verificarPacienteActivo(int id) {
    if (id <= 0) return false;
    Paciente p;
    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archivo.is_open()) return false;
    
    long pos = calcularPosicion(id, sizeof(Paciente));
    
    // Verificamos que la posición no exceda el tamaño del archivo
    archivo.seekg(0, ios::end);
    long tamArchivo = archivo.tellg();
    if (pos >= tamArchivo) {
        archivo.close();
        return false;
    }
    archivo.seekg(pos);
    archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente));
    archivo.close();

    return p.activo && p.idPaciente == id;
}

bool verificarDoctorActivo(int id) {
    if (id <= 0) return false;
    Doctor d;
    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archivo.is_open()) return false;
    
    long pos = calcularPosicion(id, sizeof(Doctor));

    archivo.seekg(0, ios::end);
    long tamArchivo = archivo.tellg();
    if (pos >= tamArchivo) {
        archivo.close();
        return false;
    }

    archivo.seekg(pos);
    archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor));
    archivo.close();

    return d.activo && d.idDoctor == id;
}

bool leerCitaPorId(int id, Cita& c) {
    if (id <= 0) return false;
    fstream archivo(ARCHIVO_CITAS, ios::binary | ios::in);
    if (!archivo.is_open()) return false;

    long pos = calcularPosicion(id, sizeof(Cita));

    archivo.seekg(0, ios::end);
    long tamArchivo = archivo.tellg();
    if (pos >= tamArchivo) {
        archivo.close();
        return false;
    }

    archivo.seekg(pos);
    if (archivo.read(reinterpret_cast<char*>(&c), sizeof(Cita))) {
        archivo.close();
        return c.activo;
    }
    archivo.close();
    return false;
}


// =================================================================
// 3. RUTINAS DE MANTENIMIENTO DE ARCHIVOS
// =================================================================


bool verificarCitaActiva(int id) {
    if (id <= 0) return false;
    Cita c;
    return leerCitaPorId(id, c) && c.activo;
}


void printIntegrityReport(const char* level, const char* entity, int id, const char* message) {
    cout << "[" << level << "][" << entity << " ID " << id << "]: " << message << endl;
}


void verificarIntegridadArchivos(Hospital* h) {
    cout << "\n========================================" << endl;
    cout << "  VERIFICACION DE INTEGRIDAD DE ARCHIVOS" << endl;
    cout << "========================================" << endl;
    
    // 1. Verificar Archivo de Pacientes
    cout << "\n--- Verificando Pacientes (" << ARCHIVO_PACIENTES << ") ---" << endl;
    fstream archPacientes(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archPacientes.is_open()) {
        cerr << "ADVERTENCIA: Archivo de Pacientes no existe o no se pudo abrir." << endl;
    } else {
        Paciente p;
        int registrosEncontrados = 0;
        for (int i = 1; i <= h->ultimoIdPaciente; ++i) {
            long pos = calcularPosicion(i, sizeof(Paciente));
            archPacientes.seekg(pos);
            if (archPacientes.read(reinterpret_cast<char*>(&p), sizeof(Paciente))) {
                if (p.idPaciente != i) {
                    printIntegrityReport("ERROR", "Paciente", i, "ID no coincide con posicion.");
                }
                if (p.activo) {
                    registrosEncontrados++;
                    // 1.1. Verificar referencias a Citas (en archivo de Citas)
                    for (int j = 0; j < p.numCitas; ++j) {
                        if (!verificarCitaActiva(p.idCitas[j])) {
                     //       printIntegrityReport("WARN", "Paciente", p.idPaciente, 
                    //            "Cita referenciada (" + to_string(p.idCitas[j]) + ") no existe o está inactiva.");
                    cout << "VERIFICAR INTEGRIDAD ARCHIVO CITAS";
                        }
                    }
                    // 1.2. No podemos verificar Historial, solo su doctor:
                    for (int j = 0; j < p.numHistorial; ++j) {
                        if (p.historial[j].activo && !verificarDoctorActivo(p.historial[j].idDoctor)) {
                        //    printIntegrityReport("WARN", "Paciente", p.idPaciente, 
                          //      "Doctor de Historial (" + to_string(p.historial[j].idDoctor) + ") no existe o está inactivo.");
                          cout << "VERIFICAR INTEGRIDAD ARCHIVO DOCTOR";
                        }
                    }
                }
            } else {
                printIntegrityReport("ERROR", "Paciente", i, "Fallo al leer registro en posicion esperada.");
            }
        }
        archPacientes.close();
        if (registrosEncontrados != h->totalPacientes) {
            // printIntegrityReport("ERROR", "Hospital", 0, 
              //  "Contador 'totalPacientes' (" + to_string(h->totalPacientes) + ") no coincide con registros activos (" + to_string(registrosEncontrados) + ").");
              cout << "NO COINCIDE EL TOTAL DE PACIENTES CON LOS REGISTROS";
        }
    }

    // 2. Verificar Archivo de Doctores
    cout << "\n--- Verificando Doctores (" << ARCHIVO_DOCTORES << ") ---" << endl;
    fstream archDoctores(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archDoctores.is_open()) {
        cerr << "ADVERTENCIA: Archivo de Doctores no existe o no se pudo abrir." << endl;
    } else {
        Doctor d;
        int registrosEncontrados = 0;
        for (int i = 1; i <= h->ultimoIdDoctor; ++i) {
            long pos = calcularPosicion(i, sizeof(Doctor));
            archDoctores.seekg(pos);
            if (archDoctores.read(reinterpret_cast<char*>(&d), sizeof(Doctor))) {
                if (d.idDoctor != i) {
                    printIntegrityReport("ERROR", "Doctor", i, "ID no coincide con posicion.");
                }
                if (d.activo) {
                    registrosEncontrados++;
                    // 2.1. Verificar referencias a Pacientes Asignados
                    for (int j = 0; j < d.numPacientesAsignados; ++j) {
                        if (!verificarPacienteActivo(d.idPacientesAsignados[j])) {
                            //printIntegrityReport("WARN", "Doctor", d.idDoctor, 
                              //  "Paciente asignado (" + to_string(d.idPacientesAsignados[j]) + ") no existe o está inactivo.");
                              cout << "VERIFICAR PACIENTE ACTIVO";
                        }
                    }
                     // 2.2. Verificar referencias a Citas (en archivo de Citas)
                    for (int j = 0; j < d.numCitas; ++j) {
                        if (!verificarCitaActiva(d.idCitas[j])) {
                          //  printIntegrityReport("WARN", "Doctor", d.idCitas[j], 
                            //    "Cita referenciada (" + to_string(d.idCitas[j]) + ") no existe o está inactiva.");
                            cout << "VERIFICAR CITA ACTIVA";
                        }
                    }
                }
            } else {
                printIntegrityReport("ERROR", "Doctor", i, "Fallo al leer registro en posicion esperada.");
            }
        }
        archDoctores.close();
        if (registrosEncontrados != h->totalDoctores) {
            // printIntegrityReport("ERROR", "Hospital", 0, 
              //  "Contador 'totalDoctores' (" + to_string(h->totalDoctores) + ") no coincide con registros activos (" + to_string(registrosEncontrados) + ").");
              cout << "TOTAL DOCTORES NO COINCIDE CON LOS REGISTROS";
        }
    }
    
    // 3. Verificar Archivo de Citas
    cout << "\n--- Verificando Citas (" << ARCHIVO_CITAS << ") ---" << endl;
    fstream archCitas(ARCHIVO_CITAS, ios::binary | ios::in);
    if (!archCitas.is_open()) {
        cerr << "ADVERTENCIA: Archivo de Citas no existe o no se pudo abrir." << endl;
    } else {
        Cita c;
        int registrosEncontrados = 0;
        for (int i = 1; i <= h->ultimoIdCita; ++i) {
            long pos = calcularPosicion(i, sizeof(Cita));
            archCitas.seekg(pos);
            if (archCitas.read(reinterpret_cast<char*>(&c), sizeof(Cita))) {
                if (c.idCita != i) {
                    printIntegrityReport("ERROR", "Cita", i, "ID no coincide con posicion.");
                }
                if (c.activo) {
                    registrosEncontrados++;
                    // 3.1. Verificar referencias a Paciente
                    if (!verificarPacienteActivo(c.idPaciente)) {
                     //   printIntegrityReport("ERROR", "Cita", c.idCita, 
                       //     "Paciente referenciado (" + to_string(c.idPaciente) + ") no existe o está inactivo.");
                       cout << "VERIFICAR PACIENTE ACTIVO";
                    }
                    // 3.2. Verificar referencias a Doctor
                    if (!verificarDoctorActivo(c.idDoctor)) {
                       // printIntegrityReport("ERROR", "Cita", c.idCita, 
                         //   "Doctor referenciado (" + to_string(c.idDoctor) + ") no existe o está inactivo.");
                         cout << "VERIFICAR DOCTOR ACTIVO";
                    }
                }
            } else {
                printIntegrityReport("ERROR", "Cita", i, "Fallo al leer registro en posicion esperada.");
            }
        }
        archCitas.close();
        if (registrosEncontrados != h->totalCitas) {
            // printIntegrityReport("ERROR", "Hospital", 0, 
               // "Contador 'totalCitas' (" + to_string(h->totalCitas) + ") no coincide con registros activos (" + to_string(registrosEncontrados) + ").");
               cout << "TOTAL CITAS NO COINCIDE CON LOS REGISTROS";
        }
    }

    cout << "\nVerificacion de integridad completada. Revise los mensajes de error/advertencia." << endl;
}

void actualizarReferencias(const char* entidadCompactada, int idMapping[], int ultimoIdPaciente, int ultimoIdDoctor, int ultimoIdCita) {
    // 1. Si se compactó Pacientes, actualizar referencias en Doctores y Citas
    if (strcmp(entidadCompactada, "Paciente") == 0) {
        // Actualizar Doctores
        fstream archDoctores(ARCHIVO_DOCTORES, ios::binary | ios::in | ios::out);
        Doctor d;
        for (int i = 1; i <= ultimoIdDoctor; ++i) {
            long pos = calcularPosicion(i, sizeof(Doctor));
            archDoctores.seekg(pos);
            if (archDoctores.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
                bool modificado = false;
                // Actualizar idPacientesAsignados
                for (int j = 0; j < d.numPacientesAsignados; ++j) {
                    int oldId = d.idPacientesAsignados[j];
                    if (oldId > 0 && idMapping[oldId] != 0) { // Si el paciente sigue activo
                        if (idMapping[oldId] != oldId) {
                            d.idPacientesAsignados[j] = idMapping[oldId];
                            modificado = true;
                        }
                    } else if (oldId > 0 && idMapping[oldId] == 0) { // Si el paciente fue eliminado
                        // Eliminación física (moviendo elementos)
                        for (int k = j; k < d.numPacientesAsignados - 1; ++k) {
                            d.idPacientesAsignados[k] = d.idPacientesAsignados[k + 1];
                        }
                        d.numPacientesAsignados--;
                        j--; // Volver a revisar el índice actual
                        modificado = true;
                    }
                }

                if (modificado) {
                    archDoctores.seekp(pos);
                    archDoctores.write(reinterpret_cast<const char*>(&d), sizeof(Doctor));
                }
            }
        }
        archDoctores.close();

        // Actualizar Citas
        fstream archCitas(ARCHIVO_CITAS, ios::binary | ios::in | ios::out);
        Cita c;
        for (int i = 1; i <= ultimoIdCita; ++i) {
            long pos = calcularPosicion(i, sizeof(Cita));
            archCitas.seekg(pos);
            if (archCitas.read(reinterpret_cast<char*>(&c), sizeof(Cita)) && c.activo) {
                int oldId = c.idPaciente;
                if (oldId > 0 && idMapping[oldId] != 0) {
                    if (idMapping[oldId] != oldId) {
                        c.idPaciente = idMapping[oldId];
                        archCitas.seekp(pos);
                        archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                    }
                }
                // Si la cita referencia a un paciente eliminado, la cita debe ser cancelada/eliminada
                else if (oldId > 0 && idMapping[oldId] == 0) {
                    c.activo = false; // Eliminación lógica de la cita
                    archCitas.seekp(pos);
                    archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                    // Nota: Se debe actualizar el contador de citas en Hospital, pero esto se hará
                    // en la compactación de Citas.
                }
            }
        }
        archCitas.close();
    }
    
    // 2. Si se compactó Doctores, actualizar referencias en Pacientes y Citas
    else if (strcmp(entidadCompactada, "Doctor") == 0) {
        // Actualizar Pacientes (Historial)
        fstream archPacientes(ARCHIVO_PACIENTES, ios::binary | ios::in | ios::out);
        Paciente p;
        for (int i = 1; i <= ultimoIdPaciente; ++i) {
            long pos = calcularPosicion(i, sizeof(Paciente));
            archPacientes.seekg(pos);
            if (archPacientes.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
                bool modificado = false;
                for (int j = 0; j < p.numHistorial; ++j) {
                    int oldId = p.historial[j].idDoctor;
                    if (oldId > 0 && idMapping[oldId] != 0) {
                        if (idMapping[oldId] != oldId) {
                            p.historial[j].idDoctor = idMapping[oldId];
                            modificado = true;
                        }
                    } else if (oldId > 0 && idMapping[oldId] == 0) {
                        // Si el doctor fue eliminado, se marca la entrada de historial como inactiva
                        p.historial[j].activo = false;
                        modificado = true;
                    }
                }
                
                if (modificado) {
                    archPacientes.seekp(pos);
                    archPacientes.write(reinterpret_cast<const char*>(&p), sizeof(Paciente));
                }
            }
        }
        archPacientes.close();

        // Actualizar Citas
        fstream archCitas(ARCHIVO_CITAS, ios::binary | ios::in | ios::out);
        Cita c;
        for (int i = 1; i <= ultimoIdCita; ++i) {
            long pos = calcularPosicion(i, sizeof(Cita));
            archCitas.seekg(pos);
            if (archCitas.read(reinterpret_cast<char*>(&c), sizeof(Cita)) && c.activo) {
                int oldId = c.idDoctor;
                if (oldId > 0 && idMapping[oldId] != 0) {
                    if (idMapping[oldId] != oldId) {
                        c.idDoctor = idMapping[oldId];
                        archCitas.seekp(pos);
                        archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                    }
                }
                // Si la cita referencia a un doctor eliminado, la cita debe ser cancelada/eliminada
                else if (oldId > 0 && idMapping[oldId] == 0) {
                    c.activo = false; // Eliminación lógica de la cita
                    archCitas.seekp(pos);
                    archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                }
            }
        }
        archCitas.close();
    }
    
    // 3. Si se compactó Citas, actualizar referencias en Pacientes y Doctores
    else if (strcmp(entidadCompactada, "Cita") == 0) {
        // Actualizar Pacientes
        fstream archPacientes(ARCHIVO_PACIENTES, ios::binary | ios::in | ios::out);
        Paciente p;
        for (int i = 1; i <= ultimoIdPaciente; ++i) {
            long pos = calcularPosicion(i, sizeof(Paciente));
            archPacientes.seekg(pos);
            if (archPacientes.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
                bool modificado = false;
                // Actualizar idCitas
                for (int j = 0; j < p.numCitas; ++j) {
                    int oldId = p.idCitas[j];
                    if (oldId > 0 && idMapping[oldId] != 0) {
                        if (idMapping[oldId] != oldId) {
                            p.idCitas[j] = idMapping[oldId];
                            modificado = true;
                        }
                    } else if (oldId > 0 && idMapping[oldId] == 0) {
                        // Eliminación física
                        for (int k = j; k < p.numCitas - 1; ++k) {
                            p.idCitas[k] = p.idCitas[k + 1];
                        }
                        p.numCitas--;
                        j--;
                        modificado = true;
                    }
                }
                
                if (modificado) {
                    archPacientes.seekp(pos);
                    archPacientes.write(reinterpret_cast<const char*>(&p), sizeof(Paciente));
                }
            }
        }
        archPacientes.close();

        // Actualizar Doctores
        fstream archDoctores(ARCHIVO_DOCTORES, ios::binary | ios::in | ios::out);
        Doctor d;
        for (int i = 1; i <= ultimoIdDoctor; ++i) {
            long pos = calcularPosicion(i, sizeof(Doctor));
            archDoctores.seekg(pos);
            if (archDoctores.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
                bool modificado = false;
                // Actualizar idCitas
                for (int j = 0; j < d.numCitas; ++j) {
                    int oldId = d.idCitas[j];
                    if (oldId > 0 && idMapping[oldId] != 0) {
                        if (idMapping[oldId] != oldId) {
                            d.idCitas[j] = idMapping[oldId];
                            modificado = true;
                        }
                    } else if (oldId > 0 && idMapping[oldId] == 0) {
                        // Eliminación física
                        for (int k = j; k < d.numCitas - 1; ++k) {
                            d.idCitas[k] = d.idCitas[k + 1];
                        }
                        d.numCitas--;
                        j--;
                        modificado = true;
                    }
                }

                if (modificado) {
                    archDoctores.seekp(pos);
                    archDoctores.write(reinterpret_cast<const char*>(&d), sizeof(Doctor));
                }
            }
        }
        archDoctores.close();
    }
}



template <typename T>
void compactarArchivo(Hospital* h, const char* nombreArchivo, int& ultimoId, int& totalRegistros, const char* entidad) {
    
    // 1. Crear un archivo temporal para la copia de registros activos
    string tempFileName = string(nombreArchivo) + ".tmp";
    fstream tempFile(tempFileName.c_str(), ios::binary | ios::out | ios::trunc);
    fstream originalFile(nombreArchivo, ios::binary | ios::in);

    if (!originalFile.is_open()) {
        cout << "ADVERTENCIA: Archivo de " << entidad << " no existe o esta vacio. No hay nada que compactar." << endl;
        return;
    }
    if (!tempFile.is_open()) {
        cerr << "ERROR: No se pudo crear el archivo temporal para compactacion." << endl;
        originalFile.close();
        return;
    }

    cout << "Compactando archivo de " << entidad << " (" << nombreArchivo << ")..." << endl;
    
    T record;
    int nuevoId = 1;
    int registrosCompactados = 0;
    
    // Mapeo de ID antiguo a ID nuevo
    // Usaremos un array simple, asumiendo que el ID no será exorbitantemente grande
    int idMapping[ultimoId + 1];
    for (int i = 0; i <= ultimoId; ++i) idMapping[i] = 0;

    // 2. Recorrer el archivo original, reescribiendo solo activos en el temporal
    for (int oldId = 1; oldId <= ultimoId; ++oldId) {
        long pos = calcularPosicion(oldId, sizeof(T));
        originalFile.seekg(pos);
        
        if (originalFile.read(reinterpret_cast<char*>(&record), sizeof(T))) {
            bool activo = false;
            // Detección de activo (usando un truco simple para acceder al miembro 'activo')
            if (entidad == "Paciente") {
                activo = ((Paciente*)&record)->activo;
                ((Paciente*)&record)->idPaciente = nuevoId;
            } else if (entidad == "Doctor") {
                activo = ((Doctor*)&record)->activo;
                ((Doctor*)&record)->idDoctor = nuevoId;
            } else if (entidad == "Cita") {
                activo = ((Cita*)&record)->activo;
                ((Cita*)&record)->idCita = nuevoId;
            }

            if (activo) {
                // Escribir en el archivo temporal con el nuevo ID
                tempFile.write(reinterpret_cast<const char*>(&record), sizeof(T));
                
                // Guardar mapeo
                idMapping[oldId] = nuevoId;
                nuevoId++;
                registrosCompactados++;
            } else {
                 idMapping[oldId] = 0; // Marcar como eliminado
            }
        }
    }
    
    originalFile.close();
    tempFile.close();
    
    // 3. Eliminar archivo original y renombrar el temporal
    if (remove(nombreArchivo) != 0) {
        cerr << "ERROR: No se pudo eliminar el archivo original " << nombreArchivo << endl;
        return;
    }
    if (rename(tempFileName.c_str(), nombreArchivo) != 0) {
        cerr << "ERROR: No se pudo renombrar el archivo temporal a " << nombreArchivo << endl;
        return;
    }

    // 4. Actualizar contadores del Hospital
    ultimoId = registrosCompactados;
    totalRegistros = registrosCompactados;
    cout << "Compactacion exitosa. Total de " << entidad << "s activos: " << totalRegistros << endl;
    guardarDatosHospital(h);
    
    // 5. Actualizar referencias cruzadas en otros archivos
    cout << "Actualizando referencias cruzadas..." << endl;
    actualizarReferencias(entidad, idMapping, h->ultimoIdPaciente, h->ultimoIdDoctor, h->ultimoIdCita);
}


void mantenimientoArchivos(Hospital* h) {
    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "   SUBMENU MANTENIMIENTO DE ARCHIVOS" << endl;
        cout << "========================================" << endl;
        cout << "1. Verificar integridad de archivos" << endl;
        cout << "2. Compactar archivos (Pacientes)" << endl;
        cout << "3. Compactar archivos (Doctores)" << endl;
        cout << "4. Compactar archivos (Citas)" << endl;
        cout << "0. Volver al menú principal" << endl;
        cout << "Opcion: ";

        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1:
                verificarIntegridadArchivos(h);
                break;
            case 2:
                // Compactar Pacientes
                compactarArchivo<Paciente>(h, ARCHIVO_PACIENTES, h->ultimoIdPaciente, h->totalPacientes, "Paciente");
                break;
            case 3:
                // Compactar Doctores
                compactarArchivo<Doctor>(h, ARCHIVO_DOCTORES, h->ultimoIdDoctor, h->totalDoctores, "Doctor");
                break;
            case 4:
                // Compactar Citas
                compactarArchivo<Cita>(h, ARCHIVO_CITAS, h->ultimoIdCita, h->totalCitas, "Cita");
                break;
            case 0:
                cout << "Volviendo al menu principal." << endl;
                break;
            default:
                cout << "Opcion no valida. Intente de nuevo." << endl;
                break;
        }
    } while (opcion != 0);
}

bool leerPacientePorId(int id, Paciente& p) {
    if (id <= 0) return false;
    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archivo.is_open()) return false;
    
    long pos = calcularPosicion(id, sizeof(Paciente));
    
    // Opcional: Verificar que la posición no esté fuera de límites antes de buscar
    archivo.seekg(0, ios::end);
    long tamArchivo = archivo.tellg();
    if (pos >= tamArchivo) {
        archivo.close();
        return false;
    }

    archivo.seekg(pos);
    if (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente))) {
        archivo.close();
        return p.activo;
    }
    archivo.close();
    return false;
}


bool escribirPaciente(Paciente& p, Hospital* h) {
    bool esNuevo = (p.idPaciente == 0);
    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in | ios::out | ios::ate);
    
    if (!archivo.is_open()) {
        // Si no existe, intentar crear
        archivo.open(ARCHIVO_PACIENTES, ios::binary | ios::out);
        if (!archivo.is_open()) {
             cerr << "ERROR: No se pudo abrir/crear el archivo de pacientes." << endl;
             return false;
        }
    }
 long pos;
    if (esNuevo) {
        h->ultimoIdPaciente++;
        p.idPaciente = h->ultimoIdPaciente;
        p.activo = true;
        p.numHistorial = 0;
        p.numCitas = 0;
        h->totalPacientes++;
        pos = calcularPosicion(p.idPaciente, sizeof(Paciente));
        // Moverse al final teórico (aunque ya estamos en ios::ate)
        archivo.seekp(pos);
    } else {
        pos = calcularPosicion(p.idPaciente, sizeof(Paciente));
        archivo.seekp(pos);
    }
    
    archivo.write(reinterpret_cast<const char*>(&p), sizeof(Paciente));
    archivo.close();
    
    if (esNuevo) {
        guardarDatosHospital(h); // Guardar el nuevo ID y contador
    }
    
    return true;
}
bool leerDoctorPorId(int id, Doctor& d) {
    if (id <= 0) return false;
    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archivo.is_open()) return false;

    long pos = calcularPosicion(id, sizeof(Doctor));

    archivo.seekg(0, ios::end);
    long tamArchivo = archivo.tellg();
    if (pos >= tamArchivo) {
        archivo.close();
        return false;
    }
    
    archivo.seekg(pos);
    if (archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor))) {
        archivo.close();
        return d.activo;
    }
    archivo.close();
    return false;
}

bool escribirDoctor(Doctor& d, Hospital* h) {
    bool esNuevo = (d.idDoctor == 0);
    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in | ios::out | ios::ate);
    
    if (!archivo.is_open()) {
        archivo.open(ARCHIVO_DOCTORES, ios::binary | ios::out);
        if (!archivo.is_open()) {
             cerr << "ERROR: No se pudo abrir/crear el archivo de doctores." << endl;
             return false;
        }
    }

    long pos;
    if (esNuevo) {
        h->ultimoIdDoctor++;
        d.idDoctor = h->ultimoIdDoctor;
        d.activo = true;
        d.numPacientesAsignados = 0;
        d.numCitas = 0;
        h->totalDoctores++;
        pos = calcularPosicion(d.idDoctor, sizeof(Doctor));
        archivo.seekp(pos);
    } else {
        pos = calcularPosicion(d.idDoctor, sizeof(Doctor));
        archivo.seekp(pos);
    }
    
    archivo.write(reinterpret_cast<const char*>(&d), sizeof(Doctor));
    archivo.close();

    if (esNuevo) {
        guardarDatosHospital(h);
    }
    
    return true;
}

bool escribirCita(Cita& c, Hospital* h) {
    bool esNuevo = (c.idCita == 0);
    fstream archivo(ARCHIVO_CITAS, ios::binary | ios::in | ios::out | ios::ate);
    
    if (!archivo.is_open()) {
        archivo.open(ARCHIVO_CITAS, ios::binary | ios::out);
        if (!archivo.is_open()) {
             cerr << "ERROR: No se pudo abrir/crear el archivo de citas." << endl;
             return false;
        }
    }

    long pos;
    if (esNuevo) {
        h->ultimoIdCita++;
        c.idCita = h->ultimoIdCita;
        c.activo = true;
        h->totalCitas++;
        pos = calcularPosicion(c.idCita, sizeof(Cita));
        archivo.seekp(pos);
    } else {
        pos = calcularPosicion(c.idCita, sizeof(Cita));
        archivo.seekp(pos);
    }
    
    archivo.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
    archivo.close();

    if (esNuevo) {
        guardarDatosHospital(h);
    }
    
    return true;
}

   int menuPrincipal() {
    int opcion;
    cout << "\n╔════════════════════════════════════════╗" << endl;
    cout << "║   SISTEMA DE GESTION HOSPITALARIA      ║" << endl;
    cout << "║                                        ║" << endl;
    cout << "╚════════════════════════════════════════╝" << endl;
    cout << "1. Gestion de Pacientes" << endl;
    cout << "2. Gestion de Doctores" << endl;
    cout << "3. Gestion de Citas" << endl;
    cout << "4. Consultas y Reportes" << endl;
    cout << "5. Mantenimiento de Archivos" << endl;
    cout << "6. Guardar y Salir" << endl;
    cout << "Opcion: ";
    if (!(cin >> opcion)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }
    return opcion;
}

void limpiarBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void obtenerFechaActual(char* fecha) {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    sprintf(fecha, "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
}

void mostrarPaciente(const Paciente& p) {
    cout << "  -------------------------------------------------" << endl;
    cout << "  ID: " << p.idPaciente << (p.activo ? "" : " (INACTIVO)") << endl;
    cout << "  Nombre: " << p.nombre << " " << p.apellido << endl;
    cout << "  Cedula: " << p.cedula << endl;
    cout << "  Edad: " << p.edad << ", Sexo: " << p.sexo << endl;
    cout << "  Citas Agendadas: " << p.numCitas << endl;
    cout << "  Historial de Consultas: " << p.numHistorial << endl;
    cout << "  -------------------------------------------------" << endl;
}

void crearPaciente(Hospital* h) {
    Paciente p;
    p.idPaciente = 0; // Marcar como nuevo registro

    cout << "\n--- Registrar Nuevo Paciente ---" << endl;
    cout << "Nombre: ";
    limpiarBuffer();
    cin.getline(p.nombre, 51);
    cout << "Apellido: ";
    cin.getline(p.apellido, 51);
    cout << "Cedula: ";
    cin.getline(p.cedula, 11);
    cout << "Edad: ";
    cin >> p.edad;
    cout << "Sexo (M/F/O): ";
    cin >> p.sexo;
    p.sexo = toupper(p.sexo);
    limpiarBuffer();

    if (escribirPaciente(p, h)) {
        cout << "\n Paciente " << p.nombre << " registrado con ID: " << p.idPaciente << endl;
    } else {
        cout << "\n ERROR al guardar el paciente en el archivo." << endl;
    }
}


    void crearPaciente(Hospital* h, const char* nombre, const char* apellido, const char* cedula, int edad, char sexo) {
    Paciente p;
    p.idPaciente = 0;
    strncpy(p.nombre, nombre, 51);
    strncpy(p.apellido, apellido, 51);
    strncpy(p.cedula, cedula, 11);
    p.edad = edad;
    p.sexo = toupper(sexo);
    escribirPaciente(p, h);
}
    void buscarPacientePorCedula(Hospital* h) {
    char cedula[11];
    cout << "Ingrese la cedula del paciente a buscar: ";
    limpiarBuffer();
    cin.getline(cedula, 11);

    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de pacientes no disponible." << endl;
        return;
    }

 Paciente p;
    bool encontrado = false;
    
    // Recorrido secuencial (ineficiente pero necesario sin indexación)
    for (int i = 1; i <= h->ultimoIdPaciente; ++i) {
        long pos = calcularPosicion(i, sizeof(Paciente));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
            if (strcmp(p.cedula, cedula) == 0) {
                mostrarPaciente(p);
                encontrado = true;
                break;
            }
        }
    }
    archivo.close();

    if (!encontrado) {
        cout << "\nPaciente con cedula " << cedula << " no encontrado o inactivo." << endl;
    }
}

void buscarPacientePorNombre(Hospital* h) {
    char nombreBuscar[51];
    cout << "Ingrese el nombre/apellido parcial del paciente a buscar: ";
    limpiarBuffer();
    cin.getline(nombreBuscar, 51);

    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de pacientes no disponible." << endl;
        return;
    }
    Paciente p;
    bool encontrado = false;
    
    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdPaciente; ++i) {
        long pos = calcularPosicion(i, sizeof(Paciente));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
            // Convierte a minúsculas para la búsqueda sin distinción
            char nombreLower[51], apellidoLower[51], buscarLower[51];
            
            // Copiar y convertir (lógica similar a la del código original)
            for (int k = 0; p.nombre[k] && k < 50; ++k) nombreLower[k] = tolower(p.nombre[k]);
            nombreLower[strlen(p.nombre)] = '\0';
            
            for (int k = 0; p.apellido[k] && k < 50; ++k) apellidoLower[k] = tolower(p.apellido[k]);
            apellidoLower[strlen(p.apellido)] = '\0';

            for (int k = 0; nombreBuscar[k] && k < 50; ++k) buscarLower[k] = tolower(nombreBuscar[k]);
            buscarLower[strlen(nombreBuscar)] = '\0';

            if (strstr(nombreLower, buscarLower) || strstr(apellidoLower, buscarLower)) {
                mostrarPaciente(p);
                encontrado = true;
            }
        }
    }
    archivo.close();
if (!encontrado) {
        cout << "\nNo se encontraron pacientes activos con ese nombre/apellido." << endl;
    }
}
void verHistorialMedicoCompleto(Hospital* h) {
    int id;
    cout << "Ingrese el ID del paciente para ver su historial: ";
    cin >> id;

    Paciente p;
    if (!leerPacientePorId(id, p)) {
        cout << "\n Paciente con ID " << id << " no encontrado o inactivo." << endl;
        return;
    }

    cout << "\n==========================================" << endl;
    cout << "  HISTORIAL MEDICO de " << p.nombre << " " << p.apellido << endl;
    cout << "==========================================" << endl;

    if (p.numHistorial == 0) {
        cout << "El paciente no tiene entradas en el historial." << endl;
        return;
    }
    
    for (int i = 0; i < p.numHistorial; ++i) {
        const HistorialMedico& hm = p.historial[i];
        if (hm.activo) {
            cout << "\n--- Consulta #" << hm.idConsulta << (hm.activo ? "" : " (INACTIVA)") << " ---" << endl;
            cout << "Fecha/Hora: " << hm.fecha << " " << hm.hora << endl;
            cout << "Doctor ID: " << hm.idDoctor << endl;
            cout << "Diagnostico: " << hm.diagnostico << endl;
            cout << "Tratamiento: " << hm.tratamiento << endl;
            cout << "Medicamentos: " << hm.medicamentos << endl;
            cout << "Costo: " << fixed << setprecision(2) << hm.costoConsulta << endl;
        }
    }
}
void actualizarDatosPaciente(Hospital* h) {
    int id;
    cout << "Ingrese el ID del paciente a actualizar: ";
    cin >> id;

    Paciente p;
    if (!leerPacientePorId(id, p)) {
        cout << "\n❌ Paciente con ID " << id << " no encontrado o inactivo." << endl;
        return;
    }

   cout << "\n--- Actualizando datos de: " << p.nombre << " " << p.apellido << " ---" << endl;
    limpiarBuffer();
    
    char input[51];

    cout << "Nuevo Nombre (" << p.nombre << "): ";
    cin.getline(input, 51);
    if (input[0] != '\0') strncpy(p.nombre, input, 51);

    cout << "Nuevo Apellido (" << p.apellido << "): ";
    cin.getline(input, 51);
    if (input[0] != '\0') strncpy(p.apellido, input, 51);

    int nuevaEdad;
    cout << "Nueva Edad (" << p.edad << "): ";
    cin >> nuevaEdad;
    if (cin.fail()) { cin.clear(); } else { p.edad = nuevaEdad; }
    limpiarBuffer();

    cout << "Nuevo Sexo (" << p.sexo << ") (M/F/O): ";
    char nuevoSexo;
    cin >> nuevoSexo;
    nuevoSexo = toupper(nuevoSexo);
    if (!cin.fail() && (nuevoSexo == 'M' || nuevoSexo == 'F' || nuevoSexo == 'O')) {
        p.sexo = nuevoSexo;
    }
    limpiarBuffer();

    if (escribirPaciente(p, h)) {
        cout << "\n Datos del paciente ID " << p.idPaciente << " actualizados con exito." << endl;
    } else {
        cout << "\n ERROR al sobrescribir el registro del paciente." << endl;
    }
}

void listarPacientes(Hospital* h) {
    cout << "\n==========================================" << endl;
    cout << "  LISTADO DE PACIENTES ACTIVOS (" << h->totalPacientes << ")" << endl;
    cout << "==========================================" << endl;

    fstream archivo(ARCHIVO_PACIENTES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de pacientes no disponible." << endl;
        return;
    }

    Paciente p;
    int cont = 0;
    
    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdPaciente; ++i) {
        long pos = calcularPosicion(i, sizeof(Paciente));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
            cout << i << ". ID: " << p.idPaciente 
                 << ", Nombre: " << p.nombre << " " << p.apellido 
                 << ", Cedula: " << p.cedula 
                 << ", Edad: " << p.edad 
                 << ", Historial: " << p.numHistorial 
                 << ", Citas: " << p.numCitas << endl;
            cont++;
        }
    }
    archivo.close();
    
    if (cont == 0) {
        cout << "No hay pacientes activos registrados." << endl;
    }
}

void eliminarPaciente(Hospital* h) {
    int id;
    cout << "Ingrese el ID del paciente a eliminar (eliminación logica): ";
    cin >> id;
    
    Paciente p;
    if (!leerPacientePorId(id, p)) {
        cout << "\n❌ Paciente con ID " << id << " no encontrado o ya inactivo." << endl;
        return;
    }
    // 1. Marcar como inactivo
    p.activo = false;
    if (escribirPaciente(p, h)) {
        // 2. Actualizar contadores del Hospital
        h->totalPacientes--;
        guardarDatosHospital(h);

        // 3. Eliminación lógica de referencias cruzadas
        
        // 3.1. Eliminar referencias en Doctores:
        // Se hace un recorrido secuencial de doctores para quitar la referencia
        fstream archDoctores(ARCHIVO_DOCTORES, ios::binary | ios::in | ios::out);
        Doctor d;
        for (int i = 1; i <= h->ultimoIdDoctor; ++i) {
            long pos = calcularPosicion(i, sizeof(Doctor));
            archDoctores.seekg(pos);
            if (archDoctores.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
                bool modificado = false;
                for (int j = 0; j < d.numPacientesAsignados; ++j) {
                    if (d.idPacientesAsignados[j] == id) {
                        // Mover elementos para eliminar el ID del array
                        for (int k = j; k < d.numPacientesAsignados - 1; ++k) {
                            d.idPacientesAsignados[k] = d.idPacientesAsignados[k + 1];
                        }
                        d.numPacientesAsignados--;
                        j--; // Retroceder para revisar el elemento que se movió a esta posición
                        modificado = true;
                    }
                }
                
                if (modificado) {
                    archDoctores.seekp(pos);
                    archDoctores.write(reinterpret_cast<const char*>(&d), sizeof(Doctor));
                }
            }
        }
        archDoctores.close();

        // 3.2. Cancelar Citas del paciente:
        // Se hace un recorrido secuencial de citas para marcarlas como canceladas
        fstream archCitas(ARCHIVO_CITAS, ios::binary | ios::in | ios::out);
        Cita c;
        for (int i = 1; i <= h->ultimoIdCita; ++i) {
            long pos = calcularPosicion(i, sizeof(Cita));
            archCitas.seekg(pos);
            if (archCitas.read(reinterpret_cast<char*>(&c), sizeof(Cita)) && c.activo) {
                if (c.idPaciente == id && c.estado == 0) { // Si es cita del paciente y está agendada
                    c.estado = 2; // Cancelada
                    c.activo = false; // Eliminación lógica
                    
                    archCitas.seekp(pos);
                    archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                    
                    // Actualizar el contador de citas en Hospital
                    h->totalCitas--;
                    
                    // Quitar referencia en el Doctor asociado
                    Doctor dAux;
                    if (leerDoctorPorId(c.idDoctor, dAux)) {
                        bool docModificado = false;
                        for (int j = 0; j < dAux.numCitas; ++j) {
                            if (dAux.idCitas[j] == c.idCita) {
                                for (int k = j; k < dAux.numCitas - 1; ++k) {
                                    dAux.idCitas[k] = dAux.idCitas[k + 1];
                                }
                                dAux.numCitas--;
                                docModificado = true;
                                break;
                            }
                        }
                        if (docModificado) escribirDoctor(dAux, h);
                    }
                }
            }
        }
        archCitas.close();
        guardarDatosHospital(h); // Guardar el totalCitas actualizado
        
        cout << "\n Paciente ID " << id << " eliminado lógicamente. Sus citas han sido canceladas y referencias eliminadas." << endl;
    } else {
        cout << "\n ERROR al marcar el paciente como inactivo." << endl;
    }
}
void gestionarPacientes(Hospital* h) {
    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "      SUBMENU GESTION DE PACIENTES" << endl;
        cout << "========================================" << endl;
        cout << "1. Registrar nuevo paciente" << endl;
        cout << "2. Buscar paciente por cedula" << endl;
        cout << "3. Buscar paciente por nombre" << endl;
        cout << "4. Ver historial médico completo" << endl;
        cout << "5. Actualizar datos del paciente" << endl;
        cout << "6. Listar todos los pacientes" << endl;
        cout << "7. Eliminar paciente (Logico)" << endl;
        cout << "0. Volver al menú principal" << endl;
        cout << "Opcion: ";
        
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: crearPaciente(h); break;
            case 2: buscarPacientePorCedula(h); break;
            case 3: buscarPacientePorNombre(h); break;
            case 4: verHistorialMedicoCompleto(h); break;
            case 5: actualizarDatosPaciente(h); break;
            case 6: listarPacientes(h); break;
            case 7: eliminarPaciente(h); break;
            case 0: cout << "Volviendo al menu principal." << endl; break;
            default: cout << "Opcion no valida. Intente de nuevo." << endl; break;
        }
    } while (opcion != 0);
}
// ----------------------
// DOCTORES (RUTINAS)
// ----------------------

void mostrarDoctor(const Doctor& d) {
    cout << "  -------------------------------------------------" << endl;
    cout << "  ID: " << d.idDoctor << (d.activo ? "" : " (INACTIVO)") << endl;
    cout << "  Nombre: " << d.nombre << " " << d.apellido << endl;
    cout << "  Cedula Prof: " << d.cedulaProfesional << endl;
    cout << "  Especialidad: " << d.especialidad << endl;
    cout << "  Experiencia: " << d.experienciaAnios << " años" << endl;
    cout << "  Costo Consulta: " << fixed << setprecision(2) << d.costoConsulta << endl;
    cout << "  Pacientes Asignados: " << d.numPacientesAsignados << endl;
    cout << "  Citas Agendadas: " << d.numCitas << endl;
    cout << "  -------------------------------------------------" << endl;
}
void crearDoctor(Hospital* h) {
    Doctor d;
    d.idDoctor = 0; // Marcar como nuevo registro

    cout << "\n--- Registrar Nuevo Doctor ---" << endl;
    cout << "Nombre: ";
    limpiarBuffer();
    cin.getline(d.nombre, 51);
    cout << "Apellido: ";
    cin.getline(d.apellido, 51);
    cout << "Cédula Profesional: ";
    cin.getline(d.cedulaProfesional, 11);
    cout << "Especialidad: ";
    cin.getline(d.especialidad, 51);
    cout << "Años de Experiencia: ";
    cin >> d.experienciaAnios;
    cout << "Costo de Consulta: ";
    cin >> d.costoConsulta;
    limpiarBuffer();

    if (escribirDoctor(d, h)) {
        cout << "\n✅ Doctor " << d.nombre << " registrado con ID: " << d.idDoctor << endl;
    } else {
        cout << "\n❌ ERROR al guardar el doctor en el archivo." << endl;
    }
}
// Sobrecarga de la función original para precarga
void crearDoctor(Hospital* h, const char* nombre, const char* apellido, const char* cedulaProf, const char* especialidad, int experiencia, float costo) {
    Doctor d;
    d.idDoctor = 0;
    strncpy(d.nombre, nombre, 51);
    strncpy(d.apellido, apellido, 51);
    strncpy(d.cedulaProfesional, cedulaProf, 11);
    strncpy(d.especialidad, especialidad, 51);
    d.experienciaAnios = experiencia;
    d.costoConsulta = costo;
    escribirDoctor(d, h);
}
void buscarDoctorPorCedulaProfesional(Hospital* h) {
    char cedula[11];
    cout << "Ingrese la cedula profesional del doctor a buscar: ";
    limpiarBuffer();
    cin.getline(cedula, 11);

    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de doctores no disponible." << endl;
        return;
    }

    Doctor d;
    bool encontrado = false;
    
    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdDoctor; ++i) {
        long pos = calcularPosicion(i, sizeof(Doctor));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
            if (strcmp(d.cedulaProfesional, cedula) == 0) {
                mostrarDoctor(d);
                encontrado = true;
                break;
            }
        }
    }
    archivo.close();
    
    if (!encontrado) {
        cout << "\nDoctor con cedula profesional " << cedula << " no encontrado o inactivo." << endl;
    }
}
void buscarDoctoresPorEspecialidad(Hospital* h) {
    char especialidadBuscar[51];
    cout << "Ingrese la especialidad a buscar: ";
    limpiarBuffer();
    cin.getline(especialidadBuscar, 51);

    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de doctores no disponible." << endl;
        return;
    }

    Doctor d;
    bool encontrado = false;
    
    // Convertir la entrada a minúsculas para búsqueda sin distinción
    char buscarLower[51];
    for (int k = 0; especialidadBuscar[k] && k < 50; ++k) buscarLower[k] = tolower(especialidadBuscar[k]);
    buscarLower[strlen(especialidadBuscar)] = '\0';

    cout << "\n--- Doctores en la especialidad '" << especialidadBuscar << "' ---" << endl;

    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdDoctor; ++i) {
        long pos = calcularPosicion(i, sizeof(Doctor));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
            char especialidadLower[51];
            for (int k = 0; d.especialidad[k] && k < 50; ++k) especialidadLower[k] = tolower(d.especialidad[k]);
            especialidadLower[strlen(d.especialidad)] = '\0';
            
            if (strstr(especialidadLower, buscarLower)) {
                mostrarDoctor(d);
                encontrado = true;
            }
        }
    }
    archivo.close();
    
    if (!encontrado) {
        cout << "No se encontraron doctores activos en esa especialidad." << endl;
    }
}

void asignarPacienteADoctor(Hospital* h) {
    int idDoctor, idPaciente;
    cout << "Ingrese el ID del Doctor: ";
    cin >> idDoctor;
    cout << "Ingrese el ID del Paciente a asignar: ";
    cin >> idPaciente;

    Doctor d;
    Paciente p;
    
    if (!leerDoctorPorId(idDoctor, d)) {
        cout << " Doctor ID " << idDoctor << " no encontrado o inactivo." << endl;
        return;
    }

    if (!leerPacientePorId(idPaciente, p)) {
        cout << " Paciente ID " << idPaciente << " no encontrado o inactivo." << endl;
        return;
    }
    
    // Verificar si ya está asignado
    for (int i = 0; i < d.numPacientesAsignados; ++i) {
        if (d.idPacientesAsignados[i] == idPaciente) {
            cout << " Paciente ID " << idPaciente << " ya esta asignado al Doctor ID " << idDoctor << "." << endl;
            return;
        }
    }
    
    // Verificar capacidad
    if (d.numPacientesAsignados >= MAX_PACIENTES_DOCTOR) {
        cout << " El doctor ha alcanzado su limite de pacientes asignados (" << MAX_PACIENTES_DOCTOR << ")." << endl;
        return;
    }

    // Asignar
    d.idPacientesAsignados[d.numPacientesAsignados++] = idPaciente;
    
    if (escribirDoctor(d, h)) {
        cout << "\n Paciente " << p.nombre << " " << p.apellido << " asignado al Doctor " << d.nombre << " " << d.apellido << "." << endl;
    } else {
        cout << "\n ERROR al sobrescribir el registro del doctor." << endl;
    }
}

void verPacientesAsignadosADoctor(Hospital* h) {
    int idDoctor;
    cout << "Ingrese el ID del Doctor para ver sus pacientes asignados: ";
    cin >> idDoctor;

    Doctor d;
    if (!leerDoctorPorId(idDoctor, d)) {
        cout << " Doctor ID " << idDoctor << " no encontrado o inactivo." << endl;
        return;
    }
    
    cout << "\n==========================================" << endl;
    cout << "  PACIENTES ASIGNADOS a Dr/a. " << d.nombre << " " << d.apellido << endl;
    cout << "==========================================" << endl;

    if (d.numPacientesAsignados == 0) {
        cout << "El doctor no tiene pacientes asignados." << endl;
        return;
    }

    Paciente p;
    for (int i = 0; i < d.numPacientesAsignados; ++i) {
        if (leerPacientePorId(d.idPacientesAsignados[i], p)) {
            cout << i+1 << ". ID: " << p.idPaciente 
                 << ", Nombre: " << p.nombre << " " << p.apellido 
                 << ", Cédula: " << p.cedula << endl;
        } else {
            cout << i+1 << ". ID: " << d.idPacientesAsignados[i] << " (Referencia rota - Paciente inactivo/eliminado)" << endl;
        }
    }
}
void listarDoctores(Hospital* h) {
    cout << "\n==========================================" << endl;
    cout << "  LISTADO DE DOCTORES ACTIVOS (" << h->totalDoctores << ")" << endl;
    cout << "==========================================" << endl;

    fstream archivo(ARCHIVO_DOCTORES, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de doctores no disponible." << endl;
        return;
    }

    Doctor d;
    int cont = 0;
    
    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdDoctor; ++i) {
        long pos = calcularPosicion(i, sizeof(Doctor));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&d), sizeof(Doctor)) && d.activo) {
            cout << i << ". ID: " << d.idDoctor 
                 << ", Nombre: " << d.nombre << " " << d.apellido 
                 << ", Especialidad: " << d.especialidad 
                 << ", Citas: " << d.numCitas << endl;
            cont++;
        }
    }
    archivo.close();
    
    if (cont == 0) {
        cout << "No hay doctores activos registrados." << endl;
    }
}
void eliminarDoctor(Hospital* h) {
    int id;
    cout << "Ingrese el ID del doctor a eliminar (eliminación logica): ";
    cin >> id;

    Doctor d;
    if (!leerDoctorPorId(id, d)) {
        cout << "\n Doctor con ID " << id << " no encontrado o ya inactivo." << endl;
        return;
    }
    
    // 1. Marcar como inactivo
    d.activo = false;
    if (escribirDoctor(d, h)) {
        // 2. Actualizar contadores del Hospital
        h->totalDoctores--;
        guardarDatosHospital(h);
        
        // 3. Eliminación lógica de referencias cruzadas
        
        // 3.1. Eliminar referencias a citas y citas del doctor
        fstream archCitas(ARCHIVO_CITAS, ios::binary | ios::in | ios::out);
        Cita c;
        for (int i = 1; i <= h->ultimoIdCita; ++i) {
            long pos = calcularPosicion(i, sizeof(Cita));
            archCitas.seekg(pos);
            if (archCitas.read(reinterpret_cast<char*>(&c), sizeof(Cita)) && c.activo) {
                if (c.idDoctor == id && c.estado == 0) { // Si es cita del doctor y está agendada
                    c.estado = 2; // Cancelada
                    c.activo = false; // Eliminación lógica de la cita
                    
                    archCitas.seekp(pos);
                    archCitas.write(reinterpret_cast<const char*>(&c), sizeof(Cita));
                    
                    // Actualizar el contador de citas en Hospital
                    h->totalCitas--;
                    
                    // Quitar referencia en el Paciente asociado
                    Paciente pAux;
                    if (leerPacientePorId(c.idPaciente, pAux)) {
                        bool pacModificado = false;
                        for (int j = 0; j < pAux.numCitas; ++j) {
                            if (pAux.idCitas[j] == c.idCita) {
                                for (int k = j; k < pAux.numCitas - 1; ++k) {
                                    pAux.idCitas[k] = pAux.idCitas[k + 1];
                                }
                                pAux.numCitas--;
                                pacModificado = true;
                                break;
                            }
                        }
                        if (pacModificado) escribirPaciente(pAux, h);
                    }
                }
            }
        }
        archCitas.close();

        // 3.2. Actualizar referencias en Historial (solo se elimina el ID, la entrada queda inactiva si no se compacta)
        fstream archPacientes(ARCHIVO_PACIENTES, ios::binary | ios::in | ios::out);
        Paciente p;
        for (int i = 1; i <= h->ultimoIdPaciente; ++i) {
            long pos = calcularPosicion(i, sizeof(Paciente));
            archPacientes.seekg(pos);
            if (archPacientes.read(reinterpret_cast<char*>(&p), sizeof(Paciente)) && p.activo) {
                bool modificado = false;
                for (int j = 0; j < p.numHistorial; ++j) {
                    if (p.historial[j].idDoctor == id) {
                        p.historial[j].activo = false;
                        modificado = true;
                    }
                }
                if (modificado) {
                    archPacientes.seekp(pos);
                    archPacientes.write(reinterpret_cast<const char*>(&p), sizeof(Paciente));
                }
            }
        }
        archPacientes.close();

        guardarDatosHospital(h); // Guardar el totalCitas actualizado

        cout << "\n Doctor ID " << id << " eliminado logicamente. Sus citas han sido canceladas y referencias a el en historial marcadas como inactivas." << endl;
    } else {
        cout << "\n ERROR al marcar el doctor como inactivo." << endl;
    }
}
void gestionarDoctores(Hospital* h) {
    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "      SUBMENU GESTION DE DOCTORES" << endl;
        cout << "========================================" << endl;
        cout << "1. Registrar nuevo doctor" << endl;
        cout << "2. Buscar doctor por cedula profesional" << endl;
        cout << "3. Buscar doctores por especialidad" << endl;
        cout << "4. Asignar paciente a doctor" << endl;
        cout << "5. Ver pacientes asignados a doctor" << endl;
        cout << "6. Listar todos los doctores" << endl;
        cout << "7. Eliminar doctor (Lógico)" << endl;
        cout << "0. Volver al menú principal" << endl;
        cout << "Opcion: ";

        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: crearDoctor(h); break;
            case 2: buscarDoctorPorCedulaProfesional(h); break;
            case 3: buscarDoctoresPorEspecialidad(h); break;
            case 4: asignarPacienteADoctor(h); break;
            case 5: verPacientesAsignadosADoctor(h); break;
            case 6: listarDoctores(h); break;
            case 7: eliminarDoctor(h); break;
            case 0: cout << "Volviendo al menú principal." << endl; break;
            default: cout << "Opcion no valida. Intente de nuevo." << endl; break;
        }
    } while (opcion != 0);
}
// ----------------------
// CITAS (RUTINAS)
// ----------------------

void agendarCita(Hospital* h) {
    int idPaciente, idDoctor;
    char fecha[11], hora[6], motivo[101];

    cout << "\n--- Agendar Nueva Cita ---" << endl;
    cout << "ID del Paciente: ";
    cin >> idPaciente;
    cout << "ID del Doctor: ";
    cin >> idDoctor;
    limpiarBuffer();

    Paciente p;
    Doctor d;
    
    if (!leerPacientePorId(idPaciente, p)) {
        cout << " Paciente ID " << idPaciente << " no encontrado o inactivo." << endl;
        return;
    }
    
    if (!leerDoctorPorId(idDoctor, d)) {
        cout << " Doctor ID " << idDoctor << " no encontrado o inactivo." << endl;
        return;
    }

    cout << "Fecha (YYYY-MM-DD): ";
    cin.getline(fecha, 11);
    cout << "Hora (HH:MM): ";
    cin.getline(hora, 6);
    cout << "Motivo: ";
    cin.getline(motivo, 101);

    // Verificar capacidad
    if (p.numCitas >= MAX_CITAS_PACIENTE) {
        cout << " El paciente ha alcanzado su límite de citas agendadas (" << MAX_CITAS_PACIENTE << ")." << endl;
        return;
    }
    if (d.numCitas >= MAX_CITAS_DOCTOR) {
        cout << " El doctor ha alcanzado su límite de citas agendadas (" << MAX_CITAS_DOCTOR << ")." << endl;
        return;
    }

    Cita c;
    c.idCita = 0; // Nuevo registro
    c.idPaciente = idPaciente;
    c.idDoctor = idDoctor;
    strncpy(c.fecha, fecha, 11);
    strncpy(c.hora, hora, 6);
    strncpy(c.motivo, motivo, 101);
    c.estado = 0; // Agendada

    if (escribirCita(c, h)) {
        // Actualizar referencias en Paciente
        p.idCitas[p.numCitas++] = c.idCita;
        escribirPaciente(p, h);
        
        // Actualizar referencias en Doctor
        d.idCitas[d.numCitas++] = c.idCita;
        escribirDoctor(d, h);
        
        cout << "\n Cita agendada con ID: " << c.idCita << endl;
    } else {
        cout << "\n ERROR al guardar la cita en el archivo." << endl;
    }
}
// Sobrecarga de la función original para precarga
void agendarCita(Hospital* h, int idPaciente, int idDoctor, const char* fecha, const char* hora, const char* motivo) {
    
    Paciente p;
    Doctor d;
    if (!leerPacientePorId(idPaciente, p) || !leerDoctorPorId(idDoctor, d)) {
        cerr << "ADVERTENCIA: No se pudo agendar cita de precarga. Paciente/Doctor inactivo/no existe." << endl;
        return;
    }

    if (p.numCitas >= MAX_CITAS_PACIENTE || d.numCitas >= MAX_CITAS_DOCTOR) {
        cerr << "ADVERTENCIA: No se pudo agendar cita de precarga. Limite alcanzado." << endl;
        return;
    }

    Cita c;
    c.idCita = 0; 
    c.idPaciente = idPaciente;
    c.idDoctor = idDoctor;
    strncpy(c.fecha, fecha, 11);
    strncpy(c.hora, hora, 6);
    strncpy(c.motivo, motivo, 101);
    c.estado = 0;

    if (escribirCita(c, h)) {
        p.idCitas[p.numCitas++] = c.idCita;
        escribirPaciente(p, h);
        
        d.idCitas[d.numCitas++] = c.idCita;
        escribirDoctor(d, h);
    }
}

void cancelarCita(Hospital* h) {
    int idCita;
    cout << "Ingrese el ID de la Cita a cancelar: ";
    cin >> idCita;

    Cita c;
    if (!leerCitaPorId(idCita, c)) {
        cout << "\n Cita ID " << idCita << " no encontrada o ya está inactiva/cancelada." << endl;
        return;
    }

    if (c.estado != 0) {
        cout << "\n La cita ya esta en estado ";
        if (c.estado == 1) cout << "ATENDIDA";
        else if (c.estado == 2) cout << "CANCELADA";
        cout << " y no puede ser cancelada." << endl;
        return;
    }

    // 1. Marcar cita como cancelada (estado=2) y como inactiva (activo=false)
    c.estado = 2;
    c.activo = false;
    
    if (escribirCita(c, h)) {
        // 2. Actualizar contadores del Hospital
        h->totalCitas--;
        guardarDatosHospital(h);

        // 3. Eliminar referencia en Paciente
        Paciente p;
        if (leerPacientePorId(c.idPaciente, p)) {
            bool modificado = false;
            for (int i = 0; i < p.numCitas; ++i) {
                if (p.idCitas[i] == idCita) {
                    for (int j = i; j < p.numCitas - 1; ++j) {
                        p.idCitas[j] = p.idCitas[j + 1];
                    }
                    p.numCitas--;
                    modificado = true;
                    break;
                }
            }
            if (modificado) escribirPaciente(p, h);
        }
        // 4. Eliminar referencia en Doctor
        Doctor d;
        if (leerDoctorPorId(c.idDoctor, d)) {
            bool modificado = false;
            for (int i = 0; i < d.numCitas; ++i) {
                if (d.idCitas[i] == idCita) {
                    for (int j = i; j < d.numCitas - 1; ++j) {
                        d.idCitas[j] = d.idCitas[j + 1];
                    }
                    d.numCitas--;
                    modificado = true;
                    break;
                }
            }
            if (modificado) escribirDoctor(d, h);
        }
        
        cout << "\n Cita ID " << idCita << " cancelada exitosamente y referencias eliminadas." << endl;
    } else {
        cout << "\n ERROR al sobrescribir el registro de la cita." << endl;
    }
}
void atenderCita(Hospital* h) {
    int idCita;
    cout << "Ingrese el ID de la Cita a atender: ";
    cin >> idCita;

    Cita c;
    if (!leerCitaPorId(idCita, c)) {
        cout << "\n❌ Cita ID " << idCita << " no encontrada o ya esta inactiva." << endl;
        return;
    }

    if (c.estado != 0) {
        cout << "\n La cita ya esta en estado ";
        if (c.estado == 1) cout << "ATENDIDA";
        else if (c.estado == 2) cout << "CANCELADA";
        cout << " y no puede ser atendida nuevamente." << endl;
        return;
    }

    // 1. Solicitar datos de la consulta
    HistorialMedico hm;
    hm.idConsulta = 0; // Se asigna dentro del paciente
    hm.idDoctor = c.idDoctor;
    hm.costoConsulta = 0.0f; // Se actualizará más abajo
    hm.activo = true;
    
    cout << "\n--- Ingreso de Historial para Cita ID " << idCita << " ---" << endl;
    limpiarBuffer();
    cout << "Diagnastico (max 200 chars): ";
    cin.getline(hm.diagnostico, 201);
    cout << "Tratamiento (max 200 chars): ";
    cin.getline(hm.tratamiento, 201);
    cout << "Medicamentos (max 150 chars): ";
    cin.getline(hm.medicamentos, 151);

    // 2. Obtener costo de doctor
    Doctor d;
    float costoBase = 0.0f;
    if (leerDoctorPorId(c.idDoctor, d)) {
        costoBase = d.costoConsulta;
        cout << "Costo base del Doctor: " << fixed << setprecision(2) << costoBase << endl;
    } else {
        cout << "ADVERTENCIA: Doctor inactivo/eliminado, costo base 0.00" << endl;
    }

    // Asignar el costo
    hm.costoConsulta = costoBase;
    strncpy(hm.fecha, c.fecha, 11);
    strncpy(hm.hora, c.hora, 6);

    // 3. Actualizar Paciente (agregar historial y eliminar referencia a cita)
    Paciente p;
    if (leerPacientePorId(c.idPaciente, p)) {
        if (p.numHistorial >= MAX_HISTORIAL_MEDICO) {
            cout << " ERROR: El paciente ha alcanzado el limite de entradas en el historial (" << MAX_HISTORIAL_MEDICO << ")." << endl;
            return;
        }

        // 3.1. Asignar ID de consulta y agregar al historial
        hm.idConsulta = p.numHistorial + 1;
        p.historial[p.numHistorial++] = hm;
        
        // 3.2. Eliminar referencia a cita
        bool citaReferenciaEliminada = false;
        for (int i = 0; i < p.numCitas; ++i) {
            if (p.idCitas[i] == idCita) {
                for (int j = i; j < p.numCitas - 1; ++j) {
                    p.idCitas[j] = p.idCitas[j + 1];
                }
                p.numCitas--;
                citaReferenciaEliminada = true;
                break;
            }
        }

         if (citaReferenciaEliminada) {
            escribirPaciente(p, h);
            cout << "✅ Historial medico actualizado y referencia de cita eliminada en paciente." << endl;
        } else {
            cout << "ADVERTENCIA: No se encontro la referencia de la cita en el paciente, pero el historial fue añadido." << endl;
        }
    } else {
        cout << "❌ ERROR: Paciente de la cita no encontrado, no se puede guardar el historial." << endl;
        return;
    }

    // 4. Actualizar Doctor (eliminar referencia a cita)
    if (leerDoctorPorId(c.idDoctor, d)) {
        bool modificado = false;
        for (int i = 0; i < d.numCitas; ++i) {
            if (d.idCitas[i] == idCita) {
                for (int j = i; j < d.numCitas - 1; ++j) {
                    d.idCitas[j] = d.idCitas[j + 1];
                }
                d.numCitas--;
                modificado = true;
                break;
            }
        }
        if (modificado) escribirDoctor(d, h);
    }
    
    // 5. Marcar cita como atendida (estado=1) y como inactiva (activo=false)
    c.estado = 1;
    c.activo = false;
    if (escribirCita(c, h)) {
        h->totalCitas--;
        guardarDatosHospital(h);
        cout << "\n Cita ID " << idCita << " marcada como atendida y eliminada logicamente." << endl;
    } else {
        cout << "\n ERROR al sobrescribir el registro de la cita. Las referencias ya fueron eliminadas." << endl;
    }
}
void gestionarCitas(Hospital* h) {
    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "        SUBMENU GESTION DE CITAS" << endl;
        cout << "========================================" << endl;
        cout << "1. Agendar nueva cita" << endl;
        cout << "2. Cancelar cita (Logico)" << endl;
        cout << "3. Atender cita y registrar historial (Logico)" << endl;
        cout << "0. Volver al menu principal" << endl;
        cout << "Opcion: ";

        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: agendarCita(h); break;
            case 2: cancelarCita(h); break;
            case 3: atenderCita(h); break;
            case 0: cout << "Volviendo al menu principal." << endl; break;
            default: cout << "Opcion no valida. Intente de nuevo." << endl; break;
        }
    } while (opcion != 0);
}

// ----------------------
// CONSULTAS Y REPORTES (RUTINAS)
// ----------------------

void mostrarEstadoCita(int estado) {
    switch (estado) {
        case 0: cout << "AGENDADA"; break;
        case 1: cout << "ATENDIDA"; break;
        case 2: cout << "CANCELADA"; break;
        default: cout << "DESCONOCIDO"; break;
    }
}


void listarCitas(Hospital* h) {
    cout << "\n==========================================" << endl;
    cout << "  LISTADO DE CITAS ACTIVAS Y ATENDIDAS (" << h->totalCitas << ")" << endl;
    cout << "==========================================" << endl;

    fstream archivo(ARCHIVO_CITAS, ios::binary | ios::in);
    if (!archivo.is_open()) {
        cout << " ERROR: Archivo de citas no disponible." << endl;
        return;
    }
    
    Cita c;
    int cont = 0;
    
    // Recorrido secuencial
    for (int i = 1; i <= h->ultimoIdCita; ++i) {
        long pos = calcularPosicion(i, sizeof(Cita));
        archivo.seekg(pos);
        if (archivo.read(reinterpret_cast<char*>(&c), sizeof(Cita)) && c.activo) {
            cout << i << ". ID: " << c.idCita 
                 << ", Paciente ID: " << c.idPaciente 
                 << ", Doctor ID: " << c.idDoctor 
                 << ", Fecha: " << c.fecha 
                 << ", Hora: " << c.hora 
                 << ", Motivo: " << c.motivo 
                 << ", Estado: ";
            mostrarEstadoCita(c.estado);
            cout << (c.estado == 0 ? " (ACTIVA)" : " (INACTIVA/FINALIZADA)"); // Clarificación
            cout << endl;
            cont++;
        }
    }
    archivo.close();
    
    if (cont == 0) {
        cout << "No hay citas activas o finalizadas (solo eliminadas logicamente)." << endl;
    }
}

void consultasYReportes(Hospital* h) {
    int opcion;
    do {
        cout << "\n========================================" << endl;
        cout << "    SUBMENU CONSULTAS Y REPORTES" << endl;
        cout << "========================================" << endl;
        cout << "1. Listar Pacientes" << endl;
        cout << "2. Listar Doctores" << endl;
        cout << "3. Listar Citas (Activas y Finalizadas)" << endl;
        cout << "0. Volver al menú principal" << endl;
        cout << "Opcion: ";
        
        if (!(cin >> opcion)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
        }

        switch (opcion) {
            case 1: listarPacientes(h); break;
            case 2: listarDoctores(h); break;
            case 3: listarCitas(h); break;
            case 0: cout << "Volviendo al menu principal." << endl; break;
            default: cout << "Opcion no valida. Intente de nuevo." << endl; break;
        }
    } while (opcion != 0);
}
int main() {
    // 1. Cargar la estructura Hospital (que contiene los contadores)
    Hospital* hospital = cargarDatosHospital();

    // Precarga de datos solo si no existen (ultimoIdPaciente == 0)
    if (hospital->ultimoIdPaciente == 0) {
        cout << "=================================================" << endl;
        cout << "      Ejecutando precarga de datos iniciales...    " << endl;
        cout << "=================================================" << endl;
        crearPaciente(hospital, "Ana", "Martinez", "12345678", 30, 'F');
        crearPaciente(hospital, "Luis", "Gomez", "98765432", 45, 'M');
        crearDoctor(hospital, "Maria", "Rodriguez", "DR-7788", "Pediatria", 10, 150.00f);
        crearDoctor(hospital, "Pedro", "Lopez", "DR-1122", "Cardiologia", 20, 300.50f);
        // Las citas de precarga usan IDs ya creados
        agendarCita(hospital, 1, 1, "2025-10-25", "10:00", "Chequeo pediatrico");
        agendarCita(hospital, 2, 2, "2025-10-25", "11:00", "Dolor de pecho");
        cout << "Precarga finalizada. Datos guardados en archivos binarios." << endl;
    }

    cout << "\n=================================================" << endl;
    cout << " Sistema de Gestion Hospitalaria Iniciado: " << hospital->nombre << endl;
    cout << "=================================================" << endl;

    int opcion;
    do {
        opcion = menuPrincipal();
        switch (opcion) {
            case 1:
                gestionarPacientes(hospital);
                break;
            case 2:
                gestionarDoctores(hospital);
                break;
            case 3:
                gestionarCitas(hospital);
                break;
            case 4:
                consultasYReportes(hospital);
                break;
            case 5:
                mantenimientoArchivos(hospital); // Nuevo submenú
                break;
            case 6:
                // Guardar y Salir (solo guarda la estructura Hospital si hubo cambios en los contadores)
                guardarDatosHospital(hospital);
                cout << "\nDatos del Hospital guardados. Saliendo del sistema. ¡Hasta pronto!" << endl;
                break;
            default:
                if (opcion != 6) {
                    cout << "Opcion no valida. Intente de nuevo." << endl;
                }
                break;
        }
    } while (opcion != 6);

    delete hospital; // Liberar la única estructura en memoria dinámica
    return 0;
}