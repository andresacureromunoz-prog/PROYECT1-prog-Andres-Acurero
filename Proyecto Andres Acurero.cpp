#include <iostream>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <cctype> // Para tolower, toupper
#include <locale.h>

using namespace std;


// Constantes
const int CAPACIDAD_INICIAL_HISTORIAL = 5;
const int CAPACIDAD_INICIAL_CITAS_PACIENTE = 5;
const int CAPACIDAD_INICIAL_PACIENTES_DOCTOR = 5;
const int CAPACIDAD_INICIAL_CITAS_DOCTOR = 10;
const int CAPACIDAD_INICIAL_HOSPITAL = 10;
const int CAPACIDAD_INICIAL_CITAS_HOSPITAL = 20;

// =================================================================
// 1. MODELO DE DATOS - ESTRUCTURAS
// =================================================================

/**
 * @struct HistorialMedico
 * @brief Representa una consulta médica individual en el historial de un paciente.
 */
struct HistorialMedico {
    int idConsulta;
    char fecha[11];      // YYYY-MM-DD\0
    char hora[6];        // HH:MM\0
    char diagnostico[201];
    char tratamiento[201];
    char medicamentos[151];
    int idDoctor;
    float costoConsulta;
};

/**
 * @struct Paciente
 * @brief Representa a un paciente del hospital.
 */
struct Paciente {
    // Identificación y datos personales
    int id;
    char nombre[51];
    char apellido[51];
    char cedula[21]; // DEBE SER ÚNICA
    
    // Datos médicos básicos
    int edad;
    char sexo; // 'M' o 'F'
    char tipoSangre[6];
    
    // Datos de contacto
    char telefono[16];
    char direccion[101];
    char email[51];
    
    // Historial médico (array dinámico)
    HistorialMedico* historial;
    int cantidadConsultas;
    int capacidadHistorial; // Crece dinámicamente
    
    // Citas agendadas (array dinámico de IDs)
    int* citasAgendadas;
    int cantidadCitasAgendadas;
    int capacidadCitasAgendadas;
    
    // Información médica adicional (arrays fijos - strings)
    char alergias[501]; // Formato: "Alergia1, Alergia2"
    char observaciones[501];
    
    // Estado
    bool activo; // Activo/Inactivo
};

/**
 * @struct Doctor
 * @brief Representa a un doctor del hospital.
 */
struct Doctor {
    // Identificación y datos personales
    int id;
    char nombre[51];
    char apellido[51];
    char cedulaProfesional[21]; // DEBE SER ÚNICA
    
    // Datos profesionales
    char especialidad[51];
    int aniosExperiencia;
    float costoConsulta; // Mayor a 0
    char horarioAtencion[51];
    
    // Datos de contacto
    char telefono[16];
    char email[51];
    
    // Pacientes asignados (array dinámico de IDs)
    int* pacientesAsignados;
    int cantidadPacientesAsignados;
    int capacidadPacientesAsignados;
    
    // Citas agendadas (array dinámico de IDs)
    int* citasAgendadas;
    int cantidadCitasAgendadas;
    int capacidadCitasAgendadas;
    
    // Estado
    bool disponible; // Disponible para nuevos pacientes
};

/**
 * @struct Cita
 * @brief Representa una cita médica agendada.
 */
struct Cita {
    int idCita;
    int idPaciente;
    int idDoctor;
    char fecha[11]; // YYYY-MM-DD\0
    char hora[6];   // HH:MM\0
    char motivo[151];
    char estado[21]; // "Agendada", "Atendida", "Cancelada"
    char observaciones[201];
    bool atendida; // true si se generó historial
};

/**
 * @struct Hospital
 * @brief Estructura principal que contiene todos los datos del sistema.
 */
struct Hospital {
    // Información básica del hospital
    char nombre[101];
    char direccion[151];
    char telefono[16];
    
    // Arrays dinámicos de pacientes
    Paciente* pacientes;
    int cantidadPacientes;
    int capacidadPacientes;
    
    // Arrays dinámicos de doctores
    Doctor* doctores;
    int cantidadDoctores;
    int capacidadDoctores;
    
    // Arrays dinámicos de citas
    Cita* citas;
    int cantidadCitas;
    int capacidadCitas;
    
    // Contadores automáticos de IDs
    int siguienteIdPaciente; // Inicia en 1
    int siguienteIdDoctor;   // Inicia en 1
    int siguienteIdCita;     // Inicia en 1
    int siguienteIdConsulta; // Inicia en 1
};

// void buscarPacientePorId(Hospital* hospital, int id);
// =================================================================
// 5. FUNCIONES DE UTILIDADES
// =================================================================

// 5.3 Funciones de copia (Deep Copy)

/**
 * @brief Crea una copia profunda de un string.
 * @param origen String a copiar.
 * @return Puntero al nuevo string dinámico o nullptr si origen es nullptr.
 */
char* copiarString(const char* origen) {
    if (origen == nullptr) {
        return nullptr;
    }
    // Calcular longitud y crear nuevo array + 1 para el '\0'
    size_t longitud = strlen(origen);
    char* copia = new char[longitud + 1];
    // Copiar el contenido
    strcpy(copia, origen);
    return copia;
}

// 5.2 Validaciones

/**
 * @brief Convierte un string a minúsculas para comparaciones case-insensitive.
 * @param str String a convertir.
 * @return String convertido a minúsculas (memoria dinámica, debe liberarse).
 */
char* toLowerString(const char* str) {
    if (str == nullptr) return nullptr;
    size_t len = strlen(str);
    char* lower = new char[len + 1];
    for (size_t i = 0; i < len; ++i) {
        lower[i] = tolower(static_cast<unsigned char>(str[i]));
    }
    lower[len] = '\0';
    return lower;
}

/**
 * @brief Determina si un año es bisiesto.
 */
bool esBisiesto(int anio) {
    return (anio % 4 == 0 && anio % 100 != 0) || (anio % 400 == 0);
}

/**
 * @brief Valida el formato y la coherencia de una fecha (YYYY-MM-DD).
 */
