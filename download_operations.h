#ifndef DOWNLOAD_OPERATIONS_H
#define DOWNLOAD_OPERATIONS_H

#include <openssl/ssl.h>
#include <threads.h>

int manager_downloads_archive(SSL *ssl);

#endif
