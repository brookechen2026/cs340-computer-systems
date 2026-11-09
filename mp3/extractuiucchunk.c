/* extractuiucchunk.c - Extracts the uiuc chunk from a PNG file */
#include "pnglib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "USAGE: %s existingfilename.png filetosaveto\n", argv[0]);
        return 1;
    }

    if (check_png_structure(argv[1])) {
        return 1;  // PNG 结构不合法
    }

    return extract_uiuc_chunk(argv[1], argv[2]);
}



int extract_uiuc_chunk(const char *input_filename, const char *output_filename) {
    FILE *file = fopen(input_filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", input_filename);
        return 1;
    }

    if (!validate_png_signature(file)) {
        fprintf(stderr, "Error: Invalid PNG file\n");
        fclose(file);
        return 1;
    }

    PNGChunk chunk;
    int found = 0;

    while (read_png_chunk(file, &chunk)) {
        if (strcmp(chunk.type, UIUC_CHUNK_TYPE) == 0) {
            FILE *out_file = fopen(output_filename, "wb");
            if (!out_file) {
                fprintf(stderr, "Error: Cannot create output file %s\n", output_filename);
                free_png_chunk(&chunk);
                fclose(file);
                return 1;
            }

            fwrite(chunk.data, 1, chunk.length, out_file);
            fclose(out_file);
            found = 1;
        }
        free_png_chunk(&chunk);
    }

    fclose(file);
    
    if (!found) {
        fprintf(stderr, "Error: uiuc chunk not found\n");
        return 4;
    }

    return 0;
}


