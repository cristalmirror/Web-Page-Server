#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM)<= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)<= 0) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        } 
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
    char *hello = "HTTP/1.1 200 OK\nContnt-Type: text/plan\nContext-Length: 11\nHola Mundo!!!.";

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);


    //1. create the sockets
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    //2. Congiguration directions and ports
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd,(struct sockaddr*) &address, sizeof(address)) < 0) {
        perror("***Error in bind: Need root permissions to the port 433***");
        return 1;
    }

    //3. listen connections
    listen(server_fd, 1);
    while (1) {
        SSL *ssl;
        //4. Accept Connection
        web_socket = accept(server_fd,(struct sockaddr*) &address, (socklen_t*) &address);
        
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl,web_socket);
        
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            SSL_write(ssl, hello, strlen(hello));
        }

        //5. Send respous using write (similar that fwrite)
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(web_socket);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    clear_openssl();
    return 0; 
}