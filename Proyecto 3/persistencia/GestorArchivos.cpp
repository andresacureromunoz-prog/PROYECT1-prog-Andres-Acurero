#include "GestorArchivos.hpp"

std::streampos GestorArchivos::calcularPosicion(int indice, size_t tamanioRegistro) {
    return sizeof(ArchivoHeader) + (indice * tamanioRegistro);
}

bool GestorArchivos::inicializarArchivo(const char* ruta) {
    std::fstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) {
        // Crear archivo nuevo con header
        std::ofstream nuevo(ruta, std::ios::out | std::ios::binary);
        if (!nuevo) return false;
        ArchivoHeader header = {0, 1, 0, 1};
        nuevo.write(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
        nuevo.close();
    }
    return true;
}

bool GestorArchivos::verificarIntegridadArchivos() {
    return inicializarArchivo(Rutas::PACIENTES) &&
           inicializarArchivo(Rutas::DOCTORES) &&
           inicializarArchivo(Rutas::CITAS) &&
           inicializarArchivo(Rutas::HISTORIALES) &&
           inicializarArchivo(Rutas::HOSPITAL);
}

bool GestorArchivos::leerHeader(const char* ruta, ArchivoHeader& header) {
    std::ifstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) return false;
    archivo.read(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool GestorArchivos::escribirHeader(const char* ruta, const ArchivoHeader& header) {
    std::fstream archivo(ruta, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo) return false;
    archivo.write(reinterpret_cast<const char*>(&header), sizeof(ArchivoHeader));
    archivo.close();
    return true;
}

bool GestorArchivos::guardarRegistro(void* registro, size_t tamanio, const char* ruta) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) return false;

    std::ofstream archivo(ruta, std::ios::out | std::ios::binary | std::ios::app);
    if (!archivo) return false;

    archivo.write(reinterpret_cast<char*>(registro), tamanio);
    archivo.close();

    header.cantidadRegistros++;
    header.registrosActivos++;
    header.proximoID++;
    escribirHeader(ruta, header);

    return true;
}

bool GestorArchivos::leerRegistroPorIndice(int indice, void* registro, size_t tamanio, const char* ruta) {
    std::ifstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) return false;

    archivo.seekg(calcularPosicion(indice, tamanio));
    archivo.read(reinterpret_cast<char*>(registro), tamanio);
    archivo.close();
    return true;
}

bool GestorArchivos::leerRegistroPorID(int id, void* registro, size_t tamanio, const char* ruta) {
    std::ifstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) return false;

    ArchivoHeader header;
    leerHeader(ruta, header);

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(calcularPosicion(i, tamanio));
        archivo.read(reinterpret_cast<char*>(registro), tamanio);

        // Asumimos que el primer campo del registro es el ID
        int* idPtr = reinterpret_cast<int*>(registro);
        if (*idPtr == id) {
            archivo.close();
            return true;
        }
    }
    archivo.close();
    return false;
}

bool GestorArchivos::actualizarRegistro(void* registro, size_t tamanio, const char* ruta, int indice) {
    std::fstream archivo(ruta, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo) return false;

    archivo.seekp(calcularPosicion(indice, tamanio));
    archivo.write(reinterpret_cast<char*>(registro), tamanio);
    archivo.close();
    return true;
}

bool GestorArchivos::eliminarRegistro(int id, size_t tamanio, const char* ruta) {
    ArchivoHeader header;
    leerHeader(ruta, header);

    char* buffer = new char[tamanio];
    std::fstream archivo(ruta, std::ios::in | std::ios::out | std::ios::binary);
    if (!archivo) return false;

    for (int i = 0; i < header.cantidadRegistros; i++) {
        archivo.seekg(calcularPosicion(i, tamanio));
        archivo.read(buffer, tamanio);

        int* idPtr = reinterpret_cast<int*>(buffer);
        if (*idPtr == id) {
            // Marcamos como eliminado (ID negativo)
            *idPtr = -1;
            archivo.seekp(calcularPosicion(i, tamanio));
            archivo.write(buffer, tamanio);
            header.registrosActivos--;
            escribirHeader(ruta, header);
            delete[] buffer;
            archivo.close();
            return true;
        }
    }
    delete[] buffer;
    archivo.close();
    return false;
}

bool GestorArchivos::listarTodosRegistros(void* buffer, int& cantidad, size_t tamanio, const char* ruta) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) return false;

    cantidad = header.registrosActivos;
    std::ifstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) return false;

    archivo.seekg(sizeof(ArchivoHeader));
    archivo.read(reinterpret_cast<char*>(buffer), tamanio * header.cantidadRegistros);
    archivo.close();
    return true;
}

int GestorArchivos::contarRegistros(const char* ruta, size_t tamanio) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) return 0;
    return header.cantidadRegistros;
}

int GestorArchivos::contarRegistrosActivos(const char* ruta, size_t tamanio) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) return 0;
    return header.registrosActivos;
}

bool GestorArchivos::compactarArchivo(const char* ruta, size_t tamanio) {
    ArchivoHeader header;
    if (!leerHeader(ruta, header)) return false;

    std::ifstream archivo(ruta, std::ios::in | std::ios::binary);
    if (!archivo) return false;

    std::ofstream temp("temp.bin", std::ios::out | std::ios::binary);
    if (!temp) return false;

    temp.write(reinterpret_cast<char*>(&header), sizeof(ArchivoHeader));

