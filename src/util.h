//
// Created by murmur wheel on 2020/3/22.
//

#ifndef GLSL_RAYTRACING_UTIL_H
#define GLSL_RAYTRACING_UTIL_H

#include <vector>

using blob = std::vector<uint8_t>;

struct Short2 {
    int16_t x, y;
};

struct Vec3 {
    float x, y, z;
};

struct Vec4 {
    float x, y, z, w;
};

template<typename T>
struct Bitmap {
    const int rows = -1;
    const int cols = -1;
    std::vector<T> buf;

    Bitmap(int rows, int cols) : rows(rows), cols(cols) {
        int len = rows * cols;
        buf.resize(len);
    }

    const T &at(int row, int col) const {
        int pos = row * cols + col;
        return buf[pos];
    }

    T &at(int row, int col) {
        int pos = row * cols + col;
        return buf[pos];
    }
};

blob readFile(const char *filename);

#endif // GLSL_RAYTRACING_UTIL_H
