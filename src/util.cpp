//
// Created by murmur wheel on 2020/3/22.
//

#include "util.h"

blob readFile(const char *filename) {
    FILE *fp = nullptr;
    if (fopen_s(&fp, filename, "rb") != 0 || !fp) {
        char msg[512] = {};
        sprintf_s(msg, "file not found: %s", filename);
        perror(msg);
        exit(1);
        return blob{};
    }

    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    blob buf(len);
    if (fread_s(buf.data(), buf.size(), 1, buf.size(), fp) != len) {
        buf.clear();
    }

    if (fp) {
        fclose(fp);
    }

    return buf;
}
