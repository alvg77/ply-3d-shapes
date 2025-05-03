#ifndef OBJECT3D_HPP
#define OBJECT3D_HPP

#include <string>
#include <vector>
#include <functional>


struct Vertex {
    float x, y, z;
};

struct Face {
    std::vector<unsigned> vertexIdxs;
};

class Object3d {
public:
    Object3d() = default;
    Object3d(const std::string& filename);
    Object3d(std::istream& is);

    int getVertexCount() const;
    int getFaceCount() const;

    void save(const std::string& filename) const;
    void print(std::ostream& os) const;

    Object3d cut(std::function<bool(float, float, float)> f) const;

    void flip();

    static Object3d generateCube(float size);
    static Object3d generateSphere(float radius);
private:
    std::vector<Face> faces;
    std::vector<Vertex> vertices;

    void load(std::istream& is);
};



#endif //OBJECT3D_HPP
