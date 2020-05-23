//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#ifndef GLSL_RAYTRACING_UTIL_H
#define GLSL_RAYTRACING_UTIL_H

#include <vector>
#include <cstdint>

// type alias
using Blob = std::vector<uint8_t>;

bool read_file(const char *path, Blob &out_blob);

#endif //GLSL_RAYTRACING_UTIL_H
