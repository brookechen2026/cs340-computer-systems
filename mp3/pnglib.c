/* pnglib.c - Implementation file for shared PNG functions */
#include "pnglib.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static uint32_t crc_table[256] = {0};

void make_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
        crc_table[i] = crc;
    }
}

uint32_t update_crc(uint32_t crc, const unsigned char *buf, size_t len) {
    if (!crc_table[1]) {
        make_crc_table();
    }
    for (size_t i = 0; i < len; i++) {
        crc = crc_table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

uint32_t calculate_crc32(const unsigned char *data, size_t length) {
    return update_crc(0xFFFFFFFF, data, length) ^ 0xFFFFFFFF;
}

// Validate the PNG signature
int validate_png_signature(FILE *file) {
    if (!file) {
        fprintf(stderr, "Error: File does not exist.\n");
        return 0;
    }
    
    unsigned char signature[PNG_SIGNATURE_SIZE];
    if (fread(signature, 1, PNG_SIGNATURE_SIZE, file) != PNG_SIGNATURE_SIZE) {
        fprintf(stderr, "Error: Failed to read PNG header.\n");
        return 0;
    }
    
    const unsigned char png_signature[PNG_SIGNATURE_SIZE] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (memcmp(signature, png_signature, PNG_SIGNATURE_SIZE) != 0) {
        fprintf(stderr, "Error: Invalid PNG header.\n");
        return 0;
    }
    
    return 1;
}

// Read a PNG chunk with extra credit checks
int read_png_chunk(FILE *file, PNGChunk *chunk) {
    if (fread(&chunk->length, 4, 1, file) != 1) return 0;
    chunk->length = ntohl(chunk->length);
    
    if (fread(chunk->type, 1, 4, file) != 4) return 0;
    chunk->type[4] = '\0';
    
    chunk->data = (unsigned char *)malloc(chunk->length);
    if (!chunk->data && chunk->length > 0) return 0;
    
    if (chunk->length > 0 && fread(chunk->data, 1, chunk->length, file) != chunk->length) return 0;
    
    if (fread(&chunk->crc, 4, 1, file) != 1) return 0;
    chunk->crc = ntohl(chunk->crc);

    // Validate CRC
    uint32_t calculated_crc = update_crc(0xFFFFFFFF, (unsigned char *)chunk->type, 4);
    calculated_crc = update_crc(calculated_crc, chunk->data, chunk->length) ^ 0xFFFFFFFF;
    if (calculated_crc != chunk->crc) {
        fprintf(stderr, "Error: Invalid CRC for chunk %s.\n", chunk->type);
        free_png_chunk(chunk);
        return 0;
    }

    return 1;
}

// Free PNG chunk memory
void free_png_chunk(PNGChunk *chunk) {
    if (chunk->data) {
        free(chunk->data);
    }
}

// Ensure PNG file structure is valid
int check_png_structure(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s.\n", filename);
        return 1;
    }

    if (!validate_png_signature(file)) {
        fclose(file);
        return 1;
    }

    int has_ihdr = 0, has_iend = 0;
    PNGChunk chunk;
    
    while (read_png_chunk(file, &chunk)) {
        if (strcmp(chunk.type, "IHDR") == 0) {
            if (has_ihdr) {
                fprintf(stderr, "Error: Multiple IHDR chunks found.\n");
                free_png_chunk(&chunk);
                fclose(file);
                return 1;
            }
            has_ihdr = 1;

            // Width and height checks
            if (chunk.length >= 8) {
                uint32_t width, height;
                memcpy(&width, chunk.data, 4);
                memcpy(&height, chunk.data + 4, 4);
                width = ntohl(width);
                height = ntohl(height);

                if (width == height) {
                    fprintf(stderr, "Error: Width and height must not be equal.\n");
                    free_png_chunk(&chunk);
                    fclose(file);
                    return 1;
                }
            }
        }

        if (strcmp(chunk.type, "IEND") == 0) {
            has_iend = 1;
        }
        
        free_png_chunk(&chunk);
    }

    if (!has_ihdr) {
        fprintf(stderr, "Error: No IHDR chunk found.\n");
        fclose(file);
        return 1;
    }

    if (!has_iend) {
        fprintf(stderr, "Error: No IEND chunk found.\n");
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

void write_png_chunk(FILE *file, const PNGChunk *chunk) {
    uint32_t length = htonl(chunk->length);
    fwrite(&length, 4, 1, file);
    fwrite(chunk->type, 1, 4, file);
    fwrite(chunk->data, 1, chunk->length, file);

    uint32_t crc = update_crc(0xFFFFFFFF, (unsigned char *)chunk->type, 4);
    crc = update_crc(crc, chunk->data, chunk->length) ^ 0xFFFFFFFF;
    crc = htonl(crc);
    fwrite(&crc, 4, 1, file);
}
