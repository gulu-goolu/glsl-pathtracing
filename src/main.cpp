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

int main() { checkShaderExists(); }