bool validarFecha(const char* fecha) {
    if (fecha == nullptr || strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') {
        return false;
    }

    int anio = 0, mes = 0, dia = 0;
    
    // Leer año
    for (int i = 0; i < 4; ++i) {
        if (!isdigit(static_cast<unsigned char>(fecha[i]))) return false;
        anio = anio * 10 + (fecha[i] - '0');
    }
    // Leer mes
    for (int i = 5; i < 7; ++i) {
        if (!isdigit(static_cast<unsigned char>(fecha[i]))) return false;
        mes = mes * 10 + (fecha[i] - '0');
    }
    // Leer día
    for (int i = 8; i < 10; ++i) {
        if (!isdigit(static_cast<unsigned char>(fecha[i]))) return false;
        dia = dia * 10 + (fecha[i] - '0');
    }
    
    // Validar rangos
    if (anio < 1900 || mes < 1 || mes > 12 || dia < 1) {
        return false;
    }

    // Días por mes
    int diasEnMes[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (mes == 2 && esBisiesto(anio)) {
        diasEnMes[2] = 29;
    }

    return dia <= diasEnMes[mes];
}

/**
 * @brief Valida el formato y la coherencia de una hora (HH:MM).
 */
bool validarHora(const char* hora) {
    if (hora == nullptr || strlen(hora) != 5 || hora[2] != ':') {
        return false;
    }

    int h = 0, m = 0;
    
    // Leer hora
    for (int i = 0; i < 2; ++i) {
        if (!isdigit(static_cast<unsigned char>(hora[i]))) return false;
        h = h * 10 + (hora[i] - '0');
    }
    // Leer minutos
    for (int i = 3; i < 5; ++i) {
        if (!isdigit(static_cast<unsigned char>(hora[i]))) return false;
        m = m * 10 + (hora[i] - '0');
    }

    return (h >= 0 && h <= 23) && (m >= 0 && m <= 59);
}

/**
 * @brief Compara dos fechas (YYYY-MM-DD).
 * @return -1 si fecha1 < fecha2, 0 si iguales, 1 si fecha1 > fecha2.
 */
int compararFechas(const char* fecha1, const char* fecha2) {
    // strcmp es suficiente ya que el formato es YYYY-MM-DD
    return strcmp(fecha1, fecha2);
}

/**
 * @brief Validación básica de cédula: no vacía y dentro de longitud.
 */
bool validarCedula(const char* cedula) {
    return cedula != nullptr && strlen(cedula) > 0 && strlen(cedula) <= 20;
}

/**
 * @brief Validación básica de email: presencia de '@' y '.'.
 */
bool validarEmail(const char* email) {
    if (email == nullptr || strlen(email) > 50) return false;
    const char* at = strchr(email, '@');
    if (at == nullptr) return false; // No hay '@'
    const char* dot = strchr(at, '.');
    return dot != nullptr && dot != at + 1; // Hay '.' después del '@' y no inmediatamente después
}

// =================================================================
// 5.1 GESTIÓN DE MEMORIA Y REDIMENSIONAMIENTO (Array Genérico)
// =================================================================

/**
 * @brief Función genérica para redimensionar un array dinámico de cualquier tipo.
 * @tparam T Tipo de la estructura a redimensionar.
 * @param array Puntero al array actual (puntero a puntero).
 * @param cantidad Puntero a la cantidad actual de elementos.
 * @param capacidad Puntero a la capacidad actual del array.
 * @param nuevaCapacidad Nueva capacidad deseada.
 * @param copiarElementos Indica si se deben copiar los elementos existentes.
 * @note **Importante:** Para `HistorialMedico`, la copia se hace por valor. Para arrays de punteros (no aplica aquí), se necesitaría lógica adicional.
 */
template <typename T>
void redimensionarArray(T** array, int* cantidad, int* capacidad, int nuevaCapacidad, bool copiarElementos) {
    if (nuevaCapacidad <= *capacidad) return; // No redimensionar si no es necesario

    T* nuevoArray = new (std::nothrow) T[nuevaCapacidad];
    if (nuevoArray == nullptr) {
        cerr << "ERROR DE MEMORIA: No se pudo asignar espacio para el nuevo array." << endl;
        return;
    }

    if (copiarElementos && *cantidad > 0) {
        
        for (int i = 0; i < *cantidad; ++i) {
            nuevoArray[i] = (*array)[i]; // Copia por valor 
        }
    }

    // Libera la memoria del array antiguo
    if (*array != nullptr) {
        delete[] *array;
    }

    // Actualiza el puntero y la capacidad
    *array = nuevoArray;
    *capacidad = nuevaCapacidad;
}

// 5.1 Funciones de Redimensionamiento Específicas

void redimensionarArrayPacientes(Hospital* hospital) {
    int nuevaCapacidad = hospital->capacidadPacientes * 2;
    redimensionarArray(&hospital->pacientes, &hospital->cantidadPacientes, &hospital->capacidadPacientes, nuevaCapacidad, true);
}

void redimensionarArrayDoctores(Hospital* hospital) {
    int nuevaCapacidad = hospital->capacidadDoctores * 2;
    redimensionarArray(&hospital->doctores, &hospital->cantidadDoctores, &hospital->capacidadDoctores, nuevaCapacidad, true);
}

void redimensionarArrayCitas(Hospital* hospital) {
    int nuevaCapacidad = hospital->capacidadCitas * 2;
    redimensionarArray(&hospital->citas, &hospital->cantidadCitas, &hospital->capacidadCitas, nuevaCapacidad, true);
}

/**
 * @brief Redimensiona el array de HistorialMedico de un paciente.
 */
void redimensionarHistorial(Paciente* paciente) {
    int nuevaCapacidad = paciente->capacidadHistorial * 2;
    redimensionarArray(&paciente->historial, &paciente->cantidadConsultas, &paciente->capacidadHistorial, nuevaCapacidad, true);
}

/**
 * @brief Redimensiona el array de IDs (citas o pacientes) de un doctor o paciente.
 * @tparam T Tipo de puntero a array de IDs (int*).
 */
template <typename T>
void redimensionarArrayIds(T** arrayIds, int* cantidad, int* capacidad) {
    int nuevaCapacidad = (*capacidad) * 2;
    redimensionarArray(arrayIds, cantidad, capacidad, nuevaCapacidad, true);
}

// 5.1 Gestión de memoria (Inicialización y Destrucción)

/**
 * @brief Inicializa la estructura Hospital.
 */
Hospital* inicializarHospital(const char* nombre) {
    Hospital* hospital = new (std::nothrow) Hospital;
    if (hospital == nullptr) {
        cerr << "ERROR DE MEMORIA: No se pudo asignar la estructura Hospital." << endl;
        return nullptr;
    }

    // Información básica
    strncpy(hospital->nombre, nombre, 100);
    hospital->nombre[100] = '\0';
    strcpy(hospital->direccion, "N/A");
    strcpy(hospital->telefono, "N/A");

    // Arrays dinámicos de Pacientes
    hospital->cantidadPacientes = 0;
    hospital->capacidadPacientes = CAPACIDAD_INICIAL_HOSPITAL;
    hospital->pacientes = new (std::nothrow) Paciente[hospital->capacidadPacientes];
    if (hospital->pacientes == nullptr) {
        delete hospital;
        cerr << "ERROR DE MEMORIA: No se pudo asignar el array de Pacientes." << endl;
        return nullptr;
    }

    // Arrays dinámicos de Doctores
    hospital->cantidadDoctores = 0;
    hospital->capacidadDoctores = CAPACIDAD_INICIAL_HOSPITAL;
    hospital->doctores = new (std::nothrow) Doctor[hospital->capacidadDoctores];
    if (hospital->doctores == nullptr) {
        delete[] hospital->pacientes;
        delete hospital;
        cerr << "ERROR DE MEMORIA: No se pudo asignar el array de Doctores." << endl;
        return nullptr;
    }
    
    // Arrays dinámicos de Citas
    hospital->cantidadCitas = 0;
    hospital->capacidadCitas = CAPACIDAD_INICIAL_CITAS_HOSPITAL;
    hospital->citas = new (std::nothrow) Cita[hospital->capacidadCitas];
    if (hospital->citas == nullptr) {
        delete[] hospital->pacientes;
        delete[] hospital->doctores;
        delete hospital;
        cerr << "ERROR DE MEMORIA: No se pudo asignar el array de Citas." << endl;
        return nullptr;
    }

    // Contadores de IDs
    hospital->siguienteIdPaciente = 1;
    hospital->siguienteIdDoctor = 1;
    hospital->siguienteIdCita = 1;
    hospital->siguienteIdConsulta = 1;
    
    return hospital;
}

/**
 * @brief Libera TODA la memoria dinámica asociada al hospital.
 */
void destruirHospital(Hospital* hospital) {
    if (hospital == nullptr) return;

    // 1. Para cada paciente: liberar arrays internos
    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        delete[] hospital->pacientes[i].historial;
        delete[] hospital->pacientes[i].citasAgendadas;
    }
    // 2. Liberar array de pacientes
    delete[] hospital->pacientes;

    // 3. Para cada doctor: liberar arrays internos
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        delete[] hospital->doctores[i].pacientesAsignados;
        delete[] hospital->doctores[i].citasAgendadas;
    }
    // 4. Liberar array de doctores
    delete[] hospital->doctores;

    // 5. Liberar array de citas
    delete[] hospital->citas;

    // 6. Liberar estructura de Hospital
    delete hospital;
}


// =================================================================
// 3. MÓDULO DE GESTIÓN DE DOCTORES (Funciones Auxiliares)
// =================================================================


Doctor* buscarDoctorPorId(Hospital* hospital, int id) {
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        if (hospital->doctores[i].id == id) {
            return &hospital->doctores[i];
        }
    }
    return nullptr;
}


Doctor* buscarDoctorPorCedula(Hospital* hospital, const char* cedula) {
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        // Asume que la cédula profesional ya está saneada (mayúsculas/minúsculas)
        if (strcmp(hospital->doctores[i].cedulaProfesional, cedula) == 0) {
            return &hospital->doctores[i];
        }
    }
    return nullptr;
}


bool removerId(int* array, int* cantidad, int id) {
    if (array == nullptr || *cantidad == 0) return false;

    int indiceEncontrado = -1;
    for (int i = 0; i < *cantidad; ++i) {
        if (array[i] == id) {
            indiceEncontrado = i;
            break;
        }
    }

    if (indiceEncontrado != -1) {
        for (int i = indiceEncontrado; i < *cantidad - 1; ++i) {
            array[i] = array[i + 1];
        }
        (*cantidad)--;
        return true;
    }
    return false;
}

// 3.1 Funciones obligatorias

Doctor* crearDoctor(Hospital* hospital, const char* nombre, const char* apellido, const char* cedula,
                    const char* especialidad, int aniosExperiencia, float costoConsulta) {
    
    // 1. Validación de unicidad de cédula
    if (buscarDoctorPorCedula(hospital, cedula) != nullptr) {
        cout << "ERROR: La cedula profesional " << cedula << " ya esta registrada." << endl;
        return nullptr;
    }
    
    // 2. Redimensionar si el array está lleno
    if (hospital->cantidadDoctores >= hospital->capacidadDoctores) {
        redimensionarArrayDoctores(hospital);
        if (hospital->cantidadDoctores >= hospital->capacidadDoctores) { // Error de redimensionamiento
            cerr << "ERROR: Fallo el redimensionamiento de doctores." << endl;
            return nullptr;
        }
    }
    
    // 3. Asignar ID y obtener puntero a la nueva posición
    Doctor* nuevoDoctor = &hospital->doctores[hospital->cantidadDoctores];
    nuevoDoctor->id = hospital->siguienteIdDoctor++;
    
    // 4. Copiar datos y validaciones
    strncpy(nuevoDoctor->nombre, nombre, 50); nuevoDoctor->nombre[50] = '\0';
    strncpy(nuevoDoctor->apellido, apellido, 50); nuevoDoctor->apellido[50] = '\0';
    strncpy(nuevoDoctor->cedulaProfesional, cedula, 20); nuevoDoctor->cedulaProfesional[20] = '\0';
    strncpy(nuevoDoctor->especialidad, especialidad, 50); nuevoDoctor->especialidad[50] = '\0';

    nuevoDoctor->aniosExperiencia = aniosExperiencia < 0 ? 0 : aniosExperiencia;
    nuevoDoctor->costoConsulta = costoConsulta <= 0 ? 1.0f : costoConsulta; // Asegurar > 0

    // 5. Inicializar arrays dinámicos
    nuevoDoctor->cantidadPacientesAsignados = 0;
    nuevoDoctor->capacidadPacientesAsignados = CAPACIDAD_INICIAL_PACIENTES_DOCTOR;
    nuevoDoctor->pacientesAsignados = new (std::nothrow) int[CAPACIDAD_INICIAL_PACIENTES_DOCTOR];

    nuevoDoctor->cantidadCitasAgendadas = 0;
    nuevoDoctor->capacidadCitasAgendadas = CAPACIDAD_INICIAL_CITAS_DOCTOR;
    nuevoDoctor->citasAgendadas = new (std::nothrow) int[CAPACIDAD_INICIAL_CITAS_DOCTOR];
    
    // 6. Otros datos y estado
    strcpy(nuevoDoctor->telefono, "N/A");
    strcpy(nuevoDoctor->email, "N/A");
    strcpy(nuevoDoctor->horarioAtencion, "N/A");
    nuevoDoctor->disponible = true;

    // 7. Incrementar contador del hospital
    hospital->cantidadDoctores++;
    
    return nuevoDoctor;
}


