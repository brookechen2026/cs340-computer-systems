#include "utf8lib.h"
#include <stdlib.h>
#include <string.h>

typedef struct Replacement {
    char *from;
    char *to;
    struct Replacement *next;
} Replacement;

struct _replacement_set_t {
    Replacement *head;
};

int decodeCharacter(const char **utf8) {
    if (!utf8 || !*utf8) return -1;

    const unsigned char *ptr = (const unsigned char *) *utf8;
    int codepoint = 0;
    int num_bytes = 0;

    if (ptr[0] < 0x80) { 
        codepoint = ptr[0];
        num_bytes = 1;
    } else if ((ptr[0] & 0xE0) == 0xC0) { 
        if ((ptr[1] & 0xC0) != 0x80) return -1;
        codepoint = ((ptr[0] & 0x1F) << 6) | (ptr[1] & 0x3F);
        num_bytes = 2;
    } else if ((ptr[0] & 0xF0) == 0xE0) { 
        if ((ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80) return -1;
        codepoint = ((ptr[0] & 0x0F) << 12) | ((ptr[1] & 0x3F) << 6) | (ptr[2] & 0x3F);
        num_bytes = 3;
    } else if ((ptr[0] & 0xF8) == 0xF0) { 
        if ((ptr[1] & 0xC0) != 0x80 || (ptr[2] & 0xC0) != 0x80 || (ptr[3] & 0xC0) != 0x80) return -1;
        codepoint = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3F) << 12) | ((ptr[2] & 0x3F) << 6) | (ptr[3] & 0x3F);
        num_bytes = 4;
    } else {
        return -1;
    }

    if (codepoint > 0x10FFFF) return -1;
    *utf8 += num_bytes;
    return codepoint;
}

int encodeCharacter(char **utf8, size_t *space, int codepoint) {
    if (!utf8 || !*utf8 || !space || codepoint < 0 || codepoint > 0x10FFFF) return 0;

    unsigned char *ptr = (unsigned char *) *utf8;
    int num_bytes = 0;

    if (codepoint < 0x80) { 
        if (*space < 1) return 0;
        ptr[0] = (char)codepoint;
        num_bytes = 1;
    } else if (codepoint < 0x800) { 
        if (*space < 2) return 0;
        ptr[0] = 0xC0 | (codepoint >> 6);
        ptr[1] = 0x80 | (codepoint & 0x3F);
        num_bytes = 2;
    } else if (codepoint < 0x10000) { 
        if (*space < 3) return 0;
        ptr[0] = 0xE0 | (codepoint >> 12);
        ptr[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        ptr[2] = 0x80 | (codepoint & 0x3F);
        num_bytes = 3;
    } else { 
        if (*space < 4) return 0;
        ptr[0] = 0xF0 | (codepoint >> 18);
        ptr[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        ptr[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        ptr[3] = 0x80 | (codepoint & 0x3F);
        num_bytes = 4;
    }

    *utf8 += num_bytes;
    *space -= num_bytes;
    return num_bytes;
}

size_t strlen8c(const char *s) {
    size_t count = 0;
    while (*s) {
        int codepoint = decodeCharacter(&s);
        if (codepoint < 0) break;
        count++;
    }
    return count;
}

size_t strlen8i(const int *s) {
    size_t count = 0;
    while (*s) {
        if (*s < 0 || *s > 0x10FFFF) {
            break; 
        }
        count += (*s < 0x80) ? 1 : (*s < 0x800) ? 2 : (*s < 0x10000) ? 3 : 4;
        s++;
    }
    return count;
}

ReplacementSet *newReplacementSet() {
    ReplacementSet *set = (ReplacementSet *)malloc(sizeof(ReplacementSet));
    set->head = NULL;
    return set;
}

void deleteReplacementSet(ReplacementSet *set) {
    if (!set) return;
    Replacement *current = set->head;
    while (current) {
        Replacement *temp = current;
        current = current->next;
        free(temp->from);
        free(temp->to);
        free(temp);
    }
    free(set);
}

void addReplacement(ReplacementSet *set, const char *from, const char *to) {
    if (!set || !from || !to) return;
    Replacement *newNode = (Replacement *)malloc(sizeof(Replacement));
    if (!newNode) return;

    newNode->from = strdup(from);
    newNode->to = strdup(to);
    newNode->next = set->head;
    set->head = newNode;
}

char *replaceAll(ReplacementSet *set, const char *input) {
    if (!set || !input) return NULL;

    size_t input_len = strlen(input);
    char *output = (char *)malloc(input_len * 4 + 1);
    if (!output) return NULL;

    size_t out_index = 0, i = 0;

    while (i < input_len) {
        Replacement *best_match = NULL;
        size_t best_length = 0;

        for (Replacement *cur = set->head; cur; cur = cur->next) {
            size_t from_len = strlen(cur->from);
            if (strncmp(&input[i], cur->from, from_len) == 0) {
                if (!best_match || from_len > best_length) {
                    best_match = cur;
                    best_length = from_len;
                }
            }
        }

        if (best_match) {
            size_t to_len = strlen(best_match->to);
            memcpy(&output[out_index], best_match->to, to_len);
            out_index += to_len;
            i += best_length;
        } else {
            output[out_index++] = input[i++];
        }
    }

    output[out_index] = '\0';
    return output;
}
