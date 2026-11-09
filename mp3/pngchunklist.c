
#include "pnglib.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s existingfilename.png\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }
    if (!validate_png_signature(file)) {
        fprintf(stderr, "Error: Invalid PNG file\n");
        fclose(file);
        return 1;
    }
    PNGChunk chunk;
    while (read_png_chunk(file, &chunk)) {
        printf("%s %u\n", chunk.type, chunk.length);
        free_png_chunk(&chunk);
    }
    fclose(file);
    return 0;
}