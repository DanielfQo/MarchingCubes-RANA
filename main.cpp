#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include "estructuras.h"
#include "procesamiento.h"
#include "marching_cubes.h"

float anguloX = 0.0f, anguloY = 0.0f;
bool rotando = false;
float ultimoX = 0.0f, ultimoY = 0.0f;
float zoom = 900.0f;

glm::vec3 centroModelo(0.0f);

std::vector<Punto3D> puntosGlobal;

std::vector<std::vector<Vertex>> verticesGlobal;
std::vector<std::vector<std::array<int, 3>>> carasGlobal;

std::vector<bool> visiblePartes;

GLFWwindow* ventana = nullptr;

glm::vec3 calcularCentroide(const std::vector<std::vector<Vertex>>& todasVertices) {
    glm::vec3 centro(0.0f);
    int total = 0;

    for (const auto& parte : todasVertices) {
        for (const auto& v : parte) {
            centro += v.position;
            total++;
        }
    }

    if (total > 0) centro /= static_cast<float>(total);
    return centro;
}

void configurarCamara() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int ancho, alto;
    glfwGetFramebufferSize(ventana, &ancho, &alto);
    if (alto == 0) alto = 1;
    float aspect = static_cast<float>(ancho) / alto;
    gluPerspective(45.0f, aspect, 1.0f, 3000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(280.0, 235.0, zoom,
          0.0, 0.0, 0.0,
          0.0, 1.0, 0.0);

}

void aplicarRotaciones() {
    glRotatef(anguloX, 1.0f, 0.0f, 0.0f);
    glRotatef(anguloY, 0.0f, 1.0f, 0.0f);
}

void dibujarEjes() {
    glLineWidth(10.0f);
    glBegin(GL_LINES);

    glColor3f(1, 0, 0);
    glVertex3f(0,0,0); glVertex3f(100,0,0);

    glColor3f(0,1,0);
    glVertex3f(0,0,0); glVertex3f(0,100,0);

    glColor3f(0,0,1);
    glVertex3f(0,0,0); glVertex3f(0,0,100);

    glEnd();
}

void dibujarSuperficie(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i + 2 < indices.size(); i += 3) {
        const auto& v0 = vertices[indices[i]];
        const auto& v1 = vertices[indices[i + 1]];
        const auto& v2 = vertices[indices[i + 2]];

        glNormal3f(v0.normal.x, v0.normal.y, v0.normal.z);  
        glColor3f(v0.color.r, v0.color.g, v0.color.b);
        glVertex3f(v0.position.x, v0.position.y, v0.position.z);

        glNormal3f(v1.normal.x, v1.normal.y, v1.normal.z);  
        glColor3f(v1.color.r, v1.color.g, v1.color.b);
        glVertex3f(v1.position.x, v1.position.y, v1.position.z);

        glNormal3f(v2.normal.x, v2.normal.y, v2.normal.z);  
        glColor3f(v2.color.r, v2.color.g, v2.color.b);
        glVertex3f(v2.position.x, v2.position.y, v2.position.z);
    }
    glEnd();
}

void dibujarCaras(const std::vector<Vertex>& vertices, const std::vector<std::array<int, 3>>& caras) {
    glBegin(GL_TRIANGLES);
    for (const auto& cara : caras) {
        for (int idx : cara) {
            const Vertex& v = vertices[idx];
            glColor3f(v.color.r, v.color.g, v.color.b);
            glNormal3f(v.normal.x, v.normal.y, v.normal.z);
            glVertex3f(v.position.x, v.position.y, v.position.z);
        }
    }
    glEnd();
}

void dibujarTodasLasPartes(const std::vector<std::vector<Vertex>>& todasVertices,
                           const std::vector<std::vector<std::array<int, 3>>>& todasCaras) {
    for (size_t i = 0; i < todasVertices.size(); i++) {
        if (visiblePartes[i]) {
            dibujarCaras(todasVertices[i], todasCaras[i]);
        }
    }
}

void configurarIluminacion() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

    GLfloat light_pos[] = { 0.0f, 100.0f, 100.0f, 1.0f };
    GLfloat light_diff[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_amb[] = { 0.2f, 0.2f, 0.2f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);

    glShadeModel(GL_SMOOTH);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    configurarCamara();
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rotando) {
        float dx = xpos - ultimoX;
        float dy = ypos - ultimoY;
        anguloX += dy * 0.5f;
        anguloY += dx * 0.5f;
        ultimoX = xpos;
        ultimoY = ypos;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            rotando = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ultimoX = xpos;
            ultimoY = ypos;
        } else if (action == GLFW_RELEASE) {
            rotando = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom -= (float)(yoffset * 10.0f);
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 2000.0f) zoom = 2000.0f;
}
std::vector<std::string> nombresCarpetas = {
    "blood" , "brain", "duodenum", "eye", "eyeRetna", "eyeWhite",
    "heart", "kidney", "ileum", "lIntestine", "liver", "lung",
     "nerve", "skeleton", "stomach", "spleen" , "muscle"
};

void imprimirEstadoPartes() {
    std::cout << "\n--- Estado de visibilidad de las partes ---\n";
    for (size_t i = 0; i < nombresCarpetas.size(); ++i) {
        char letra = 'A' + static_cast<char>(i);
        std::cout << letra << " - " << nombresCarpetas[i] 
                  << ": " << (visiblePartes[i] ? "activo" : "inactivo") << std::endl;
    }
    std::cout << "-------------------------------------------\n";
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key >= GLFW_KEY_A && key < GLFW_KEY_A + (int)visiblePartes.size()) {
            int idx = key - GLFW_KEY_A;
            visiblePartes[idx] = !visiblePartes[idx]; 
            imprimirEstadoPartes();
        }
    }
}