Doctor** buscarDoctoresPorEspecialidad(Hospital* hospital, const char* especialidad, int* cantidad) {
    *cantidad = 0;
    if (hospital == nullptr || especialidad == nullptr) return nullptr;

    char* especialidadLower = toLowerString(especialidad);
    if (especialidadLower == nullptr) return nullptr;
    
    // Paso 1: Contar cuántos doctores cumplen
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        char* docEspecialidadLower = toLowerString(hospital->doctores[i].especialidad);
        if (docEspecialidadLower != nullptr && strstr(docEspecialidadLower, especialidadLower) != nullptr) {
            (*cantidad)++;
        }
        delete[] docEspecialidadLower;
    }
    
    delete[] especialidadLower;

    // Paso 2: Si cantidad == 0, retornar nullptr
    if (*cantidad == 0) return nullptr;

    // Paso 3: Crear array dinámico de punteros
    Doctor** resultados = new (std::nothrow) Doctor*[*cantidad];
    if (resultados == nullptr) {
        *cantidad = 0;
        return nullptr;
    }

    // Paso 4: Llenar el array con punteros
    int indiceResultados = 0;
    especialidadLower = toLowerString(especialidad); // Recalcular
    if (especialidadLower == nullptr) return nullptr;
    
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        char* docEspecialidadLower = toLowerString(hospital->doctores[i].especialidad);
        if (docEspecialidadLower != nullptr && strstr(docEspecialidadLower, especialidadLower) != nullptr) {
            resultados[indiceResultados++] = &hospital->doctores[i];
        }
        delete[] docEspecialidadLower;
    }

    delete[] especialidadLower;

    return resultados;
}


bool asignarPacienteADoctor(Doctor* doctor, int idPaciente) {
    if (doctor == nullptr || idPaciente <= 0) return false;

    // Verificar si ya está asignado (evitar duplicados)
    for (int i = 0; i < doctor->cantidadPacientesAsignados; ++i) {
        if (doctor->pacientesAsignados[i] == idPaciente) {
            return false; // Ya asignado
        }
    }

    // Redimensionar si es necesario
    if (doctor->cantidadPacientesAsignados >= doctor->capacidadPacientesAsignados) {
        redimensionarArrayIds(&doctor->pacientesAsignados, &doctor->cantidadPacientesAsignados, &doctor->capacidadPacientesAsignados);
        if (doctor->cantidadPacientesAsignados >= doctor->capacidadPacientesAsignados) { // Error de redimensionamiento
            cerr << "ERROR: Fallo el redimensionamiento de pacientes asignados." << endl;
            return false;
        }
    }

    // Agregar ID
    doctor->pacientesAsignados[doctor->cantidadPacientesAsignados++] = idPaciente;
    return true;
}


bool removerPacienteDeDoctor(Doctor* doctor, int idPaciente) {
    if (doctor == nullptr || idPaciente <= 0) return false;

    return removerId(doctor->pacientesAsignados, &doctor->cantidadPacientesAsignados, idPaciente);
}

Paciente* buscarPacientePorId(Hospital* hospital, int id) {
    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        if (hospital->pacientes[i].id == id) {
            return &hospital->pacientes[i];
        }
    }
    return nullptr;
}

void listarPacientesDeDoctor(Hospital* hospital, int idDoctor) {
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (doctor == nullptr) {
        cout << "ERROR: Doctor con ID " << idDoctor << " no encontrado." << endl;
        return;
    }

    cout << "\n--- Pacientes Asignados a Dr. " << doctor->nombre << " " << doctor->apellido << " (ID: " << idDoctor << ") ---" << endl;
    if (doctor->cantidadPacientesAsignados == 0) {
        cout << "No tiene pacientes asignados actualmente." << endl;
        return;
    }

    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;
    cout << "| " << left << setw(5) << "ID"
         << "| " << setw(40) << "Nombre Completo"
         << "| " << setw(15) << "Cedula"
         << "|" << endl;
    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;

    for (int i = 0; i < doctor->cantidadPacientesAsignados; ++i) {
        Paciente* p = buscarPacientePorId(hospital, doctor->pacientesAsignados[i]);
        if (p != nullptr) {
            cout << "| " << left << setw(5) << p->id
                 << "| " << setw(40) << (string(p->nombre) + " " + string(p->apellido)).c_str()
                 << "| " << setw(15) << p->cedula
                 << "|" << endl;
        }
    }
    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;
}


void listarDoctores(Hospital* hospital) {
    cout << "\n--- LISTADO DE DOCTORES ---" << endl;
    if (hospital->cantidadDoctores == 0) {
        cout << "No hay doctores registrados." << endl;
        return;
    }

    cout << setfill('-') << setw(100) << "" << setfill(' ') << endl;
    cout << "| " << left << setw(5) << "ID"
         << "| " << setw(30) << "Nombre"
         << "| " << setw(15) << "Especialidad"
         << "| " << right << setw(10) << "Costo Cons."
         << "| " << setw(10) << "Pacientes"
         << "| " << left << setw(10) << "Disponible"
         << "|" << endl;
    cout << setfill('-') << setw(100) << "" << setfill(' ') << endl;

    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        Doctor& d = hospital->doctores[i];
        cout << "| " << left << setw(5) << d.id
             << "| " << setw(30) << (string(d.nombre) + " " + string(d.apellido)).c_str()
             << "| " << setw(15) << d.especialidad
             << "| " << right << setw(10) << fixed << setprecision(2) << d.costoConsulta
             << "| " << setw(10) << d.cantidadPacientesAsignados
             << "| " << left << setw(10) << (d.disponible ? "Si" : "No")
             << "|" << endl;
    }
    cout << setfill('-') << setw(100) << "" << setfill(' ') << endl;
}

bool eliminarDoctor(Hospital* hospital, int id) {
    int indiceEncontrado = -1;
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        if (hospital->doctores[i].id == id) {
            indiceEncontrado = i;
            break;
        }
    }

    if (indiceEncontrado == -1) {
        return false;
    }

    Doctor& doctorAEliminar = hospital->doctores[indiceEncontrado];

    // 1. Liberar memoria de arrays dinámicos
    delete[] doctorAEliminar.pacientesAsignados;
    delete[] doctorAEliminar.citasAgendadas;
    
    // 2. Cancelar citas asociadas (Nota: Se deben buscar las citas por ID en el array de citas del Hospital)

    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (hospital->citas[i].idDoctor == id && strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            strcpy(hospital->citas[i].estado, "Cancelada");
            
        }
    }
    
    // 3. Compactar array de doctores
    for (int i = indiceEncontrado; i < hospital->cantidadDoctores - 1; ++i) {
        hospital->doctores[i] = hospital->doctores[i + 1];
    }
    hospital->cantidadDoctores--;

    return true;
}


// =================================================================
// 2. MÓDULO DE GESTIÓN DE PACIENTES (Funciones Auxiliares)
// =================================================================


Paciente* buscarPacientePorCedula(Hospital* hospital, const char* cedula) {
    if (hospital == nullptr || cedula == nullptr) return nullptr;

    char* cedulaLower = toLowerString(cedula);
    if (cedulaLower == nullptr) return nullptr;

    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        char* pacCedulaLower = toLowerString(hospital->pacientes[i].cedula);
        if (pacCedulaLower != nullptr && strcmp(pacCedulaLower, cedulaLower) == 0) {
            delete[] cedulaLower;
            delete[] pacCedulaLower;
            return &hospital->pacientes[i];
        }
        delete[] pacCedulaLower;
    }

    delete[] cedulaLower;
    return nullptr;
}


// 2.1 Funciones CRUD obligatorias


