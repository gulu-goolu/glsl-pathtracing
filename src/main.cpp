#include "app.h"
#include <stdio.h>

void checkShaderExists() {
    FILE *fp;
    if (fopen_s(&fp, "trace.comp.spv", "rb") != 0 || !fp) {
        printf("resources not exists\n");
    } else {
        printf("found shader\n");
    }

    if (fp) {
        fclose(fp);
    }
}

int main() {
    App app;
    app.startup(1920, 1080);
    app.run();
    app.cleanup();
}
