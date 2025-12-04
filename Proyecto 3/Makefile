# Nombre del compilador
CXX = g++
# Opciones de compilación
CXXFLAGS = -std=c++11 -Wall -Wextra

# Directorios
SRC_DIR = .
BIN_DIR = bin
OBJ_DIR = obj

# Archivos fuente (todos los .cpp del proyecto)
SRCS = \
    main.cpp \
    hospital/Hospital.cpp \
    pacientes/Paciente.cpp \
    pacientes/operacionesPacientes.cpp \
    doctores/Doctor.cpp \
    doctores/operacionesDoctores.cpp \
    citas/Cita.cpp \
    citas/operacionesCitas.cpp \
    historial/HistorialMedico.cpp \
    historial/operacionesHistorial.cpp \
    persistencia/GestorArchivos.cpp \
    utilidades/Validaciones.cpp \
    utilidades/Formatos.cpp

# Objetos generados
OBJS = $(SRCS:.cpp=.o)

# Ejecutable final
TARGET = hospital

# Regla principal
all: $(TARGET)

# Cómo generar el ejecutable
$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regla para compilar cada .cpp en .o
%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
    rm -f $(OBJS) $(TARGET)

# Limpiar todo (objetos + ejecutable + binarios de datos)
distclean: clean
    rm -f datos/*.bin