Paciente* crearPaciente(Hospital* hospital, const char* nombre, const char* apellido,
                        const char* cedula, int edad, char sexo) {
    
    // 1. Verificar unicidad de cédula
    if (buscarPacientePorCedula(hospital, cedula) != nullptr) {
        cout << "ERROR: La cedula " << cedula << " ya esta registrada." << endl;
        return nullptr;
    }
    
    // 2. Redimensionar si el array de pacientes está lleno
    if (hospital->cantidadPacientes >= hospital->capacidadPacientes) {
        redimensionarArrayPacientes(hospital);
        if (hospital->cantidadPacientes >= hospital->capacidadPacientes) { 
            cerr << "ERROR: Fallo el redimensionamiento de pacientes." << endl;
            return nullptr;
        }
    }
    
    // 3. Asignar ID y obtener puntero a la nueva posición
    Paciente* nuevoPaciente = &hospital->pacientes[hospital->cantidadPacientes];
    nuevoPaciente->id = hospital->siguienteIdPaciente++;
    
    // 4. Copiar datos y validaciones
    strncpy(nuevoPaciente->nombre, nombre, 50); nuevoPaciente->nombre[50] = '\0';
    strncpy(nuevoPaciente->apellido, apellido, 50); nuevoPaciente->apellido[50] = '\0';
    strncpy(nuevoPaciente->cedula, cedula, 20); nuevoPaciente->cedula[20] = '\0';

    nuevoPaciente->edad = (edad >= 0 && edad <= 120) ? edad : 0;
    nuevoPaciente->sexo = (toupper(sexo) == 'M' || toupper(sexo) == 'F') ? toupper(sexo) : 'N'; 
    
    // Inicializar arrays de alergias y observaciones como strings vacíos
    strcpy(nuevoPaciente->tipoSangre, "N/A");
    strcpy(nuevoPaciente->telefono, "N/A");
    strcpy(nuevoPaciente->direccion, "N/A");
    strcpy(nuevoPaciente->email, "N/A");
    strcpy(nuevoPaciente->alergias, "");
    strcpy(nuevoPaciente->observaciones, "");
    
    // 5. Inicializar arrays dinámicos del paciente
    
    // Historial médico
    nuevoPaciente->cantidadConsultas = 0;
    nuevoPaciente->capacidadHistorial = CAPACIDAD_INICIAL_HISTORIAL;
    nuevoPaciente->historial = new (std::nothrow) HistorialMedico[CAPACIDAD_INICIAL_HISTORIAL];

    // Citas agendadas (IDs)
    nuevoPaciente->cantidadCitasAgendadas = 0;
    nuevoPaciente->capacidadCitasAgendadas = CAPACIDAD_INICIAL_CITAS_PACIENTE;
    nuevoPaciente->citasAgendadas = new (std::nothrow) int[CAPACIDAD_INICIAL_CITAS_PACIENTE];
    
    // 6. Estado
    nuevoPaciente->activo = true;

    // 7. Incrementar contador del hospital
    hospital->cantidadPacientes++;
    
    return nuevoPaciente;
}


Paciente** buscarPacientesPorNombre(Hospital* hospital, const char* nombre, int* cantidad) {
    *cantidad = 0;
    if (hospital == nullptr || nombre == nullptr) return nullptr;

    char* nombreLower = toLowerString(nombre);
    if (nombreLower == nullptr) return nullptr;
    
    // Paso 1: Contar cuántos pacientes cumplen
    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        char* nombreCompleto = new char[strlen(hospital->pacientes[i].nombre) + strlen(hospital->pacientes[i].apellido) + 2];
        strcpy(nombreCompleto, hospital->pacientes[i].nombre);
        strcat(nombreCompleto, " ");
        strcat(nombreCompleto, hospital->pacientes[i].apellido);

        char* nombreCompletoLower = toLowerString(nombreCompleto);
        if (nombreCompletoLower != nullptr && strstr(nombreCompletoLower, nombreLower) != nullptr) {
            (*cantidad)++;
        }
        delete[] nombreCompleto;
        delete[] nombreCompletoLower;
    }
    
    delete[] nombreLower;

    if (*cantidad == 0) return nullptr;

    // Paso 2: Crear array dinámico de punteros
    Paciente** resultados = new (std::nothrow) Paciente*[*cantidad];
    if (resultados == nullptr) {
        *cantidad = 0;
        return nullptr;
    }

    // Paso 3: Llenar el array con punteros
    int indiceResultados = 0;
    nombreLower = toLowerString(nombre); // Recalcular
    if (nombreLower == nullptr) {
        delete[] resultados;
        return nullptr;
    }
    
    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        char* nombreCompleto = new char[strlen(hospital->pacientes[i].nombre) + strlen(hospital->pacientes[i].apellido) + 2];
        strcpy(nombreCompleto, hospital->pacientes[i].nombre);
        strcat(nombreCompleto, " ");
        strcat(nombreCompleto, hospital->pacientes[i].apellido);
        
        char* nombreCompletoLower = toLowerString(nombreCompleto);
        if (nombreCompletoLower != nullptr && strstr(nombreCompletoLower, nombreLower) != nullptr) {
            resultados[indiceResultados++] = &hospital->pacientes[i];
        }
        delete[] nombreCompleto;
        delete[] nombreCompletoLower;
    }

    delete[] nombreLower;
    
    return resultados;
}


bool actualizarPaciente(Hospital* hospital, int id) {
    Paciente* paciente = buscarPacientePorId(hospital, id);
    if (paciente == nullptr) {
        cout << "ERROR: Paciente con ID " << id << " no encontrado." << endl;
        return false;
    }

    int opcion;
    char buffer[501];
    
    cout << "\n--- ACTUALIZAR DATOS DE PACIENTE (ID: " << id << ") ---" << endl;
    cout << "1. Nombre actual: " << paciente->nombre << endl;
    cout << "2. Apellido actual: " << paciente->apellido << endl;
    cout << "3. Email actual: " << paciente->email << endl;
    cout << "4. Telefono actual: " << paciente->telefono << endl;
    cout << "5. Direccion actual: " << paciente->direccion << endl;
    cout << "0. Cancelar" << endl;
    cout << "Seleccione el campo a modificar: ";
    
    // Se asume la lectura de la opción
    if (!(cin >> opcion)) {
        cin.clear(); cin.ignore(10000, '\n');
        cout << "Opcion invalida." << endl;
        return false;
    }
    cin.ignore(10000, '\n');
    
    switch (opcion) {
        case 1:
            cout << "Ingrese nuevo Nombre (max 50): ";
            cin.getline(buffer, 51);
            strncpy(paciente->nombre, buffer, 50); paciente->nombre[50] = '\0';
            cout << "Nombre actualizado." << endl;
            break;
        case 2:
            cout << "Ingrese nuevo Apellido (max 50): ";
            cin.getline(buffer, 51);
            strncpy(paciente->apellido, buffer, 50); paciente->apellido[50] = '\0';
            cout << "Apellido actualizado." << endl;
            break;
        case 3:
            cout << "Ingrese nuevo Email (máx 50): ";
            cin.getline(buffer, 51);
            if (validarEmail(buffer)) {
                strncpy(paciente->email, buffer, 50); paciente->email[50] = '\0';
                cout << "Email actualizado." << endl;
            } else {
                cout << "ERROR: Formato de Email invalido o excede longitud." << endl;
                return false;
            }
            break;
        case 4:
            cout << "Ingrese nuevo Telefono (max 15): ";
            cin.getline(buffer, 16);
            strncpy(paciente->telefono, buffer, 15); paciente->telefono[15] = '\0';
            cout << "Teléfono actualizado." << endl;
            break;
        case 5:
            cout << "Ingrese nueva Direccion (max 100): ";
            cin.getline(buffer, 101);
            strncpy(paciente->direccion, buffer, 100); paciente->direccion[100] = '\0';
            cout << "Direccion actualizada." << endl;
            break;
        case 0:
            cout << "Actualizacion cancelada." << endl;
            return false;
        default:
            cout << "Opcion no reconocida." << endl;
            return false;
    }

    return true;
}


bool eliminarPaciente(Hospital* hospital, int id) {
    int indiceEncontrado = -1;
    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        if (hospital->pacientes[i].id == id) {
            indiceEncontrado = i;
            break;
        }
    }

    if (indiceEncontrado == -1) {
        return false;
    }

    Paciente& pacienteAEliminar = hospital->pacientes[indiceEncontrado];

    // 1. Liberar TODA la memoria asociada al paciente
    delete[] pacienteAEliminar.historial;
    delete[] pacienteAEliminar.citasAgendadas;
    
    // 2. Eliminar o cancelar todas las citas asociadas
    // Se cancelan las citas en el array principal del hospital
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (hospital->citas[i].idPaciente == id && strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            strcpy(hospital->citas[i].estado, "Cancelada");
            // Nota: Aquí no se remueve la cita del array principal, solo se marca como "Cancelada".
            // Una implementación robusta requeriría un mecanismo de 'limpieza' de citas canceladas.
        }
    }
    
    // 3. Remover el paciente de las listas de doctores asignados
    for (int i = 0; i < hospital->cantidadDoctores; ++i) {
        removerPacienteDeDoctor(&hospital->doctores[i], id);
        // También se debería remover el ID de cita de los doctores, pero esto se hace al cancelar la cita (atenderCita/cancelarCita)
    }
    
    // 4. Compactar el array de pacientes
    for (int i = indiceEncontrado; i < hospital->cantidadPacientes - 1; ++i) {
       
        hospital->pacientes[i] = hospital->pacientes[i + 1];
    }
    hospital->cantidadPacientes--;

    return true;
}


void listarPacientes(Hospital* hospital) {
    cout << "\n--- LISTADO DE PACIENTES ---" << endl;
    if (hospital->cantidadPacientes == 0) {
        cout << "No hay pacientes registrados." << endl;
        return;
    }

    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;
    cout << "| " << left << setw(5) << "ID"
         << "| " << setw(30) << "Nombre Completo"
         << "| " << setw(15) << "Cedula"
         << "| " << right << setw(5) << "Edad"
         << "| " << setw(5) << "Sexo"
         << "| " << setw(10) << "Consultas"
         << "| " << left << setw(10) << "Activo"
         << "|" << endl;
    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;

    for (int i = 0; i < hospital->cantidadPacientes; ++i) {
        Paciente& p = hospital->pacientes[i];
        cout << "| " << left << setw(5) << p.id
             << "| " << setw(30) << (string(p.nombre) + " " + string(p.apellido)).c_str()
             << "| " << setw(15) << p.cedula
             << "| " << right << setw(5) << p.edad
             << "| " << setw(5) << p.sexo
             << "| " << setw(10) << p.cantidadConsultas
             << "| " << left << setw(10) << (p.activo ? "Si" : "No")
             << "|" << endl;
    }
    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;
}


