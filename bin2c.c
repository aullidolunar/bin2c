/*
 * This is bin2c program, which allows you to convert binary file to
 * C language array, for use as embedded resource, for instance you can
 * embed graphics or audio file directly into your program.
 * This is public domain software, use it on your own risk.
 * Contact Serge Fukanchik at fuxx@mail.ru  if you have any questions.
 *
 * Some modifications were made by Gwilym Kuiper (kuiper.gwilym@gmail.com)
 * I have decided not to change the licence.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_BZ2
#include <bzlib.h>
#endif

void separar(const char *nombre_archivo, char **base, char **header, char **ext) {
// Encontrar el último punto y el último separador de directorio
    char *punto = strrchr(nombre_archivo, '.');
    char *ultimo_separador = strrchr(nombre_archivo, '/');
    if (ultimo_separador == NULL) {
        ultimo_separador = strrchr(nombre_archivo, '\\');
    }

    // Si no hay punto, el archivo no tiene extensión
    if (punto == NULL) {
        // Si hay un separador, tomamos lo que está después
        if (ultimo_separador != NULL) {
            *base = strdup(ultimo_separador + 1);
        } else {
            *base = strdup(nombre_archivo);
        }
        *header = strdup(*base);
        strcat(*header, ".h");
        *ext = strdup("");
        return;
    }

    // Calcular las longitudes (considerando el posible separador)
    size_t len_base = punto - (ultimo_separador ? ultimo_separador + 1 : nombre_archivo);
    size_t len_ext = strlen(punto + 1);

    // Asignar memoria y copiar las partes correspondientes (similar al código original)
    *base = (char *)malloc(len_base + 1);
    *header = (char *)malloc(len_base + 5);
    *ext = (char *)malloc(len_ext + 1);

    strncpy(*base, (ultimo_separador ? ultimo_separador + 1 : nombre_archivo), len_base);
    (*base)[len_base] = '\0';

    strncpy(*header, *base, len_base);
    strcpy(*header + len_base, ".h");

    strcpy(*ext, punto + 1);
}

int
main(int argc, char *argv[])
{
    char *nombre;
    char *base;
    char *ext;
    char *buf;
    unsigned int i, file_size, need_comma;

    FILE *f_input, *f_output;

#ifdef USE_BZ2
    char *bz2_buf;
    unsigned int uncompressed_size, bz2_size;
#endif

    if (argc < 2) {
        fprintf(stderr, "Usage: %s binary_file\n", argv[0]);
        return 0;
    }

    f_input = fopen(argv[1], "rb");
    if (f_input == NULL) {
        fprintf(stderr, "%s: can't open %s for reading\n", argv[0], argv[1]);
        return 0;
    }

    // Get the file length
    fseek(f_input, 0, SEEK_END);
    file_size = ftell(f_input);
    fseek(f_input, 0, SEEK_SET);

    buf = (char *) malloc(file_size);
    assert(buf);

    fread(buf, file_size, 1, f_input);
    fclose(f_input);

#ifdef USE_BZ2
    // allocate for bz2.
    bz2_size =
      (file_size + file_size / 100 + 1) + 600; // as per the documentation

    bz2_buf = (char *) malloc(bz2_size);
    assert(bz2_buf);

    // compress the data
    int status =
      BZ2_bzBuffToBuffCompress(bz2_buf, &bz2_size, buf, file_size, 9, 1, 0);

    if (status != BZ_OK) {
        fprintf(stderr, "Failed to compress data: error %i\n", status);
        return -1;
    }

    // and be very lazy
    free(buf);
    uncompressed_size = file_size;
    file_size = bz2_size;
    buf = bz2_buf;
#endif
  // ++++++++++++++++++++ generar auto header
  separar(argv[1], &base, &nombre, &ext);
  
    f_output = fopen(nombre, "w");
    if (f_output == NULL) {
        fprintf(stderr, "%s: can't open %s for writing\n", argv[0], argv[2]);
        free (base);
        free(nombre);
        free(ext);
        return 0;
    }

    need_comma = 0;

    fprintf(f_output, "const unsigned char %s_%s[%i] = {", base, ext, file_size);
    for (i = 0; i < file_size; ++i) {
        if (need_comma)
            fprintf(f_output, ", ");
        else
            need_comma = 1;
        if ((i % 11) == 0)
            fprintf(f_output, "\n\t");
        fprintf(f_output, "0x%.2x", buf[i] & 0xff);
    }
    fprintf(f_output, "\n};\n\n");

    fprintf(f_output, "const int %s_%s_length = %i;\n", base, ext, file_size);

#ifdef USE_BZ2
    fprintf(f_output, "const int %s_%s_length_uncompressed = %i;\n", base, ext,
            uncompressed_size);
#endif

      free (base);
        free(nombre);
        free(ext);
    fclose(f_output);

    return 0;
}
