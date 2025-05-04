#include "Object3d.hpp"

#include <fstream>
#include <cmath>
#include <functional>
#include <vector>

Object3d::Object3d(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    load(file);
}

Object3d::Object3d(std::istream& is) {
    load(is);
}

int Object3d::getVertexCount() const {
    return static_cast<int>(vertices.size());
}

int Object3d::getFaceCount() const {
    return static_cast<int>(faces.size());
}

void Object3d::save(const std::string& filename) const {
    std::ofstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file " + filename);
    }

    print(file);
}

void Object3d::print(std::ostream& os) const {
    os << "ply\nformat ascii 1.0\n";
    os << "element vertex " << getVertexCount() << '\n';
    os << "property float x\nproperty float y\nproperty float z\n";
    os << "element face " << getFaceCount() << '\n';
    os << "property list uchar int vertex_indices\n";
    os << "end_header\n";

    for (unsigned i = 0; i < getVertexCount(); ++i) {
        os << vertices[i].x << ' ' << vertices[i].y << ' ' << vertices[i].z << '\n';
    }

    for (const Face& face : faces) {
        os << face.vertexIdxs.size();
        for (const unsigned idx : face.vertexIdxs) {
            os << ' ' << idx;
        }
        os << '\n';
    }
}

Object3d Object3d::cut(std::function<bool(float, float, float)> f) const {
    Object3d cutObj;

    std::vector<unsigned> oldToNew (vertices.size(), -1);

    for (unsigned i = 0; i < getVertexCount(); ++i) {
        const Vertex& v = vertices[i];
        if (f(v.x, v.y, v.z)) {
            oldToNew[i] = cutObj.vertices.size();
            cutObj.vertices.push_back(v);
        }
    }

    for (unsigned i = 0; i < getFaceCount(); ++i) {
        const Face& f = faces[i];
        Face newFace;
        bool keep = true;

        for (const unsigned idx : f.vertexIdxs) {
            if (oldToNew[idx] == -1) {
                keep = false;
                break;
            }

            newFace.vertexIdxs.push_back(oldToNew[idx]);
        }

        if (keep) {
            cutObj.faces.push_back(newFace);
        }
    }

    return cutObj;
}

void Object3d::flip() {
    std::reverse(vertices.begin(), vertices.end());
}

Object3d Object3d::generateCube(float size) {
    Object3d cube;

    const float half_size = size / 2.0f;

    cube.vertices = {
        {-half_size, -half_size, -half_size},
        {half_size, -half_size, -half_size},
        {half_size, -half_size, half_size},
        {-half_size, -half_size, half_size},
        {-half_size, half_size, -half_size},
        {half_size, half_size, -half_size},
        {half_size, half_size, half_size},
        {-half_size, half_size, half_size},
    };

    cube.faces = {
        {{0, 1, 2, 3}},
        {{7, 6, 5, 4}},
        {{0, 4, 5, 1}},
        {{1, 5, 6, 2}},
        {{2, 6, 7, 3}},
        {{3, 7, 4, 0}}
    };

    return cube;
}

Object3d Object3d::generateSphere(float radius) {
    Object3d sphere;

    constexpr unsigned latSegments = 100;
    constexpr unsigned lonSegments = 100;

    sphere.vertices.push_back({0, 0, radius});

    for (unsigned i = 1; i < latSegments; ++i) {
        const float theta = static_cast<float>(i * M_PI / latSegments);
        const float sinTheta = std::sin(theta);
        const float cosTheta = std::cos(theta);

        for (unsigned j = 0; j < lonSegments; ++j) {
            const float phi = static_cast<float>(j * 2 * M_PI / lonSegments);

            const float sinPhi = std::sin(phi);
            const float cosPhi = std::cos(phi);

            const float x = radius * sinTheta * cosPhi;
            const float y = radius * sinTheta * sinPhi;
            const float z = radius * cosTheta;

            sphere.vertices.push_back({x, y, z});
        }
    }

    sphere.vertices.push_back({0, 0, -radius});

    const std::function<unsigned(unsigned, unsigned)> calcVertexIdx = [&](unsigned lat, unsigned lon) -> unsigned {
        if (lat == 0) return 0;
        if (lat == latSegments - 1) return sphere.vertices.size() - 1;
        return 1 + (lat - 1) * lonSegments + lon;
    };

    for (unsigned i = 0; i < lonSegments; ++i) {
        sphere.faces.push_back({{0, calcVertexIdx(1, i), calcVertexIdx(1, (i + 1) % lonSegments)}});
        sphere.faces.push_back({{static_cast<unsigned>(sphere.vertices.size() - 1), calcVertexIdx(latSegments - 2, i), calcVertexIdx(latSegments - 2, (i + 1) % lonSegments)}});
    }

    for (unsigned i = 1; i < latSegments - 1; ++i) {
        for (unsigned j = 0; j < lonSegments; ++j) {
            unsigned topLeft = calcVertexIdx(i, j);
            unsigned topRight = calcVertexIdx(i, (j + 1) % lonSegments);
            unsigned bottomLeft = calcVertexIdx(i + 1, j);
            unsigned bottomRight = calcVertexIdx(i + 1, (j + 1) % lonSegments);

            sphere.faces.push_back({{topLeft, bottomLeft, bottomRight}});
            sphere.faces.push_back({{topLeft, bottomRight, topRight}});
        }
    }

    return sphere;
}

void Object3d::load(std::istream& is) {
    std::string line;
    int vertexCount = 0;
    int faceCount = 0;
    bool headerEnded = false;

    while (std::getline(is, line)) {
        if (line == "end_header") {
            headerEnded = true;
            break;
        }

        if (line.rfind("comment", 0) == 0) {
            continue;
        }

        if (line.rfind("element vertex", 0) == 0) {
            const unsigned pos = line.find_last_of(' ');
            if (pos != std::string::npos) {
                vertexCount = std::stoi(line.substr(pos + 1));
            }
        } else if (line.rfind("element face", 0) == 0) {
            const unsigned pos = line.find_last_of(' ');
            if (pos != std::string::npos) {
                faceCount = std::stoi(line.substr(pos + 1));
            }
        }
    }

    if (!headerEnded) {
        throw std::runtime_error("File format invalid!");
    }

    vertices.resize(vertexCount);
    for (unsigned i = 0; i < vertexCount; i++) {
        is >> vertices[i].x >> vertices[i].y >> vertices[i].z;
    }

    faces.resize(faceCount);
    for (unsigned i = 0; i < faceCount; i++) {
        int count;
        is >> count;
        faces[i].vertexIdxs.resize(count);
        for (unsigned j = 0; j < count; j++) {
            is >> faces[i].vertexIdxs[j];
        }
    }
}
