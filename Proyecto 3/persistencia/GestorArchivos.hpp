#ifndef GESTOR_ARCHIVOS_HPP
#define GESTOR_ARCHIVOS_HPP

#include <fstream>
#include <iostream>
#include <cstring>
#include "Constantes.hpp"

// Header de cada archivo binario
struct ArchivoHeader {
    int cantidadRegistros;
    int proximoID;
    int registrosActivos;
    int version;
};

class GestorArchivos {
private:
    // Métodos auxiliares
    static std::streampos calcularPosicion(int indice, size_t tamanioRegistro);

public:
    // Inicialización
    static bool inicializarArchivo(const char* ruta);
    static bool verificarIntegridadArchivos();

    // Operaciones de Header
    static bool leerHeader(const char* ruta, ArchivoHeader& header);
    static bool escribirHeader(const char* ruta, const ArchivoHeader& header);

    // Operaciones genéricas de registros
    static bool guardarRegistro(void* registro, size_t tamanio, const char* ruta);
    static bool leerRegistroPorIndice(int indice, void* registro, size_t tamanio, const char* ruta);
    static bool leerRegistroPorID(int id, void* registro, size_t tamanio, const char* ruta);
    static bool actualizarRegistro(void* registro, size_t tamanio, const char* ruta, int indice);
    static bool eliminarRegistro(int id, size_t tamanio, const char* ruta);
    static bool listarTodosRegistros(void* buffer, int& cantidad, size_t tamanio, const char* ruta);

    // Métodos de mantenimiento
    static int contarRegistros(const char* ruta, size_t tamanio);
    static int contarRegistrosActivos(const char* ruta, size_t tamanio);
    static bool compactarArchivo(const char* ruta, size_t tamanio);
};

#endif