// 2.2 Funciones de historial médico (arrays dinámicos)


void agregarConsultaAlHistorial(Paciente* paciente, HistorialMedico consulta) {
    if (paciente == nullptr) return;

    // 1. Verificar si el array está lleno
    if (paciente->cantidadConsultas >= paciente->capacidadHistorial) {
        
        // 2. Redimensionar (duplicar capacidad)
        int nuevaCapacidad = paciente->capacidadHistorial * 2;
        HistorialMedico* nuevoHistorial = new (std::nothrow) HistorialMedico[nuevaCapacidad];
        if (nuevoHistorial == nullptr) {
            cerr << "ERROR DE MEMORIA: Fallo el redimensionamiento del historial." << endl;
            return;
        }

        // Copiar todos los elementos del array antiguo al nuevo
        for (int i = 0; i < paciente->cantidadConsultas; ++i) {
            nuevoHistorial[i] = paciente->historial[i]; // Copia por valor
        }

        // Liberar memoria del array antiguo
        delete[] paciente->historial;
        
        // Actualizar el puntero y la capacidad
        paciente->historial = nuevoHistorial;
        paciente->capacidadHistorial = nuevaCapacidad;
    }

    // 3. Agregar la nueva consulta en la posición actual
    paciente->historial[paciente->cantidadConsultas] = consulta;

    // 4. Incrementar cantidadConsultas
    paciente->cantidadConsultas++;
}


HistorialMedico* obtenerHistorialCompleto(Paciente* paciente, int* cantidad) {
    if (paciente == nullptr) {
        *cantidad = 0;
        return nullptr;
    }
    *cantidad = paciente->cantidadConsultas;
    return paciente->historial; // Retorna el puntero original
}


void mostrarHistorialMedico(Hospital* hospital, Paciente* paciente) {
    if (paciente == nullptr) {
        cout << "ERROR: Paciente no valido." << endl;
        return;
    }

    cout << "\n--- HISTORIAL MÉDICO DE " << paciente->nombre << " " << paciente->apellido << " (ID: " << paciente->id << ") ---" << endl;
    if (paciente->cantidadConsultas == 0) {
        cout << "El paciente no tiene consultas registradas." << endl;
        return;
    }

    cout << setfill('-') << setw(130) << "" << setfill(' ') << endl;
    cout << "| " << left << setw(5) << "ID"
         << "| " << setw(10) << "Fecha"
         << "| " << setw(5) << "Hora"
         << "| " << setw(15) << "Costo"
         << "| " << setw(30) << "Doctor"
         << "| " << setw(50) << "Diagnostico (Inicio)"
         << "|" << endl;
    cout << setfill('-') << setw(130) << "" << setfill(' ') << endl;

    for (int i = 0; i < paciente->cantidadConsultas; ++i) {
        HistorialMedico& h = paciente->historial[i];
        Doctor* doctor = buscarDoctorPorId(hospital, h.idDoctor);
        string nombreDoctor = (doctor != nullptr) ? (string(doctor->nombre) + " " + string(doctor->apellido)) : "Doctor Eliminado";
        
        // Mostrar solo los primeros 47 caracteres del diagnóstico
        char diagnosticoCorto[51];
        strncpy(diagnosticoCorto, h.diagnostico, 47);
        diagnosticoCorto[47] = '\0';
        if (strlen(h.diagnostico) > 47) {
            strcat(diagnosticoCorto, "...");
        }

        cout << "| " << left << setw(5) << h.idConsulta
             << "| " << setw(10) << h.fecha
             << "| " << setw(5) << h.hora
             << "| " << right << setw(15) << fixed << setprecision(2) << h.costoConsulta
             << "| " << left << setw(30) << nombreDoctor.c_str()
             << "| " << setw(50) << diagnosticoCorto
             << "|" << endl;
    }
    cout << setfill('-') << setw(130) << "" << setfill(' ') << endl;
}


HistorialMedico* obtenerUltimaConsulta(Paciente* paciente) {
    if (paciente == nullptr || paciente->cantidadConsultas == 0) {
        return nullptr;
    }

    return &paciente->historial[paciente->cantidadConsultas - 1];
}


// =================================================================
// 4. MÓDULO DE GESTIÓN DE CITAS (Funciones Auxiliares)
// =================================================================


Cita* buscarCitaPorId(Hospital* hospital, int idCita) {
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (hospital->citas[i].idCita == idCita) {
            return &hospital->citas[i];
        }
    }
    return nullptr;
}


// 4.1 Funciones obligatorias

bool verificarDisponibilidad(Hospital* hospital, int idDoctor, const char* fecha, const char* hora) {
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);
    if (doctor == nullptr) return false; // Doctor no existe

    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        Cita& cita = hospital->citas[i];
        // Solo importa si es una cita agendada (no cancelada ni atendida)
        if (cita.idDoctor == idDoctor && strcmp(cita.estado, "Agendada") == 0) {
            if (strcmp(cita.fecha, fecha) == 0 && strcmp(cita.hora, hora) == 0) {
                return false; // No disponible
            }
        }
    }
    return true; // Disponible
}


Cita* agendarCita(Hospital* hospital, int idPaciente, int idDoctor,
                    const char* fecha, const char* hora, const char* motivo) {
    
    Paciente* paciente = buscarPacientePorId(hospital, idPaciente);
    Doctor* doctor = buscarDoctorPorId(hospital, idDoctor);

    // 1. Validaciones obligatorias
    if (paciente == nullptr) {
        cout << "ERROR: Paciente con ID " << idPaciente << " no encontrado." << endl;
        return nullptr;
    }
    if (doctor == nullptr) {
        cout << "ERROR: Doctor con ID " << idDoctor << " no encontrado." << endl;
        return nullptr;
    }
    if (!validarFecha(fecha)) {
        cout << "ERROR: Formato de fecha (YYYY-MM-DD) invalido." << endl;
        return nullptr;
    }
    if (!validarHora(hora)) {
        cout << "ERROR: Formato de hora (HH:MM) invalido." << endl;
        return nullptr;
    }
    if (!verificarDisponibilidad(hospital, idDoctor, fecha, hora)) {
        cout << "ERROR: El doctor ya tiene una cita agendada para esa fecha y hora." << endl;
        return nullptr;
    }
    
    // 2. Redimensionar si el array de citas está lleno
    if (hospital->cantidadCitas >= hospital->capacidadCitas) {
        redimensionarArrayCitas(hospital);
        if (hospital->cantidadCitas >= hospital->capacidadCitas) { 
            cerr << "ERROR: Fallo el redimensionamiento de citas." << endl;
            return nullptr;
        }
    }
    
    // 3. Crear estructura Cita
    Cita* nuevaCita = &hospital->citas[hospital->cantidadCitas];
    nuevaCita->idCita = hospital->siguienteIdCita++;
    nuevaCita->idPaciente = idPaciente;
    nuevaCita->idDoctor = idDoctor;
    strncpy(nuevaCita->fecha, fecha, 10); nuevaCita->fecha[10] = '\0';
    strncpy(nuevaCita->hora, hora, 5); nuevaCita->hora[5] = '\0';
    strncpy(nuevaCita->motivo, motivo, 150); nuevaCita->motivo[150] = '\0';
    
    // Estado inicial
    strcpy(nuevaCita->estado, "Agendada");
    nuevaCita->atendida = false;
    strcpy(nuevaCita->observaciones, "");
    
    // 4. Agregar ID de cita a los arrays del paciente y doctor
    
    // Paciente
    if (paciente->cantidadCitasAgendadas >= paciente->capacidadCitasAgendadas) {
        redimensionarArrayIds(&paciente->citasAgendadas, &paciente->cantidadCitasAgendadas, &paciente->capacidadCitasAgendadas);
    }
    if (paciente->cantidadCitasAgendadas < paciente->capacidadCitasAgendadas) {
        paciente->citasAgendadas[paciente->cantidadCitasAgendadas++] = nuevaCita->idCita;
    } else {
        cerr << "ADVERTENCIA: Fallo al agregar cita a paciente (ID " << idPaciente << ")." << endl;
    }

    // Doctor
    if (doctor->cantidadCitasAgendadas >= doctor->capacidadCitasAgendadas) {
        redimensionarArrayIds(&doctor->citasAgendadas, &doctor->cantidadCitasAgendadas, &doctor->capacidadCitasAgendadas);
    }
    if (doctor->cantidadCitasAgendadas < doctor->capacidadCitasAgendadas) {
        doctor->citasAgendadas[doctor->cantidadCitasAgendadas++] = nuevaCita->idCita;
    } else {
        cerr << "ADVERTENCIA: Fallo al agregar cita a doctor (ID " << idDoctor << ")." << endl;
    }

    // 5. Incrementar contador principal
    hospital->cantidadCitas++;
    
    return nuevaCita;
}


bool cancelarCita(Hospital* hospital, int idCita) {
    Cita* cita = buscarCitaPorId(hospital, idCita);
    if (cita == nullptr || strcmp(cita->estado, "Cancelada") == 0 || strcmp(cita->estado, "Atendida") == 0) {
        return false;
    }

    // 1. Cambiar estado a "Cancelada"
    strcpy(cita->estado, "Cancelada");

    // 2. Remover de arrays de paciente y doctor
    Paciente* paciente = buscarPacientePorId(hospital, cita->idPaciente);
    Doctor* doctor = buscarDoctorPorId(hospital, cita->idDoctor);

    if (paciente != nullptr) {
        removerId(paciente->citasAgendadas, &paciente->cantidadCitasAgendadas, idCita);
    }
    if (doctor != nullptr) {
        removerId(doctor->citasAgendadas, &doctor->cantidadCitasAgendadas, idCita);
    }

    return true;
}


