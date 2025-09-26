#include "pull_horizons.h"

size_t url_encoded_length(char *str){
    size_t len = 0;
    while (*str) {
        if (*str == ' ' || *str == '@' || *str == '\'') {
            len += 3;  // %20, %40, %27
        } else {
            len += 1;
        }
        str++;
    }
    return len;
}

// Actual encoding function
char* url_encode(char *str){
    size_t new_len = url_encoded_length(str);
    char *encoded = malloc(new_len + 1);  // +1 for '\0'
    if (!encoded) return NULL;

    char *out = encoded;
    while (*str) {
        switch (*str) {
            case ' ':
                strcpy(out, "%20"); out += 3; break;
            case '@':
                strcpy(out, "%40"); out += 3; break;
            case '\'':
                strcpy(out, "%27"); out += 3; break;
            default:
                *out++ = *str; break;
        }
        str++;
    }
    *out = '\0';
    return encoded;
}

char* build_url(int size, char** url_strs){
    size_t total_len = 0;
    for (int i = 0; i < size; ++i) {
        total_len += strlen(url_strs[i]);
    }
    total_len += size - 1;  // for size - 1 '&'s between size strings
    total_len += 1;  // null terminator

    char* url = malloc(total_len);
    if (url == NULL) {
        fprintf(stderr, "Error: malloc in build_url failed.\n");
        exit(1);  // Exit with error code 1
    }

    url[0] = '\0'; // Start with empty string
    for (int i = 0; i < size; ++i) {
        strcat(url, url_strs[i]);
    }

    return url; 
}

size_t write_to_memory(void* ptr, size_t size, size_t nmemb, void* userp){
    size_t total = size * nmemb;
    MemoryBlock *mem = (MemoryBlock *)userp;

    char *temp = realloc(mem->data, mem->size + total + 1); // +1 for null terminator
    if (temp == NULL) return 0; // realloc failed

    mem->data = temp;
    memcpy(&(mem->data[mem->size]), ptr, total);
    mem->size += total;
    mem->data[mem->size] = '\0'; // Null-terminate

    return total;
}

char** split_lines(const char* input, size_t* num_lines_out){
    size_t lines = 0;
    const char *ptr = input;

    // Count number of lines
    while (*ptr) {
        if (*ptr == '\n') lines++;
        ptr++;
    }

    // Allocate array of char*
    char** output = malloc(lines * sizeof(char*));
    if (!output) return NULL;

    size_t idx = 0;
    const char *start = input;
    while (*input && idx < lines) {
        if (*input == '\n') {
            size_t len = input - start;
            output[idx] = malloc(len + 1);
            strncpy(output[idx], start, len);
            output[idx][len] = '\0';
            idx++;
            start = input + 1;
        }
        input++;
    }

    *num_lines_out = idx;
    return output;
}

char* add_one_second(char *input){
    static char output[32]; // enough for our format
    struct tm t = {0};

    if (sscanf(input, "%d-%d-%d %d:%d:%d",
               &t.tm_year, &t.tm_mon, &t.tm_mday,
               &t.tm_hour, &t.tm_min, &t.tm_sec) != 6) {
        return NULL;
    }

    t.tm_year -= 1900;
    t.tm_mon  -= 1;

    time_t time_val = mktime(&t);
    if (time_val == -1) return NULL;

    time_val += 1;
    struct tm *new_time = localtime(&time_val);

    // safer than snprintf
    strftime(output, sizeof(output), "%Y-%m-%d %H:%M:%S", new_time);

    return output;
}

void extract_state_after_SOE(char **lines, double *state){
    if (!lines || !state) return;

    for (int i = 0; lines[i] != NULL; i++) {
        if (strcmp(lines[i], "$$SOE") == 0) {
            // Ensure at least two more lines exist
            if (lines[i+2] == NULL) return;

            int n = sscanf(lines[i+2],
                           "%*s : %lf %lf %lf %lf %lf %lf",
                           &state[0], &state[1], &state[2],
                           &state[3], &state[4], &state[5]);
                
            if (n != 6) return; // parsing failed
            return;             // success
        }
    }
    return ; // "$$SOE" not found
}

