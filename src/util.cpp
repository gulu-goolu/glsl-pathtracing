//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "util.h"
#include <cstdio>

bool read_file(const char *path, Blob &out_blob) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    const auto file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    out_blob.resize(file_size);
    auto read_size = fread(out_blob.data(), 1, file_size, fp);
    fclose(fp);

    return file_size == read_size;
}