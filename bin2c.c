
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_BZ2
#include <bzlib.h>
#endif

// Function to split file name into base name, header name, and extension.
void separar(const char *nombre_archivo, char **base, char **header, char **ext) {
    char *punto = strrchr(nombre_archivo, '.');
    char *ultimo_separador = strrchr(nombre_archivo, '/');

    if (ultimo_separador == NULL) { 
        ultimo_separador = strrchr(nombre_archivo, '\\');
    }

    if (punto == NULL) {
        *base = strdup(ultimo_separador ? ultimo_separador + 1 : nombre_archivo);
        *header = (char *)malloc(strlen(*base) + 3); // ".h"
        strcpy(*header, *base);
        strcat(*header, ".h");
        *ext = strdup("");
        return;
    }

    size_t len_base = punto - (ultimo_separador ? ultimo_separador + 1 : nombre_archivo);
    *base = (char *)malloc(len_base + 1);
    strncpy(*base, (ultimo_separador ? ultimo_separador + 1 : nombre_archivo), len_base);
    (*base)[len_base] = '\0';

    *header = (char *)malloc(len_base + 4); // ".h" + null terminator
    strcpy(*header, *base);
    strcat(*header, ".h");

    *ext = strdup(punto + 1);
}

// Function to convert binary file to C array.
int bin2c(const char *biname, const char *filename) {
    FILE *f_input, *f_output;
    unsigned char *buf;
    size_t file_size;
    char *base, *nombre, *ext;
    size_t i, need_comma;

    f_input = fopen(filename, "rb");
    if (f_input == NULL) {
        fprintf(stderr, "%s: can't open %s for reading\n", biname, filename);
        return -1;
    }

    fseek(f_input, 0, SEEK_END);
    file_size = ftell(f_input);
    rewind(f_input);

    buf = (unsigned char *)malloc(file_size);
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(f_input);
        return -1;
    }

    if (fread(buf, 1, file_size, f_input) != file_size) {
        fprintf(stderr, "Failed to read file %s\n", filename);
        free(buf);
        fclose(f_input);
        return -1;
    }
    fclose(f_input);

#ifdef USE_BZ2
    unsigned char *bz2_buf;
    unsigned int bz2_size = (file_size + file_size / 100 + 1) + 600;
    bz2_buf = (unsigned char *)malloc(bz2_size);
    if (!bz2_buf) {
        fprintf(stderr, "BZ2 buffer allocation failed\n");
        free(buf);
        return -1;
    }

    int status = BZ2_bzBuffToBuffCompress((char *)bz2_buf, &bz2_size, (char *)buf, file_size, 9, 1, 0);
    if (status != BZ_OK) {
        fprintf(stderr, "Failed to compress data: error %d\n", status);
        free(buf);
        free(bz2_buf);
        return -1;
    }

    free(buf);
    buf = bz2_buf;
    file_size = bz2_size;
#endif

    separar(filename, &base, &nombre, &ext);
    f_output = fopen(nombre, "w");
    if (f_output == NULL) {
        fprintf(stderr, "%s: can't open %s for writing\n", biname, nombre);
        free(buf);
        free(base);
        free(nombre);
        free(ext);
        return -1;
    }

    fprintf(f_output, "const unsigned char %s_%s[%zu] = {", base, ext, file_size);
    need_comma = 0;
    for (i = 0; i < file_size; ++i) {
        if (need_comma) {
            fprintf(f_output, ", ");
        } else {
            need_comma = 1;
        }
        if ((i % 12) == 0) {
            fprintf(f_output, "\n\t");
        }
        fprintf(f_output, "0x%.2x", buf[i]);
    }
    fprintf(f_output, "\n};\n\n");
    fprintf(f_output, "const int %s_%s_length = %zu;\n", base, ext, file_size);

#ifdef USE_BZ2
    fprintf(f_output, "const int %s_%s_length_uncompressed = %zu;\n", base, ext, file_size);
#endif

    printf("%s created.\n", nombre);
    free(buf);
    fclose(f_output);
    free(base);
    free(nombre);
    free(ext);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s binary_file1 binary_file2 ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        if (bin2c(argv[0], argv[i]) != 0) {
            fprintf(stderr, "Error processing file %s\n", argv[i]);
        }
    }

    return 0;
}
