#include"./include/download_operations.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int manager_downloads_archive(SSL *ssl, const char *filepath) {

    printf("[SERVER]: Downloads manager system running\n");
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("[SERVER]: ***Error opening file***");
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        SSL_write(ssl, not_found, strlen(not_found));
        return thrd_error;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_data = malloc(fsize);
    if (file_data == NULL) {
        fclose(file);
        return thrd_nomem;
    }
    fread(file_data, 1, fsize, file);
    fclose(file);

    // extract just the filename for Content-Disposition
    const char *fname = strrchr(filepath, '/');
    fname = fname ? fname + 1 : filepath;

    int tam = snprintf(NULL, 0,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Disposition: attachment; filename=\"%s\"\r\n"
            "Content-Length: %ld\r\n"
            "\r\n", fname, fsize);
    char *header = malloc(tam + 1);
    if (header == NULL) {
        free(file_data);
        return thrd_nomem;
    }
    snprintf(header, tam + 1,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Disposition: attachment; filename=\"%s\"\r\n"
            "Content-Length: %ld\r\n"
            "\r\n", fname, fsize);

    SSL_write(ssl, header, strlen(header));
    SSL_write(ssl, file_data, fsize);

    free(file_data);
    free(header);
    return thrd_success;
}