bool atenderCita(Hospital* hospital, int idCita, const char* diagnostico,
                 const char* tratamiento, const char* medicamentos) {
    
    // 1. Buscar la cita por ID
    Cita* cita = buscarCitaPorId(hospital, idCita);
    if (cita == nullptr) return false;

    // 2. Verificar que esté en estado "Agendada"
    if (strcmp(cita->estado, "Agendada") != 0) {
        cout << "ERROR: La cita no está en estado 'Agendada' (Estado actual: " << cita->estado << ")." << endl;
        return false;
    }

    // 3. Cambiar estado a "Atendida" y atendida = true
    strcpy(cita->estado, "Atendida");
    cita->atendida = true;

    Paciente* paciente = buscarPacientePorId(hospital, cita->idPaciente);
    Doctor* doctor = buscarDoctorPorId(hospital, cita->idDoctor);

    if (paciente == nullptr || doctor == nullptr) {
        cerr << "ADVERTENCIA: Paciente o Doctor de la cita ya no existen." << endl;
        // La cita se marca como atendida, pero no se genera historial. Se debe permitir al usuario corregir.
        return false;
    }

    // 4. Crear estructura HistorialMedico
    HistorialMedico nuevaConsulta;
    nuevaConsulta.idConsulta = hospital->siguienteIdConsulta++;
    strncpy(nuevaConsulta.fecha, cita->fecha, 10);
    strncpy(nuevaConsulta.hora, cita->hora, 5);
    strncpy(nuevaConsulta.diagnostico, diagnostico, 200); nuevaConsulta.diagnostico[200] = '\0';
    strncpy(nuevaConsulta.tratamiento, tratamiento, 200); nuevaConsulta.tratamiento[200] = '\0';
    strncpy(nuevaConsulta.medicamentos, medicamentos, 150); nuevaConsulta.medicamentos[150] = '\0';
    nuevaConsulta.idDoctor = cita->idDoctor;
    nuevaConsulta.costoConsulta = doctor->costoConsulta; // Obtener de doctor->costoConsulta

    // 5. Agregar al historial del paciente
    agregarConsultaAlHistorial(paciente, nuevaConsulta);
    
    // 6. Remover el ID de cita de los arrays de citas agendadas (ahora es atendida)
    removerId(paciente->citasAgendadas, &paciente->cantidadCitasAgendadas, idCita);
    removerId(doctor->citasAgendadas, &doctor->cantidadCitasAgendadas, idCita);

    // 7. Incrementar contador de ID de consulta (ya hecho arriba)

    return true;
}


Cita** obtenerCitasPorEntidadId(Hospital* hospital, int idEntidad, bool esPaciente, int* cantidad) {
    *cantidad = 0;
    if (hospital == nullptr) return nullptr;

    // Paso 1: Contar
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        Cita& c = hospital->citas[i];
        if (esPaciente) {
            if (c.idPaciente == idEntidad) {
                (*cantidad)++;
            }
        } else { // es Doctor
            if (c.idDoctor == idEntidad) {
                (*cantidad)++;
            }
        }
    }

    if (*cantidad == 0) return nullptr;

    // Paso 2: Crear array dinámico de punteros
    Cita** resultados = new (std::nothrow) Cita*[*cantidad];
    if (resultados == nullptr) {
        *cantidad = 0;
        return nullptr;
    }

    // Paso 3: Llenar el array
    int indiceResultados = 0;
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        Cita& c = hospital->citas[i];
        if ((esPaciente && c.idPaciente == idEntidad) || (!esPaciente && c.idDoctor == idEntidad)) {
            resultados[indiceResultados++] = &hospital->citas[i];
        }
    }

    return resultados;
}


Cita** obtenerCitasDePaciente(Hospital* hospital, int idPaciente, int* cantidad) {
    return obtenerCitasPorEntidadId(hospital, idPaciente, true, cantidad);
}


Cita** obtenerCitasDeDoctor(Hospital* hospital, int idDoctor, int* cantidad) {
    return obtenerCitasPorEntidadId(hospital, idDoctor, false, cantidad);
}


Cita** obtenerCitasPorFecha(Hospital* hospital, const char* fecha, int* cantidad) {
    *cantidad = 0;
    if (hospital == nullptr || !validarFecha(fecha)) return nullptr;

    // Paso 1: Contar
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (strcmp(hospital->citas[i].fecha, fecha) == 0) {
            (*cantidad)++;
        }
    }

    if (*cantidad == 0) return nullptr;

    // Paso 2: Crear array dinámico de punteros
    Cita** resultados = new (std::nothrow) Cita*[*cantidad];
    if (resultados == nullptr) {
        *cantidad = 0;
        return nullptr;
    }

    // Paso 3: Llenar el array
    int indiceResultados = 0;
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (strcmp(hospital->citas[i].fecha, fecha) == 0) {
            resultados[indiceResultados++] = &hospital->citas[i];
        }
    }

    return resultados;
}


void mostrarListaCitas(Hospital* hospital, Cita** citas, int cantidad, const char* titulo) {
    cout << "\n--- " << titulo << " ---" << endl;
    if (cantidad == 0) {
        cout << "No se encontraron citas." << endl;
        return;
    }

    cout << setfill('-') << setw(140) << "" << setfill(' ') << endl;
    cout << "| " << left << setw(5) << "ID"
         << "| " << setw(10) << "Fecha"
         << "| " << setw(5) << "Hora"
         << "| " << setw(30) << "Paciente"
         << "| " << setw(30) << "Doctor"
         << "| " << setw(20) << "Motivo"
         << "| " << setw(10) << "Estado"
         << "|" << endl;
    cout << setfill('-') << setw(140) << "" << setfill(' ') << endl;

    for (int i = 0; i < cantidad; ++i) {
        Cita* c = citas[i];
        Paciente* p = buscarPacientePorId(hospital, c->idPaciente);
        Doctor* d = buscarDoctorPorId(hospital, c->idDoctor);

        string nombrePaciente = (p != nullptr) ? (string(p->nombre) + " " + string(p->apellido)) : "Paciente Eliminado";
        string nombreDoctor = (d != nullptr) ? (string(d->nombre) + " " + string(d->apellido)) : "Doctor Eliminado";
        
        char motivoCorto[21];
        strncpy(motivoCorto, c->motivo, 17);
        motivoCorto[17] = '\0';
        if (strlen(c->motivo) > 17) {
            strcat(motivoCorto, "...");
        }

        cout << "| " << left << setw(5) << c->idCita
             << "| " << setw(10) << c->fecha
             << "| " << setw(5) << c->hora
             << "| " << setw(30) << nombrePaciente.c_str()
             << "| " << setw(30) << nombreDoctor.c_str()
             << "| " << setw(20) << motivoCorto
             << "| " << setw(10) << c->estado
             << "|" << endl;
    }
    cout << setfill('-') << setw(140) << "" << setfill(' ') << endl;
}


void listarCitasPendientes(Hospital* hospital) {
    int cantidad = 0;
    // Paso 1: Contar y crear array temporal de punteros
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            cantidad++;
        }
    }

    if (cantidad == 0) {
        cout << "\n--- LISTADO DE CITAS PENDIENTES ---" << endl;
        cout << "No hay citas pendientes (Agendadas)." << endl;
        return;
    }

    Cita** citasPendientes = new (std::nothrow) Cita*[cantidad];
    if (citasPendientes == nullptr) {
        cerr << "ERROR DE MEMORIA: No se pudo asignar el array de punteros a citas." << endl;
        return;
    }

    // Paso 2: Llenar el array
    int indice = 0;
    for (int i = 0; i < hospital->cantidadCitas; ++i) {
        if (strcmp(hospital->citas[i].estado, "Agendada") == 0) {
            citasPendientes[indice++] = &hospital->citas[i];
        }
    }

    // Paso 3: Mostrar
    mostrarListaCitas(hospital, citasPendientes, cantidad, "LISTADO DE CITAS PENDIENTES (Agendadas)");

    // Paso 4: Liberar el array de punteros
    delete[] citasPendientes;
}


// =================================================================
// 6. INTERFAZ DE USUARIO (UI)
// =================================================================


void limpiarBuffer() {
    cin.ignore(10000, '\n');
}


int menuPrincipal() {
    int opcion;
    cout << "\n\n||========================================||" << endl;
    cout << "||    SISTEMA DE GESTION HOSPITALARIA     ||" << endl;
    cout << "||========================================||" << endl;
    cout << "1. Gestion de Pacientes" << endl;
    cout << "2. Gestion de Doctores" << endl;
    cout << "3. Gestion de Citas" << endl;
    cout << "4. Salir" << endl;
    cout << "Seleccione una opcion: ";
    if (!(cin >> opcion)) {
        cin.clear();
        opcion = -1; // Opción inválida
    }
    system("cls");
    limpiarBuffer();
    return opcion;
    
}