void pull_horizons(char* target_body, char* ephem_type, char* center, char* ref_plane, char* start, char* stop, char* step, char* units, char* vec_table_set, char* file_name, int date_type){

    // fixed parameters
    char* FORMAT = "text";

    // configurable parameters
    char* COMMAND = url_encode(target_body); 
    char* EPHEM_TYPE; int chunk_size; 
    if(strcmp(ephem_type, "Observer Table") == 0){
        EPHEM_TYPE = "OBSERVER"; 
        chunk_size = 1; 
    }else if(strcmp(ephem_type, "Vector Table") == 0){
        EPHEM_TYPE = "VECTORS"; 
        chunk_size = 2; 
    }else if(strcmp(ephem_type, "Osculating Orbital Elements") == 0){
        EPHEM_TYPE = "ELEMENTS"; 
        chunk_size = 5; 
    }else if(strcmp(ephem_type, "Small-Body SPK File") == 0){
        EPHEM_TYPE = "SPK"; 
    }else if(strcmp(ephem_type, "Approach") == 0){
        EPHEM_TYPE = "APPROACH"; 
    }else{
        fprintf(stderr, "Error: input ephem_type is not of the acceptable options. Note: options are case-sensitive.\n");
        exit(1);  // Exit with error code 1
    }
    char* CENTER = url_encode(center); 
    char* REF_PLANE; 
    if(strcmp(ref_plane, "Ecliptic") == 0){
        REF_PLANE = "E"; 
    }else if(strcmp(ref_plane, "Equatorial") == 0){
        REF_PLANE = "F"; 
    }else if(strcmp(ref_plane, "Body Mean") == 0){
        REF_PLANE = "B"; 
    }else{
        fprintf(stderr, "Error: input ref_plane is not of the acceptable options. Note: options are case-sensitive.\n");
        exit(1);  // Exit with error code 1
    }
    char* START_TIME = url_encode(start); 
    char* STOP_TIME = url_encode(stop); 
    char* STEP_SIZE = url_encode(step); 
    char* OUT_UNITS = url_encode(units); 
    char* VEC_TABLE = vec_table_set;
    if (vec_table_set != NULL){
        if(strchr(VEC_TABLE, '1') || strchr(VEC_TABLE, '2')){
            if(strchr(VEC_TABLE, 'x')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'a')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'r')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'p')){
                chunk_size++;
            }
        }
    }

    if (vec_table_set != NULL){
        if(strchr(VEC_TABLE, '2') && chunk_size == 2) chunk_size++; // state is double-lined when VEC_TABLE=='2'
    }

    char* url_pre  = "https://ssd.jpl.nasa.gov/api/horizons.api?"; 

    char* horizons_url = NULL; 
    if(vec_table_set == NULL){
        char* url_strs[] = {url_pre, 
                            "format=", FORMAT, 
                            "&COMMAND=%27", COMMAND, 
                            "%27&EPHEM_TYPE=%27", EPHEM_TYPE, 
                            "%27&CENTER=%27", CENTER, 
                            "%27&REF_PLANE=%27", REF_PLANE, 
                            "%27&START_TIME=%27", START_TIME, 
                            "%27&STOP_TIME=%27", STOP_TIME, 
                            "%27&STEP_SIZE=%27", STEP_SIZE,
                            "%27&OUT_UNITS=%27", OUT_UNITS, 
                            "%27"};
        horizons_url = build_url(20, url_strs);
    }else{
        char* url_strs[] = {url_pre, 
                            "format=", FORMAT, 
                            "&COMMAND=%27", COMMAND, 
                            "%27&EPHEM_TYPE=%27", EPHEM_TYPE,
                            "%27&CENTER=%27", CENTER, 
                            "%27&REF_PLANE=%27", REF_PLANE, 
                            "%27&START_TIME=%27", START_TIME, 
                            "%27&STOP_TIME=%27", STOP_TIME, 
                            "%27&STEP_SIZE=%27", STEP_SIZE, 
                            "%27&OUT_UNITS=%27", OUT_UNITS, 
                            "%27&VEC_TABLE=%27", VEC_TABLE, 
                            "%27"};
        horizons_url = build_url(22, url_strs);
    }
    // printf("\n%s\n", horizons_url); 

    CURL* curl;
    CURLcode res;
    MemoryBlock chunk = {NULL, 0};

    curl = curl_easy_init();
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, horizons_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            exit(1);  // Exit with error code 1
        }
        curl_easy_cleanup(curl);

        // Split into lines
        size_t num_lines;
        char **lines = split_lines(chunk.data, &num_lines);

        // Open output file
        FILE *fp = NULL;
        if (date_type == 0 || date_type == 1) {
            fp = fopen(file_name, "w");  // overwrite
        } else if (date_type == 2 || date_type == 3) {
            fp = fopen(file_name, "a");  // append
        }
        if (!fp) {
            fprintf(stderr, "Error: failed to open output file.\n");
            exit(1);  // Exit with error code 1
        }

        // Process based on date_type
        int i;
        int soe_index = -1;
        for (i = 0; i < num_lines; i++) {
            if (strncmp(lines[i], "$$SOE", 5) == 0) {
                soe_index = i;
                break;
            }
        }

        switch(date_type){
            case 0: // Write entire file
                printf("\n%s\n", horizons_url); 

                // Write entire file
                for (i = 0; i < num_lines; i++) {
                    fprintf(fp, "%s\n", lines[i]);
                }
                break;

            case 1: // Write start of file through first epoch to txt file
                if (soe_index != -1) {
                    for (i = 0; i <= soe_index + chunk_size && i < num_lines; i++) {
                        fprintf(fp, "%s\n", lines[i]);
                    }
                }
                break;

            case 2: // Append first epoch to txt 
                if (soe_index != -1) {
                    for (i = soe_index + 1; i <= soe_index + chunk_size && i < num_lines; i++) {
                        fprintf(fp, "%s\n", lines[i]);
                    }
                }
                break;

            case 3:{ // Append last epoch through end of file to txt file
                int eoe_index = -1;
                for (i = 0; i < num_lines; i++) {
                    if (strncmp(lines[i], "$$EOE", 5) == 0) {
                        eoe_index = i;
                        break;
                    }
                }
            
                if(eoe_index != -1){
                    int start_index = (eoe_index >= chunk_size) ? eoe_index - chunk_size : 0;
                    for (i = start_index; i < num_lines; i++) {
                        fprintf(fp, "%s\n", lines[i]);
                    }
                }else{
                    fprintf(stderr, "Warning: '$$EOE' not found in response.\n");
                    exit(1);  // Exit with error code 1
                }
                break;
            }

            default:
                fprintf(stderr, "Invalid date_type: %d\n", date_type);
                fclose(fp);
                exit(1);  // Exit with error code 1
        }
        fclose(fp);

        // Cleanup memory
        for (i = 0; i < num_lines; i++) {
            free(lines[i]);
        }
        free(lines);
        free(chunk.data);

    }else{
        fprintf(stderr, "Error: failed to initialize libcurl.\n");
        exit(1);  // Exit with error code 1
    }

    return; 
}

