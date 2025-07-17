# Compilador
CXX = g++

# Flags de compilación
CXXFLAGS = -I/usr/include/eigen3 $(shell pkg-config --cflags opencv4) -Wall -std=c++17

# Librerías para enlazar
LDFLAGS = $(shell pkg-config --libs opencv4) -lGL -lGLU -lglut -lglfw

# Archivos fuente y objeto
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET = main

# Regla principal
all: $(TARGET)

# Cómo generar el ejecutable
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Cómo compilar cada archivo .cpp a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f *.o $(TARGET)

# Generación automática de dependencias
DEPFILES = $(OBJS:.o=.d)
-include $(DEPFILES)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@
