#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <threads.h>


void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void clear_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {

    const SSL_METHOD *method;
    SSL_CTX *ctx;
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("***ERROR: Can't create context SSL***");
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    //load yout certificate and key privated
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM)<= 0 ){
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if(SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM)<= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    
    // Verificar que coincidan
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Error: La llave privada no coincide con el certificado\n");
        exit(EXIT_FAILURE);
    }
}

//modify character that isn't special.
void change_char(char *msg_original, char objetive, char remplaze) {
    for (int i = 0; msg_original[i] != '\0'; i++) {
        if (msg_original[i] == objetive) {
            msg_original[i] = remplaze;
        }
    }
}

//cliente manager and index.html send system
int client_manager(void *ssl_arg) {
    printf("[SERVER]: <<THREAD>> new thread is created\n");
    SSL  *ssl = (SSL *)ssl_arg;

    if (SSL_accept(ssl) <= 0) {
            fprintf(stderr, "Fallo en el handshake SSL\n");
            ERR_print_errors_fp(stderr);
    } else {

        FILE *html_file = fopen("index.html","r"); //open file
        //initial size buffer define
        size_t tam = 1024;
        size_t total_read = 0;
        char *buffer = malloc(tam);

        //manager memory error
        if (buffer == NULL) return 1;
            
        int read_bytes; 
        /*reallocation memory size system:
            this code part has create to case of
            size message is mayor that 1024 bytes 

            if is necesary this system increment 
            exponetly form the size in heap memory
            with realloc() 
        */ 
        while((read_bytes = SSL_read(ssl, buffer + total_read, tam - total_read - 1)) > 0) {
                
            //incrememt in "read bytes size"     
            total_read += read_bytes;

            // comprobation of size buffe
            if (total_read>= tam - 1) {
                tam *= 2;//exponential new size
                char *temp = realloc(buffer, tam); //secure resize allocation
                    
                //if fail, free allocation in buffer to avoid memory keaks
                if (temp == NULL) {
                    free(buffer);
                    return 1;
                }
                buffer = temp;
            }
            buffer[total_read] = '\0';
            //exit for while
            if (strstr(buffer,"\r\n\r\n")) break;
        }

        //search the message in POST method
        char *message_ptr = strstr(buffer,"data=");

        change_char(message_ptr,'+',' ');

        if (message_ptr) {
            printf("[CLIENT]: %s\n",message_ptr + 5);
        }

        free(buffer);
        //read file (index.html) and send to browser.
        if (html_file) {
            //determine file size archive
            fseek(html_file,0,SEEK_END);
            long fsize = ftell(html_file);
            fseek(html_file,0,SEEK_SET);

            //read content of a buffer
            char content[fsize + 1];
            fread(content, 1, fsize, html_file);
            content[fsize] = '\0';
            fclose(html_file); // close all the flow

            //construct the respose HHTP with correct format
            char respose[fsize + 200];
            snprintf(respose, sizeof(respose), "HTTP/1.1 200 OK\r\n"
                                            "Content-Type: text/html\r\n"
                                            "Content-Length: %ld\r\n"
                                            "Connection: close\r\n"
                                            "\r\n" // this line is void, obligatory
                                            "%s",fsize,content);

            
            SSL_write(ssl, respose, strlen(respose));
        }
    }

    //5. Send respous using write (similar that fwrite)
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return thrd_success;
}

//main functions
int main(int argc, char *argv[]) {
    //define elemnts
    int server_fd, web_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    uint16_t port;
    //define port for argumnets
    if (argc > 1) {
        port = (uint16_t) strtoul(argv[1], NULL, 10);
    }
    

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);
    const char *ip_str = argv[2];

    //1. create the sockets
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //2. Congiguration directions and ports
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip_str);
    address.sin_port = htons(port);
    
    if (bind(server_fd,(struct sockaddr*) &address, sizeof(address)) < 0) {
        perror("[SERVER]: ***Error in bind: Need root permissions to the port\n***");
        return 1;
    }

    struct sockaddr_in local_address;
    socklen_t address_leng = sizeof(local_address);

    if (getsockname(server_fd,(struct sockaddr *) &local_address, &address_leng)) {
        perror("[SERVER]: ***Error trying take the congigurations socket***\n");
    } else {
        char *ip = inet_ntoa(local_address.sin_addr);
        uint16_t real_port = ntohs(local_address.sin_port);
        
        printf("[SERVER]: succesfull init server \n");
        printf("[SERVER]: IP => %s\n",ip);
        printf("[SERVER]: PORT => %u\n",real_port);
    }

    //3. listen connections
    listen(server_fd, 1000);
    while (1) {
        SSL *ssl;
        //4. Accept Connection
        web_socket = accept(server_fd,(struct sockaddr*) &address, &address_leng);
        
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl,web_socket);
        
        //thread definition
        thrd_t id_thread;
        
        //create the thread pass the object ssl how argument
        if (thrd_create(&id_thread, client_manager, ssl) == thrd_success) {
            //disengage the thread for that not block the server
            thrd_detach(id_thread);
        } else {
            fprintf(stderr,"[SERVER]: ***Error creating thread*** \n");
        }
        
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    clear_openssl();
    return 0; 
}