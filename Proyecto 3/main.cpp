#include "hospital/Hospital.hpp"
#include "Paciente/operacionesPacientes.hpp"
#include "doctores/operacionesDoctores.hpp"
#include "Cita/Operacionescita.hpp"
#include "historial/operacionesHistorial.hpp"
#include "persistencia/GestorArchivos.hpp"
#include <iostream>
#include <vector>

using namespace std;

// Buffers temporales (en integración real se usarán GestorArchivos)
vector<Paciente> bufferPacientes;
vector<Doctor> bufferDoctores;
vector<Cita> bufferCitas;
vector<HistorialMedico> bufferHistorial;

int proximoIdPaciente = 1;
int proximoIdDoctor = 1;
int proximoIdCita = 1;
int proximoIdHistorial = 1;

// Menús
void mostrarMenuPrincipal();
void menuPacientes(Hospital& hospital);
void menuDoctores(Hospital& hospital);
void menuCitas(Hospital& hospital);
void menuHistorial(Hospital& hospital);

int main() {
    // Inicializar sistema de archivos
    if (!GestorArchivos::verificarIntegridadArchivos()) {
        cerr << "Error al inicializar archivos" << endl;
        return 1;
    }

    // Crear hospital por defecto
    Hospital hospital("Hospital General", "Calle Principal #123", "555-1234");

    int opcion;
    do {
        mostrarMenuPrincipal();
        cin >> opcion;
        cin.ignore();

        switch(opcion) {
            case 1: menuPacientes(hospital); break;
            case 2: menuDoctores(hospital); break;
            case 3: menuCitas(hospital); break;
            case 4: menuHistorial(hospital); break;
            case 5:
                cout << "Datos guardados. Hasta pronto!" << endl;
                break;
            default:
                cout << "Opción inválida" << endl;
        }
    } while(opcion != 5);

    return 0;
}

void mostrarMenuPrincipal() {
    cout << "\n=== SISTEMA DE GESTIÓN HOSPITALARIA v3 ===" << endl;
    cout << "1. Gestión de Pacientes" << endl;
    cout << "2. Gestión de Doctores" << endl;
    cout << "3. Gestión de Citas" << endl;
    cout << "4. Historial Médico" << endl;
    cout << "5. Guardar y Salir" << endl;
    cout << "Opción: ";
}

void menuPacientes(Hospital& hospital) {
    int opcion;
    do {
        cout << "\n=== GESTIÓN DE PACIENTES ===" << endl;
        cout << "1. Registrar nuevo paciente" << endl;
        cout << "2. Buscar por ID" << endl;
        cout << "3. Listar todos" << endl;
        cout << "4. Volver" << endl;
        cout << "Opción: ";
        cin >> opcion; cin.ignore();

        switch(opcion) {
            case 1: ServicioPacientes::registrarPaciente(bufferPacientes, proximoIdPaciente); hospital.incrementarPacientesRegistrados(); break;
            case 2: {
                int id; cout << "ID: "; cin >> id; cin.ignore();
                const Paciente* p = ServicioPacientes::buscarPacientePorID(bufferPacientes, id);
                if (p) p->mostrarInformacionCompleta(); else cout << "No encontrado." << endl;
            } break;
            case 3: ServicioPacientes::listarTodosPacientes(bufferPacientes); break;
            case 4: cout << "Volviendo..." << endl; break;
            default: cout << "Opción inválida" << endl;
        }
    } while(opcion != 4);
}

void menuDoctores(Hospital& hospital) {
    int opcion;
    do {
        cout << "\n=== GESTIÓN DE DOCTORES ===" << endl;
        cout << "1. Registrar nuevo doctor" << endl;
        cout << "2. Buscar por ID" << endl;
        cout << "3. Listar todos" << endl;
        cout << "4. Volver" << endl;
        cout << "Opción: ";
        cin >> opcion; cin.ignore();

        switch(opcion) {
            case 1: ServicioDoctores::registrarDoctor(bufferDoctores, proximoIdDoctor); hospital.incrementarDoctoresRegistrados(); break;
            case 2: {
                int id; cout << "ID: "; cin >> id; cin.ignore();
                const Doctor* d = ServicioDoctores::buscarDoctorPorID(bufferDoctores, id);
                if (d) d->mostrarInformacionCompleta(); else cout << "No encontrado." << endl;
            } break;
            case 3: ServicioDoctores::listarTodosDoctores(bufferDoctores); break;
            case 4: cout << "Volviendo..." << endl; break;
            default: cout << "Opción inválida" << endl;
        }
    } while(opcion != 4);
}

void menuCitas(Hospital& hospital) {
    int opcion;
    do {
        cout << "\n=== GESTIÓN DE CITAS ===" << endl;
        cout << "1. Registrar nueva cita" << endl;
        cout << "2. Buscar por ID" << endl;
        cout << "3. Listar todas" << endl;
        cout << "4. Volver" << endl;
        cout << "Opción: ";
        cin >> opcion; cin.ignore();

        switch(opcion) {
            case 1: ServicioCitas::registrarCita(bufferCitas, proximoIdCita); hospital.incrementarCitasRegistradas(); break;
            case 2: {
                int id; cout << "ID: "; cin >> id; cin.ignore();
                const Cita* c = ServicioCitas::buscarCitaPorID(bufferCitas, id);
                if (c) c->mostrarInformacionCompleta(); else cout << "No encontrada." << endl;
            } break;
            case 3: ServicioCitas::listarTodasCitas(bufferCitas); break;
            case 4: cout << "Volviendo..." << endl; break;
            default: cout << "Opción inválida" << endl;
        }
    } while(opcion != 4);
}

void menuHistorial(Hospital& hospital) {
    int opcion;
    do {
        cout << "\n=== HISTORIAL MÉDICO ===" << endl;
        cout << "1. Registrar nueva consulta" << endl;
        cout << "2. Buscar consulta por ID" << endl;
        cout << "3. Listar historial de paciente" << endl;
        cout << "4. Volver" << endl;
        cout << "Opción: ";
        cin >> opcion; cin.ignore();

        switch(opcion) {
            case 1: ServicioHistorial::registrarConsulta(bufferHistorial, proximoIdHistorial); hospital.incrementarHistorialesRegistrados(); break;
            case 2: {
                int id; cout << "ID: "; cin >> id; cin.ignore();
                const HistorialMedico* h = ServicioHistorial::buscarConsultaPorID(bufferHistorial, id);
                if (h) h->mostrarInformacionCompleta(); else cout << "No encontrado." << endl;
            } break;
            case 3: {
                int pacienteId; cout << "ID Paciente: "; cin >> pacienteId; cin.ignore();
                ServicioHistorial::listarHistorialPaciente(bufferHistorial, pacienteId);
            } break;
            case 4: cout << "Volviendo..." << endl; break;
            default: cout << "Opción inválida" << endl;
        }
    } while(opcion != 4);
}