void overwrite_line(FILE *fp, long pos, size_t original_len, const char *new_content){
    char buffer[256];
    size_t new_len = strlen(new_content);

    // Clamp to original_len - 1 to make space for '\n'
    if (new_len >= original_len) {
        // If too long, truncate and ensure newline
        memcpy(buffer, new_content, original_len - 1);
        buffer[original_len - 1] = '\n';
    } else {
        // If shorter, copy and pad with spaces
        memset(buffer, ' ', original_len);
        memcpy(buffer, new_content, new_len);
        buffer[original_len - 1] = '\n';
    }

    fseek(fp, pos - original_len, SEEK_SET);
    fwrite(buffer, 1, original_len, fp);
    fflush(fp);
}

void pull_horizons_single(char* target_body, char* ephem_type, char* center, char* ref_plane, char* start, char* units, char* vec_table_set, double* state, int dim){

    char* stop = add_one_second(start); 

    // fixed parameters
    char* FORMAT = "text";

    // configurable parameters
    char* COMMAND = url_encode(target_body); 
    char* EPHEM_TYPE; int chunk_size; 
    if(strcmp(ephem_type, "Observer Table") == 0){
        EPHEM_TYPE = "OBSERVER"; 
        chunk_size = 1; 
    }else if(strcmp(ephem_type, "Vector Table") == 0){
        EPHEM_TYPE = "VECTORS"; 
        chunk_size = 2; 
    }else if(strcmp(ephem_type, "Osculating Orbital Elements") == 0){
        EPHEM_TYPE = "ELEMENTS"; 
        chunk_size = 5; 
    }else if(strcmp(ephem_type, "Small-Body SPK File") == 0){
        EPHEM_TYPE = "SPK"; 
    }else if(strcmp(ephem_type, "Approach") == 0){
        EPHEM_TYPE = "APPROACH"; 
    }else{
        fprintf(stderr, "Error: input ephem_type is not of the acceptable options. Note: options are case-sensitive.\n");
        exit(1);  // Exit with error code 1
    }
    char* CENTER = url_encode(center); 
    char* REF_PLANE; 
    if(strcmp(ref_plane, "Ecliptic") == 0){
        REF_PLANE = "E"; 
    }else if(strcmp(ref_plane, "Equatorial") == 0){
        REF_PLANE = "F"; 
    }else if(strcmp(ref_plane, "Body Mean") == 0){
        REF_PLANE = "B"; 
    }else{
        fprintf(stderr, "Error: input ref_plane is not of the acceptable options. Note: options are case-sensitive.\n");
        exit(1);  // Exit with error code 1
    }
    char* START_TIME = url_encode(start); 
    char* STOP_TIME = url_encode(stop); 
    char* OUT_UNITS = url_encode(units); 
    char* VEC_TABLE = vec_table_set;
    if (vec_table_set != NULL){
        if(strchr(VEC_TABLE, '1') || strchr(VEC_TABLE, '2')){
            if(strchr(VEC_TABLE, 'x')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'a')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'r')){
                chunk_size++;
            }
            if(strchr(VEC_TABLE, 'p')){
                chunk_size++;
            }
        }
    }

    if (vec_table_set != NULL){
        if(strchr(VEC_TABLE, '2') && chunk_size == 2) chunk_size++; // state is double-lined when VEC_TABLE=='2'
    }

    char* url_pre  = "https://ssd.jpl.nasa.gov/api/horizons.api?"; 

    char* horizons_url = NULL; 
    if(vec_table_set == NULL){
        char* url_strs[] = {url_pre, 
                            "format=", FORMAT, 
                            "&COMMAND=%27", COMMAND, 
                            "%27&EPHEM_TYPE=%27", EPHEM_TYPE, 
                            "%27&CENTER=%27", CENTER, 
                            "%27&REF_PLANE=%27", REF_PLANE, 
                            "%27&START_TIME=%27", START_TIME, 
                            "%27&STOP_TIME=%27", STOP_TIME, 
                            "%27&OUT_UNITS=%27", OUT_UNITS, 
                            "%27"};
        horizons_url = build_url(18, url_strs);
    }else{
        char* url_strs[] = {url_pre, 
                            "format=", FORMAT, 
                            "&COMMAND=%27", COMMAND, 
                            "%27&EPHEM_TYPE=%27", EPHEM_TYPE,
                            "%27&CENTER=%27", CENTER, 
                            "%27&REF_PLANE=%27", REF_PLANE, 
                            "%27&START_TIME=%27", START_TIME, 
                            "%27&STOP_TIME=%27", STOP_TIME, 
                            "%27&OUT_UNITS=%27", OUT_UNITS, 
                            "%27&VEC_TABLE=%27", VEC_TABLE, 
                            "%27"};
        horizons_url = build_url(20, url_strs);
    }
    // printf("\n%s\n", horizons_url); 

    CURL* curl;
    CURLcode res;
    MemoryBlock chunk = {NULL, 0};

    curl = curl_easy_init();
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, horizons_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);
        if(res != CURLE_OK){
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            exit(1);  // Exit with error code 1
        }
        curl_easy_cleanup(curl);

        // Split into lines
        size_t num_lines;
        char **lines = split_lines(chunk.data, &num_lines);

        extract_state_after_SOE(lines, state);

        // Cleanup memory
        for (int i = 0; i < num_lines; i++) {
            free(lines[i]);
        }
        free(lines);
        free(chunk.data);

    }else{
        fprintf(stderr, "Error: failed to initialize libcurl.\n");
        exit(1);  // Exit with error code 1
    }

    return; 
}

