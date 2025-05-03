#include "Object3d/Object3d.hpp"

int main() {

    Object3d b("./bunny.ply");
    Object3d b_cut = b.cut([](float x, float y, float z) -> bool {
        return y + z < .12;
    });
    b_cut.save("./bunny_cut.ply");

    return 0;
}
