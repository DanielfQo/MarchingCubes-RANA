#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include "estructuras.h"
#include "marching_cubes_tables.h"
#include <unordered_map>

using namespace std;

using Volumen3D = std::vector<std::vector<std::vector<float>>>;

Volumen3D generarVolumenDesdeNube(
    const std::vector<Punto3D>& nube,
    int minX, int minY, int minZ,
    int resX, int resY, int resZ,
    float sigma
) {
    Volumen3D volumen(
        resZ, std::vector<std::vector<float>>(resY, std::vector<float>(resX, 0.0f))
    );

    int radio = std::ceil(3 * sigma);

    for (const auto& centro : nube) {
        int cx = int(centro.x - minX);
        int cy = int(centro.y - minY);
        int cz = int(centro.z - minZ);

        for (int dz = -radio; dz <= radio; ++dz) {
            for (int dy = -radio; dy <= radio; ++dy) {
                for (int dx = -radio; dx <= radio; ++dx) {
                    int x = cx + dx;
                    int y = cy + dy;
                    int z = cz + dz;

                    if (x < 0 || y < 0 || z < 0 || x >= resX || y >= resY || z >= resZ)
                        continue;

                    float wx = centro.x - (minX + x);
                    float wy = centro.y - (minY + y);
                    float wz = centro.z - (minZ + z);
                    float dist2 = wx * wx + wy * wy + wz * wz;
                    volumen[z][y][x] += std::exp(-dist2 / (2 * sigma * sigma));
                }
            }
        }
    }

    return volumen;
}

Punto3D interpolar(float isolevel, const Punto3D& p1, const Punto3D& p2, float valp1, float valp2) {
    float mu = (isolevel - valp1) / (valp2 - valp1 + 1e-8f);
    float x = p1.x + mu * (p2.x - p1.x);
    float y = p1.y + mu * (p2.y - p1.y);
    float z = p1.z + mu * (p2.z - p1.z);
    glm::vec3 color = glm::mix(p1.color, p2.color, mu);
    return Punto3D(x, y, z, color);
}

void marchingCubes(const Volumen3D& vol,
                   const std::vector<std::vector<std::vector<glm::vec3>>>& colores,
                   int RES_X, int RES_Y, int RES_Z, float ISO_VALUE,
                   std::vector<Vertex>& vertices, std::vector<std::array<int, 3>>& caras,
                   int offsetX = 0, int offsetY = 0, int offsetZ = 0) {

    int dx[8] = {0, 1, 1, 0, 0, 1, 1, 0};
    int dy[8] = {0, 0, 1, 1, 0, 0, 1, 1};
    int dz[8] = {0, 0, 0, 0, 1, 1, 1, 1};

    const int edgeIndex[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int z = 0; z < RES_Z - 1; ++z) {
        for (int y = 0; y < RES_Y - 1; ++y) {
            for (int x = 0; x < RES_X - 1; ++x) {
                float cubeVal[8];
                Punto3D cubePos[8];
                for (int i = 0; i < 8; ++i) {
                    int xi = x + dx[i];
                    int yi = y + dy[i];
                    int zi = z + dz[i];
                    float valor = vol[zi][yi][xi];
                    glm::vec3 color = colores[zi][yi][xi];
                    cubeVal[i] = valor;
                    cubePos[i] = Punto3D(xi + offsetX, yi + offsetY, zi + offsetZ, color);
                }

                int cubeIndex = 0;
                for (int i = 0; i < 8; ++i)
                    if (cubeVal[i] < ISO_VALUE) cubeIndex |= (1 << i);

                if (edgeTable[cubeIndex] == 0) continue;

                Punto3D vertList[12];
                for (int i = 0; i < 12; ++i) {
                    if (edgeTable[cubeIndex] & (1 << i)) {
                        int a = edgeIndex[i][0];
                        int b = edgeIndex[i][1];
                        vertList[i] = interpolar(ISO_VALUE, cubePos[a], cubePos[b], cubeVal[a], cubeVal[b]);
                    }
                }

                for (int i = 0; triTable[cubeIndex][i] != -1; i += 3) {
                    int idx0 = vertices.size();
                    Punto3D p0 = vertList[triTable[cubeIndex][i]];
                    vertices.push_back({glm::vec3(p0.x, p0.y, p0.z), glm::vec3(0.0f), p0.color});

                    int idx1 = vertices.size();
                    Punto3D p1 = vertList[triTable[cubeIndex][i + 1]];
                    vertices.push_back({glm::vec3(p1.x, p1.y, p1.z), glm::vec3(0.0f), p1.color});

                    int idx2 = vertices.size();
                    Punto3D p2 = vertList[triTable[cubeIndex][i + 2]];
                    vertices.push_back({glm::vec3(p2.x, p2.y, p2.z), glm::vec3(0.0f), p2.color});

                    caras.push_back({idx0, idx1, idx2});
                }
            }
        }
    }
}

void calcularNormales(std::vector<Vertex>& vertices, const std::vector<std::array<int, 3>>& caras) {
    for (auto& v : vertices)
        v.normal = glm::vec3(0.0f);

    for (const auto& cara : caras) {
        int i0 = cara[0];
        int i1 = cara[1];
        int i2 = cara[2];

        glm::vec3 p0 = vertices[i0].position;
        glm::vec3 p1 = vertices[i1].position;
        glm::vec3 p2 = vertices[i2].position;

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;

        glm::vec3 normal = glm::cross(edge1, edge2);
        if (glm::length(normal) > 1e-6f)
            normal = glm::normalize(normal);

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    for (auto& v : vertices) {
        if (glm::length(v.normal) > 1e-6f)
            v.normal = glm::normalize(v.normal);
        else
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

void exportarOBJ(const std::vector<Vertex>& vertices, const std::vector<std::array<int, 3>>& caras, const std::string& nombre) {
    std::ofstream archivo(nombre);
    for (const auto& v : vertices)
        archivo << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
    for (const auto& f : caras)
        archivo << "f " << f[0] + 1 << " " << f[1] + 1 << " " << f[2] + 1 << "\n";
    archivo.close();
}

#endif