// Colores para cada parte
glm::vec3 mascara_colors[] = {
    {0.13f, 0.54f, 0.13f},   // blood: verde oscuro (sangre venosa en sapo)
    {0.36f, 0.54f, 0.27f},   // brain: gris verdoso (cerebro)
    {0.95f, 0.85f, 0.60f},   // duodenum: beige claro (intestino delgado)
    {0.85f, 0.85f, 0.60f},   // eye: amarillo pálido (ojo)
    {0.85f, 0.22f, 0.22f},   // eyeRetna: rojo oscuro (retina)
    {0.98f, 0.98f, 0.98f},   // eyeWhite: blanco puro (esclerótica)
    {0.80f, 0.13f, 0.13f},   // heart: rojo oscuro (corazón)
    {0.60f, 0.80f, 0.60f},   // kidney: verde claro (riñón)
    {0.95f, 0.85f, 0.60f},   // ileum: beige claro (íleon)
    {0.95f, 0.85f, 0.60f},   // lIntestine: beige claro (intestino grueso)
    {0.85f, 0.60f, 0.13f},   // liver: marrón amarillento (hígado)
    {0.60f, 0.80f, 0.80f},   // lung: azul verdoso pálido (pulmón)
    {0.90f, 0.90f, 0.60f},   // nerve: amarillo pálido (nervio)
    {0.95f, 0.95f, 0.85f},   // skeleton: hueso blanco amarillento
    {0.95f, 0.85f, 0.60f},   // stomach: beige claro (estómago)
    {0.80f, 0.60f, 0.13f},   // spleen: marrón amarillento (bazo)
    {0.80f, 0.60f, 0.40f}, // muscle: marrón claro (músculo)
};

void procesarPartesDelCuerpo(const std::vector<std::string>& partes) {
    procesarTodasLasCarpetas(partes);

    verticesGlobal.clear();
    carasGlobal.clear();

    for (size_t i = 0; i < partes.size(); ++i) {
        const auto& nombreParte = partes[i];
        std::string archivoPuntos = "nubes/puntos_3D_" + nombreParte + ".txt";
        auto puntos = cargarPuntosDesdeArchivo(archivoPuntos);

        std::cout << "Cargando parte: " << nombreParte << " con " << puntos.size() << " puntos." << std::endl;

        int minX = INT_MAX, maxX = INT_MIN;
        int minY = INT_MAX, maxY = INT_MIN;
        int minZ = INT_MAX, maxZ = INT_MIN;
        for (const auto& p : puntos) {
            minX = std::min(minX, int(p.x));
            maxX = std::max(maxX, int(p.x));
            minY = std::min(minY, int(p.y));
            maxY = std::max(maxY, int(p.y));
            minZ = std::min(minZ, int(p.z));
            maxZ = std::max(maxZ, int(p.z));
        }

        int margen = 20;

        minX -= margen;
        minY -= margen;
        minZ -= margen;

        maxX += margen;
        maxY += margen;
        maxZ += margen;

        int resX = maxX - minX + 1;
        int resY = maxY - minY + 1;
        int resZ = maxZ - minZ + 1;

        auto volumen = generarVolumenDesdeNube(puntos, minX, minY, minZ, resX, resY, resZ, 1.0f);

        std::vector<std::vector<std::vector<glm::vec3>>> volumen_color(
            resZ, std::vector<std::vector<glm::vec3>>(resY, std::vector<glm::vec3>(resX, mascara_colors[i])));

        std::vector<Vertex> verticesParte;
        std::vector<std::array<int, 3>> carasParte;

        float isoLevel = 0.5f;

        marchingCubes(volumen, volumen_color, resX, resY, resZ, isoLevel, verticesParte, carasParte, minX, minY, minZ);

        calcularNormales({verticesParte}, {carasParte});

        exportarOBJ(verticesParte, carasParte, "mallas/modelo_" + nombreParte + ".obj");

        verticesGlobal.push_back(verticesParte);
        carasGlobal.push_back(carasParte);

    }
}

int main(int argc, char** argv) {
    std::vector<std::string> partesAMostrar = nombresCarpetas;
    visiblePartes.resize(nombresCarpetas.size(), true); 

    procesarPartesDelCuerpo(partesAMostrar);

    if (!glfwInit()) {
        std::cerr << "Error" << std::endl;
        return -1;
    }

    ventana = glfwCreateWindow(800, 800, "SAPO", NULL, NULL);
    if (!ventana) {
        std::cerr << "error" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(ventana);
    glfwSetFramebufferSizeCallback(ventana, framebuffer_size_callback);
    glfwSetCursorPosCallback(ventana, cursor_position_callback);
    glfwSetMouseButtonCallback(ventana, mouse_button_callback);
    glfwSetScrollCallback(ventana, scroll_callback);
    glfwSetKeyCallback(ventana, key_callback);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    configurarIluminacion();
    imprimirEstadoPartes();

    centroModelo = calcularCentroide(verticesGlobal);

    std::cout << "Centro del modelo: (" << centroModelo.x << ", " << centroModelo.y << ", " << centroModelo.z << ")\n";

    while (!glfwWindowShouldClose(ventana)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        configurarCamara();

        glPushMatrix();

            aplicarRotaciones();
            glTranslatef(-centroModelo.x, -centroModelo.y, -centroModelo.z);

            dibujarTodasLasPartes(verticesGlobal, carasGlobal);

        glPopMatrix();

        glfwSwapBuffers(ventana);
        glfwPollEvents();
    }

    glfwDestroyWindow(ventana);
    glfwTerminate();

    return 0;
}