int menuPacientes() {
    int opcion;
    cout << "\n--- MENU DE PACIENTES ---" << endl;
    cout << "1. Registrar nuevo paciente" << endl;
    cout << "2. Buscar paciente por cedula" << endl;
    cout << "3. Buscar paciente por nombre" << endl;
    cout << "4. Ver historial medico completo" << endl;
    cout << "5. Actualizar datos del paciente" << endl;
    cout << "6. Listar todos los pacientes" << endl;
    cout << "7. Eliminar paciente" << endl;
    cout << "0. Volver al menu principal" << endl;
    cout << "Seleccione una opcion: ";
    if (!(cin >> opcion)) {
        cin.clear();
        opcion = -1;
    }
    system("cls");
    limpiarBuffer();
    return opcion;
}


void gestionarPacientes(Hospital* hospital) {
    int opcion;
    do {
        opcion = menuPacientes();
        char buffer[501];
        int id;
        
        switch (opcion) {
            case 1: { // Registrar nuevo paciente
                char nombre[51], apellido[51], cedula[21], sexo;
                int edad;
                cout << "--- REGISTRAR PACIENTE ---" << endl;
                cout << "Nombre (máx 50): "; cin.getline(nombre, 51);
                cout << "Apellido (máx 50): "; cin.getline(apellido, 51);
                cout << "Cedula de Identidad (max 20, UNICA): "; cin.getline(cedula, 21);
                
                if (!validarCedula(cedula)) {
                    cout << "ERROR: Cedula invalida o vacia." << endl;
                    break;
                }
                
                cout << "Edad (0-120): "; 
                if (!(cin >> edad)) { edad = -1; cin.clear(); } limpiarBuffer();
                if (edad < 0 || edad > 120) {
                    cout << "ERROR: Edad fuera de rango (0-120)." << endl;
                    break;
                }

                cout << "Sexo ('M' o 'F'): "; cin >> sexo; limpiarBuffer();
                
                Paciente* p = crearPaciente(hospital, nombre, apellido, cedula, edad, sexo);
                if (p != nullptr) {
                    cout << "\n Paciente registrado con EXITO. ID: " << p->id << endl;
                }
                break;
                system("cls");
            }
            case 2: { // Buscar paciente por cédula
                char cedula[21];
                cout << "--- BUSCAR PACIENTE POR CEDULA ---" << endl;
                cout << "Cedula a buscar: "; cin.getline(cedula, 21);
                Paciente* p = buscarPacientePorCedula(hospital, cedula);
                if (p != nullptr) {
                    cout << "\n Paciente encontrado:" << endl;
                    cout << "   ID: " << p->id << endl;
                    cout << "   Nombre: " << p->nombre << " " << p->apellido << endl;
                    cout << "   Edad/Sexo: " << p->edad << "/" << p->sexo << endl;
                    cout << "   Consultas: " << p->cantidadConsultas << endl;
                } else {
                    cout << "\n Paciente con cedula " << cedula << " NO encontrado." << endl;
                }
                break;
                system("cls");
            }
            case 3: { // Buscar paciente por nombre
                char nombre[51];
                int cantidad = 0;
                cout << "--- BUSCAR PACIENTE POR NOMBRE ---" << endl;
                cout << "Nombre (o parte del nombre) a buscar: "; cin.getline(nombre, 51);
                Paciente** resultados = buscarPacientesPorNombre(hospital, nombre, &cantidad);
                
                if (cantidad > 0) {
                    cout << "\n Se encontraron " << cantidad << " pacientes con el nombre '" << nombre << "':" << endl;
                    // Reutilizamos listarPacientes para mostrar los resultados de la búsqueda
                    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;
                    cout << "| " << left << setw(5) << "ID"
                         << "| " << setw(30) << "Nombre Completo"
                         << "| " << setw(15) << "Cédula"
                         << "| " << right << setw(5) << "Edad"
                         << "| " << setw(5) << "Sexo"
                         << "| " << setw(10) << "Consultas"
                         << "| " << left << setw(10) << "Activo"
                         << "|" << endl;
                    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;

                    for (int i = 0; i < cantidad; ++i) {
                        Paciente* p = resultados[i];
                        cout << "| " << left << setw(5) << p->id
                             << "| " << setw(30) << (string(p->nombre) + " " + string(p->apellido)).c_str()
                             << "| " << setw(15) << p->cedula
                             << "| " << right << setw(5) << p->edad
                             << "| " << setw(5) << p->sexo
                             << "| " << setw(10) << p->cantidadConsultas
                             << "| " << left << setw(10) << (p->activo ? "Si" : "No")
                             << "|" << endl;
                    }
                    cout << setfill('-') << setw(90) << "" << setfill(' ') << endl;

                    // Liberar el array de punteros
                    delete[] resultados;
                } else {
                    cout << "\n No se encontraron pacientes con el nombre '" << nombre << "'." << endl;
                }
                break;
                system("cls");
            }
            case 4: { // Ver historial médico completo
                cout << "--- VER HISTORIAL MÉDICO ---" << endl;
                cout << "ID del paciente: "; 
                if (!(cin >> id)) { id = -1; cin.clear(); } limpiarBuffer();
                Paciente* p = buscarPacientePorId(hospital, id);
                if (p != nullptr) {
                    mostrarHistorialMedico(hospital, p);
                } else {
                    cout << "\n Paciente con ID " << id << " no encontrado." << endl;
                }
                break;
                system("cls");
            }
            case 5: { // Actualizar datos del paciente
                cout << "--- ACTUALIZAR PACIENTE ---" << endl;
                cout << "ID del paciente a actualizar: "; 
                if (!(cin >> id)) { id = -1; cin.clear(); } limpiarBuffer();
                if (actualizarPaciente(hospital, id)) {
                    cout << "\n Datos del paciente ID " << id << " actualizados correctamente." << endl;
                } else {
                    // El error se muestra dentro de actualizarPaciente
                }
                break;
                system("cls");
            }
            case 6: { // Listar todos los pacientes
                listarPacientes(hospital);
                break;
                system("cls");
            }
            case 7: { // Eliminar paciente
                cout << "--- ELIMINAR PACIENTE ---" << endl;
                cout << "ID del paciente a eliminar: "; 
                if (!(cin >> id)) { id = -1; cin.clear(); } limpiarBuffer();
                
                if (eliminarPaciente(hospital, id)) {
                    cout << "\n Paciente ID " << id << " eliminado con exito." << endl;
                } else {
                    cout << "\n No se pudo eliminar el paciente con ID " << id << " (posiblemente no existe)." << endl;
                }
                break;
                system("cls");
            }
            case 0:
                cout << "Volviendo al menú principal..." << endl;
                break;
                system("cls");
            default:
                cout << "Opcion invalida. Intente de nuevo." << endl;
                break;
        }
    } while (opcion != 0);
}



int menuDoctores() {
    int opcion;
    cout << "\n--- MENU DE DOCTORES ---" << endl;
    cout << "1. Registrar nuevo doctor" << endl;
    cout << "2. Buscar doctor por ID" << endl;
    cout << "3. Buscar doctores por especialidad" << endl;
    cout << "4. Asignar paciente a doctor" << endl;
    cout << "5. Ver pacientes asignados a doctor" << endl;
    cout << "6. Listar todos los doctores" << endl;
    cout << "7. Eliminar doctor" << endl;
    cout << "0. Volver al menu principal" << endl;
    cout << "Seleccione una opcion: ";
    if (!(cin >> opcion)) {
        cin.clear();
        opcion = -1;
    }
    limpiarBuffer();
    return opcion;
}


