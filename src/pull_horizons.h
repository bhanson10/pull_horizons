// pull_horizons.h

#ifndef PULL_HORIZONS_H
#define PULL_HORIZONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

typedef struct {
    char* data;
    size_t size;
}MemoryBlock;

size_t url_encoded_length(char *str);

char *url_encode(char *str);

char* build_url(int size, char** link_strs);

size_t write_to_memory(void* ptr, size_t size, size_t nmemb, void* userp);

char** split_lines(const char* input, size_t* num_lines_out);

void pull_horizons(char* target_body, char* ephem_type, char* center, char* ref_plane, char* start, char* stop, char* step, char* units, char* vec_table_set, char* file_name, int date_type);

void overwrite_line(FILE *fp, long pos, size_t original_len, const char *new_content);

void pull_horizons_irreg(char* target_body, char* ephem_type, char* center, char* ref_plane, char* units, char* vec_table_set, char* file_name, int N, char* t_file_name);

#endif // PULL_HORIZONS_H
