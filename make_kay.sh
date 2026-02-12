#!/bin/bash

# 1. Uso del Sha-Bang para indicar el intérprete (Bash) [1].
# 2. Verificación y asignación de parámetros posicionales ($1) [1, 2].

# Comprobar si se proporcionó el parámetro de la IP
if [ -z "$1" ]; then
    echo "Error: No se ha proporcionado la dirección IP."
    echo "Uso: $0 <ipejemplo>"
    exit 1
fi

# Asignar el primer parámetro a una variable para mayor claridad
IP_EJEMPLO=$1

echo "Iniciando generación de certificados para la IP: $IP_EJEMPLO"

# --- Paso A: Crear tu propia entidad (La "Root CA") ---
echo "Generando Root CA..."
# Crear la llave de la autoridad
openssl genrsa -out MyLocalCA.key 2048

# Crear el certificado de la autoridad
openssl req -x509 -new -nodes -key MyLocalCA.key -sha256 -days 1825 -out MyLocalCA.pem \
-subj "/C=AR/ST=PBA/L=La Plata/O=MiProyecto/CN=Nostromus Root CA"

# --- Paso B: Crear y Firmar el certificado del servidor (No Autofirmado) ---
echo "Generando y firmando certificado del servidor..."

# 1. Crear la llave del servidor
openssl genrsa -out server.key 2048

# 2. Crear el pedido de firma (CSR) usando la variable dinámica de la IP
openssl req -new -key server.key -out server.csr \
-subj "/C=AR/ST=PBA/L=La Plata/CN=$IP_EJEMPLO"

# 3. Firmar el certificado usando la CA local creada anteriormente
openssl x509 -req -in server.csr -CA MyLocalCA.pem -CAkey MyLocalCA.key \
-CAcreateserial -out server.crt -days 825 -sha256

echo "Proceso finalizado. Se han generado los archivos: MyLocalCA.pem, server.key y server.crt"