void gestionarDoctores(Hospital* hospital) {
    int opcion;
    do {
        opcion = menuDoctores();
        char buffer[501];
        int idDoctor, idPaciente, cantidad;
        
        switch (opcion) {
            case 1: { // Registrar nuevo doctor
                char nombre[51], apellido[51], cedula[21], especialidad[51];
                int anios;
                float costo;
                cout << "--- REGISTRAR DOCTOR ---" << endl;
                cout << "Nombre (max 50): "; cin.getline(nombre, 51);
                cout << "Apellido (max 50): "; cin.getline(apellido, 51);
                cout << "Cedula Profesional (max 20, UNICA): "; cin.getline(cedula, 21);
                
                if (!validarCedula(cedula)) {
                    cout << "ERROR: Cedula invalida o vacia." << endl;
                    break;
                }
                
                cout << "Especialidad (max 50): "; cin.getline(especialidad, 51);
                cout << "Anios de Experiencia (min 0): "; 
                if (!(cin >> anios)) { anios = -1; cin.clear(); } limpiarBuffer();
                cout << "Costo de Consulta (mayor a 0): "; 
                if (!(cin >> costo)) { costo = 0.0f; cin.clear(); } limpiarBuffer();
                
                if (anios < 0 || costo <= 0) {
                    cout << "ERROR: Datos invalidos para experiencia o costo." << endl;
                    break;
                }
                
                Doctor* d = crearDoctor(hospital, nombre, apellido, cedula, especialidad, anios, costo);
                if (d != nullptr) {
                    cout << "\n Doctor registrado con EXITO. ID: " << d->id << endl;
                }
                break;
            }
            case 2: { // Buscar doctor por ID
                cout << "--- BUSCAR DOCTOR POR ID ---" << endl;
                cout << "ID del doctor a buscar: "; 
                if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                Doctor* d = buscarDoctorPorId(hospital, idDoctor);
                if (d != nullptr) {
                    cout << "\n Doctor encontrado:" << endl;
                    cout << "   ID: " << d->id << endl;
                    cout << "   Nombre: " << d->nombre << " " << d->apellido << endl;
                    cout << "   Especialidad: " << d->especialidad << endl;
                    cout << "   Costo: " << fixed << setprecision(2) << d->costoConsulta << endl;
                } else {
                    cout << "\n Doctor con ID " << idDoctor << " NO encontrado." << endl;
                }
                break;
            }
            case 3: { // Buscar doctores por especialidad
                char especialidad[51];
                cout << "--- BUSCAR DOCTORES POR ESPECIALIDAD ---" << endl;
                cout << "Especialidad a buscar: "; cin.getline(especialidad, 51);
                Doctor** resultados = buscarDoctoresPorEspecialidad(hospital, especialidad, &cantidad);
                
                if (cantidad > 0) {
                    cout << "\n Se encontraron " << cantidad << " doctores en la especialidad '" << especialidad << "':" << endl;
                    // Mostrar resultados en formato tabular
                    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;
                    cout << "| " << left << setw(5) << "ID"
                         << "| " << setw(30) << "Nombre Completo"
                         << "| " << setw(15) << "Especialidad"
                         << "| " << setw(10) << "Costo"
                         << "|" << endl;
                    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;

                    for (int i = 0; i < cantidad; ++i) {
                        Doctor* d = resultados[i];
                        cout << "| " << left << setw(5) << d->id
                             << "| " << setw(30) << (string(d->nombre) + " " + string(d->apellido)).c_str()
                             << "| " << setw(15) << d->especialidad
                             << "| " << right << setw(10) << fixed << setprecision(2) << d->costoConsulta
                             << "|" << endl;
                    }
                    cout << setfill('-') << setw(70) << "" << setfill(' ') << endl;

                    // Liberar el array de punteros
                    delete[] resultados;
                } else {
                    cout << "\n No se encontraron doctores en esa especialidad." << endl;
                }
                break;
            }
            case 4: { // Asignar paciente a doctor
                cout << "--- ASIGNAR PACIENTE A DOCTOR ---" << endl;
                cout << "ID del Doctor: "; 
                if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                cout << "ID del Paciente: "; 
                if (!(cin >> idPaciente)) { idPaciente = -1; cin.clear(); } limpiarBuffer();
                
                Doctor* d = buscarDoctorPorId(hospital, idDoctor);
                Paciente* p = buscarPacientePorId(hospital, idPaciente);
                
                if (d == nullptr || p == nullptr) {
                    cout << " Error: Doctor o Paciente no encontrado." << endl;
                } else if (asignarPacienteADoctor(d, idPaciente)) {
                    cout << "\n Paciente ID " << idPaciente << " asignado al Dr. " << d->apellido << " con exito." << endl;
                } else {
                    cout << "\n Error: El paciente ID " << idPaciente << " ya estaba asignado al Dr. " << d->apellido << "." << endl;
                }
                break;
            }
            case 5: { // Ver pacientes asignados a doctor
                cout << "--- PACIENTES ASIGNADOS ---" << endl;
                cout << "ID del Doctor: "; 
                if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                listarPacientesDeDoctor(hospital, idDoctor);
                break;
            }
            case 6: { // Listar todos los doctores
                listarDoctores(hospital);
                break;
            }
            case 7: { // Eliminar doctor
                cout << "--- ELIMINAR DOCTOR ---" << endl;
                cout << "ID del doctor a eliminar: "; 
                if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                
                if (eliminarDoctor(hospital, idDoctor)) {
                    cout << "\n Doctor ID " << idDoctor << " eliminado con exito. Citas canceladas." << endl;
                } else {
                    cout << "\n No se pudo eliminar el doctor con ID " << idDoctor << " (posiblemente no existe)." << endl;
                }
                break;
            }
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
            default:
                cout << "Opcion invalida. Intente de nuevo." << endl;
                break;
        }
    } while (opcion != 0);
}



int menuCitas() {
    int opcion;
    cout << "\n--- MENU DE CITAS ---" << endl;
    cout << "1. Agendar nueva cita" << endl;
    cout << "2. Cancelar cita" << endl;
    cout << "3. Atender cita" << endl;
    cout << "4. Ver citas de un paciente" << endl;
    cout << "5. Ver citas de un doctor" << endl;
    cout << "6. Ver citas de una fecha" << endl;
    cout << "7. Ver citas pendientes" << endl;
    cout << "0. Volver al menu principal" << endl;
    cout << "Seleccione una opcion: ";
    if (!(cin >> opcion)) {
        cin.clear();
        opcion = -1;
    }
    limpiarBuffer();
    return opcion;
}


void gestionarCitas(Hospital* hospital) {
    int opcion;
    do {
        opcion = menuCitas();
        int idCita, idPaciente, idDoctor, cantidad;
        char fecha[11], hora[6], motivo[151];
        
        switch (opcion) {
            case 1: { // Agendar nueva cita
                cout << "--- AGENDAR CITA ---" << endl;
                cout << "ID del Paciente: "; if (!(cin >> idPaciente)) { idPaciente = -1; cin.clear(); } limpiarBuffer();
                cout << "ID del Doctor: "; if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                cout << "Fecha (YYYY-MM-DD): "; cin.getline(fecha, 11);
                cout << "Hora (HH:MM): "; cin.getline(hora, 6);
                cout << "Motivo de la consulta (max 150): "; cin.getline(motivo, 151);

                Cita* c = agendarCita(hospital, idPaciente, idDoctor, fecha, hora, motivo);
                if (c != nullptr) {
                    cout << "\n Cita agendada con EXITO. ID: " << c->idCita << endl;
                } // Los errores se imprimen dentro de agendarCita
                break;
            }
            case 2: { // Cancelar cita
                cout << "--- CANCELAR CITA ---" << endl;
                cout << "ID de la Cita a cancelar: "; 
                if (!(cin >> idCita)) { idCita = -1; cin.clear(); } limpiarBuffer();
                
                if (cancelarCita(hospital, idCita)) {
                    cout << "\n Cita ID " << idCita << " cancelada correctamente." << endl;
                } else {
                    cout << "\n Error: Cita ID " << idCita << " no encontrada o ya está atendida/cancelada." << endl;
                }
                break;
            }
            case 3: { // Atender cita
                char diagnostico[201], tratamiento[201], medicamentos[151];
                cout << "--- ATENDER CITA ---" << endl;
                cout << "ID de la Cita a atender: "; 
                if (!(cin >> idCita)) { idCita = -1; cin.clear(); } limpiarBuffer();
                
                cout << "Diagnóstico (máx 200): "; cin.getline(diagnostico, 201);
                cout << "Tratamiento prescrito (máx 200): "; cin.getline(tratamiento, 201);
                cout << "Medicamentos recetados (máx 150): "; cin.getline(medicamentos, 151);

                if (atenderCita(hospital, idCita, diagnostico, tratamiento, medicamentos)) {
                    cout << "\n Cita ID " << idCita << " atendida correctamente y Historial Médico generado." << endl;
                } else {
                    cout << "\n Error al atender la cita ID " << idCita << "." << endl;
                }
                break;
            }
            case 4: { // Ver citas de un paciente
                cout << "--- CITAS DE PACIENTE ---" << endl;
                cout << "ID del Paciente: "; 
                if (!(cin >> idPaciente)) { idPaciente = -1; cin.clear(); } limpiarBuffer();
                
                Cita** citas = obtenerCitasDePaciente(hospital, idPaciente, &cantidad);
                mostrarListaCitas(hospital, citas, cantidad, "CITAS DE PACIENTE");
                delete[] citas;
                break;
            }
            case 5: { // Ver citas de un doctor
                cout << "--- CITAS DE DOCTOR ---" << endl;
                cout << "ID del Doctor: "; 
                if (!(cin >> idDoctor)) { idDoctor = -1; cin.clear(); } limpiarBuffer();
                
                Cita** citas = obtenerCitasDeDoctor(hospital, idDoctor, &cantidad);
                mostrarListaCitas(hospital, citas, cantidad, "CITAS DE DOCTOR");
                delete[] citas;
                break;
            }
            case 6: { // Ver citas de una fecha
                cout << "--- CITAS POR FECHA ---" << endl;
                cout << "Fecha (YYYY-MM-DD): "; cin.getline(fecha, 11);

                Cita** citas = obtenerCitasPorFecha(hospital, fecha, &cantidad);
                mostrarListaCitas(hospital, citas, cantidad, "CITAS POR FECHA");
                delete[] citas;
                break;
            }
            case 7: { // Ver citas pendientes
                listarCitasPendientes(hospital);
                break;
            }
            case 0:
                cout << "Volviendo al menu principal..." << endl;
                break;
            default:
                cout << "Opcion invalida. Intente de nuevo." << endl;
                break;
        }
    } while (opcion != 0);
}


// =================================================================
// Función principal (MAIN)
// =================================================================

int main() {
    // Inicialización
   setlocale(LC_ALL, "");
    Hospital* hospital = inicializarHospital("Hospital Central");
    if (hospital == nullptr) {
        cerr << "Error critico: No se pudo inicializar el hospital." << endl;
        return 1;
    }

    cout << "===============================================================" << endl;
    cout << " Sistema de Gestion Hospitalaria Iniciado: " << hospital->nombre << endl;
    cout << "===============================================================" << endl;

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
                cout << "\nSaliendo del sistema. Liberando memoria..." << endl;
                break;
            default:
                cout << "Opción no válida. Por favor, seleccione 1, 2, 3 o 4." << endl;
                break;
        }
    } while (opcion != 4);

    // Liberación de memoria completa al salir
    destruirHospital(hospital);
    cout << "Memoria liberada. Programa finalizado." << endl;

    return 0;
}