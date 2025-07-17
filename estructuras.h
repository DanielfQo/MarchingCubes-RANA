#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <Eigen/Dense>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <cmath>

#include <glm/glm.hpp>

struct Punto3D {
    float x, y, z;
    glm::vec3 color;

    Punto3D() : x(0), y(0), z(0), color(1.0f, 1.0f, 1.0f) {} 

    Punto3D(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_), color(1.0f, 1.0f, 1.0f) {} 

    Punto3D(float x_, float y_, float z_, glm::vec3 c)
        : x(x_), y(y_), z(z_), color(c) {}
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};