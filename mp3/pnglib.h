#ifndef PNGLIB_H
#define PNGLIB_H

#include <stdio.h>
#include <stdint.h>

#define PNG_SIGNATURE_SIZE 8
#define CHUNK_HEADER_SIZE 8
#define CHUNK_CRC_SIZE 4
#define UIUC_CHUNK_TYPE "uiuc"

typedef struct {
    uint32_t length;
    char type[5];  
    unsigned char *data;
    uint32_t crc;
} PNGChunk;

int validate_png_signature(FILE *file);
int read_png_chunk(FILE *file, PNGChunk *chunk);
void free_png_chunk(PNGChunk *chunk);
//void write_png_chunk(FILE *file, PNGChunk *chunk);
void write_png_chunk(FILE *file, const PNGChunk *chunk);

uint32_t calculate_crc32(const unsigned char *data, size_t length);

void make_crc_table(void);
uint32_t update_crc(uint32_t crc, const unsigned char *buf, size_t len);

// **添加 check_png_structure 函数声明**
int check_png_structure(const char *filename);

void list_png_chunks(const char *filename);
int extract_uiuc_chunk(const char *input_filename, const char *output_filename);
int insert_uiuc_chunk(const char *input_filename, const char *data_filename, const char *output_filename);

#endif /* PNGLIB_H */
