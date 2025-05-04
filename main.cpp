#include <iostream>

#include "Object3d/Object3d.hpp"

int main() {
    try {
        Object3d b("./bunny.ply");

        Object3d b_cut = b.cut([](float x, float y, float z) -> bool {
            return y + z < .12;
        });
        b_cut.save("./bunny_cut.ply");
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