void pull_horizons_irreg(char* target_body, char* ephem_type, char* center, char* ref_plane, char* units, char* vec_table_set, char* file_name, int N, char* t_file_name){
    
    char* step  = "1"; 

    FILE* fp = fopen(t_file_name, "r");
    if (t_file_name == NULL) {
        fprintf(stderr, "Error: could not open file %s", t_file_name);
        exit(1);
    }

    char line_f[256];
    char line_p[256];
    char line[256];
    fgets(line_p, sizeof(line_p), fp); line_p[strcspn(line_p, "\n")] = '\0'; strcpy(line_f, line_p);

    for(int i = 0; i < N; i++){
        if(i == 0){
            fgets(line, sizeof(line), fp); line[strcspn(line, "\n")] = '\0';
            pull_horizons(target_body, ephem_type, center, ref_plane, line_p, line, step, units, vec_table_set, file_name, 1); 
        }else if(i == (N - 1)){
            pull_horizons(target_body, ephem_type, center, ref_plane, line_p, line, step, units, vec_table_set, file_name, 3); 
        }else{
            fgets(line, sizeof(line), fp); line[strcspn(line, "\n")] = '\0';
            pull_horizons(target_body, ephem_type, center, ref_plane, line_p, line, step, units, vec_table_set, file_name, 2); 
        }

        if(i != (N - 2)){
            strcpy(line_p, line);
        }
    }

    // change start, stop, and step values in ephemeris intro
    fp = fopen(file_name, "r+");  // r+ allows read and write
    if (!fp) {
        fprintf(stderr, "Error: opening output file.\n");
        exit(1);
    }
    char line_buf[256];
    long pos;
    int line_num = 0;
    while (fgets(line_buf, sizeof(line_buf), fp) && line_num < 100) {
        pos = ftell(fp);  // Position *after* the line
        size_t line_len = strlen(line_buf);

        if (strncmp(line_buf, "Start time", 10) == 0) {
            char new_line[1000];
            snprintf(new_line, sizeof(new_line), "Start time : A.D. %s TDB", line_f);
            overwrite_line(fp, pos, line_len, new_line);
        } else if (strncmp(line_buf, "Stop  time", 10) == 0) {
            char new_line[1000];
            snprintf(new_line, sizeof(new_line), "Stop  time : A.D. %s TDB", line);
            overwrite_line(fp, pos, line_len, new_line);
        } else if (strncmp(line_buf, "Step-size", 9) == 0) {
            char new_line[1000];
            snprintf(new_line, sizeof(new_line), "Steps : %.6e", (double)N);
            overwrite_line(fp, pos, line_len, new_line);
        }
        line_num++;
    }
    fclose(fp);

    return; 
}


