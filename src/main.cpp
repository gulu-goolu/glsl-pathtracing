//
// Created by murmur.wheel@gmail.com on 2020/5/23.
//

#include "util.h"

int main() {
    Blob shader_frag_blob;
    if (read_file("res/compile.frag.spv", shader_frag_blob)) {
        printf("load shader success!!\n");
    }
}