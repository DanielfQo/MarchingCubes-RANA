#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <tuple>
#include <sys/stat.h>
#include "estructuras.h"

bool procesarImagenesYGuardar(const std::string& carpeta, const std::string& archivoSalida, int inicio = 1, int fin = 136, float escalaZ = 1.0f) {
    std::ofstream archivo(archivoSalida);
    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de salida: " << archivoSalida << "\n";
        return false;
    }
    for (int z = inicio; z <= fin; ++z) {
        std::stringstream nombre_archivo;
        nombre_archivo << carpeta << z << ".png";
        cv::Mat binaria = cv::imread(nombre_archivo.str(), cv::IMREAD_GRAYSCALE);
        if (binaria.empty()) continue;
        for (int y = 0; y < binaria.rows; ++y) {
            for (int x = 0; x < binaria.cols; ++x) {
                if (binaria.at<uchar>(y, x) > 0) {
                    archivo << x << " " << y << " " << z * escalaZ << "\n";
                }
            }
        }
    }
    archivo.close();
    return true;
}

std::vector<Punto3D> cargarPuntosDesdeArchivo(const std::string& archivoEntrada) {
    std::vector<Punto3D> puntos;
    std::ifstream archivo(archivoEntrada);
    if (!archivo.is_open()) return puntos;
    float x, y, z;
    while (archivo >> x >> y >> z) {
        puntos.emplace_back(x, y, z);
    }
    archivo.close();
    return puntos;
}

void procesarTodasLasCarpetas(const std::vector<std::string>& nombresCarpetas, const std::string& rutaBaseImagenes = "images/", const std::string& rutaBaseNubes = "nubes/") {
    for (const auto& nombre : nombresCarpetas) {
        std::string carpeta = rutaBaseImagenes + nombre + "/";
        std::string archivoPuntos = rutaBaseNubes + "puntos_3D_" + nombre + ".txt";
        struct stat buffer;
        if (stat(archivoPuntos.c_str(), &buffer) != 0) {
            procesarImagenesYGuardar(carpeta, archivoPuntos);
        }
    }
}


