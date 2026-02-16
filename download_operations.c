#include"download_operations.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int manager_downloads_archive(SSL *ssl) {
    //open the archive in binary mode "rb" for avoid alterations
    FILE *file = fopen("/executables/aesexe","rb");
    if (file == NULL) {
        perror("[SERVER]: ***Error to opern archive***");
        return thrd_error;
    }

    //determine size of archive
    fseek(file, 0,SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    //reserv dinamic memory for the content 
    char *file_data = malloc(fsize);
    if (file_data == NULL) {
        fclose(file);
        return thrd_nomem;
    }
    //read the archive and close the flow
    fread(file_data, 1, fsize, file);
    fclose(file);
    //Prepare HTTP headers (we set them to 512 bytes for security)
    char header[12];
    snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Disposition: attachment; filename=\"aesexe\"\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",fsize);
    //Secuere send midel SSL
    SSL_write(ssl, header, strlen(header));
    SSL_write(ssl, file_data, fsize);

    //clean the memory
    free(file_data);
    return thrd_success;
}