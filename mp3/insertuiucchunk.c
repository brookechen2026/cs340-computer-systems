#include "pnglib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "USAGE: %s existingfilename.png filetoinsert result.png\n", argv[0]);
        return 1;
    }

    if (check_png_structure(argv[1])) {
        return 1;  // PNG 结构不合法
    }

    return insert_uiuc_chunk(argv[1], argv[2], argv[3]);
}




int insert_uiuc_chunk(const char *input_filename, const char *data_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_filename);
        return 1;
    }

    FILE *data_file = fopen(data_filename, "rb");
    if (!data_file) {
        fprintf(stderr, "Error: Cannot open data file %s\n", data_filename);
        fclose(input_file);
        return 1;
    }

    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        fprintf(stderr, "Error: Cannot create output file %s\n", output_filename);
        fclose(input_file);
        fclose(data_file);
        return 1;
    }

    if (!validate_png_signature(input_file)) {
        fprintf(stderr, "Error: Invalid PNG file\n");
        fclose(input_file);
        fclose(data_file);
        fclose(output_file);
        return 1;
    }

    unsigned char signature[PNG_SIGNATURE_SIZE];
    fread(signature, 1, PNG_SIGNATURE_SIZE, input_file);
    fwrite(signature, 1, PNG_SIGNATURE_SIZE, output_file);

    fseek(data_file, 0, SEEK_END);
    long data_size = ftell(data_file);
    fseek(data_file, 0, SEEK_SET);
    unsigned char *data = malloc(data_size);
    fread(data, 1, data_size, data_file);
    fclose(data_file);

    PNGChunk chunk;
    int uiuc_inserted = 0;

    while (read_png_chunk(input_file, &chunk)) {
        if (!uiuc_inserted && strcmp(chunk.type, "IDAT") == 0) {
            PNGChunk uiuc_chunk;
            uiuc_chunk.length = data_size;
            strcpy(uiuc_chunk.type, UIUC_CHUNK_TYPE);
            uiuc_chunk.data = data;
            uiuc_chunk.crc = calculate_crc32((unsigned char *)uiuc_chunk.type, 4);
            uiuc_chunk.crc = calculate_crc32(uiuc_chunk.data, uiuc_chunk.length);

            write_png_chunk(output_file, &uiuc_chunk);
            uiuc_inserted = 1;
        }

        write_png_chunk(output_file, &chunk);
        free_png_chunk(&chunk);
    }

    fclose(input_file);
    fclose(output_file);
    free(data);

    if (!uiuc_inserted) {
        fprintf(stderr, "Error: Failed to insert uiuc chunk\n");
        return 1;
    }

    return 0;
}
