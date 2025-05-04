cmake_minimum_required(VERSION 3.30.5)
project(ply_3d_shapes)

set(CMAKE_CXX_STANDARD 14)

add_executable(ply_3d_shapes
        README.md
        main.cpp
        Object3d/Object3d.cpp
        Object3d/Object3d.hpp)