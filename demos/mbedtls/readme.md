# readme.md

```bash
# Generate the CA private key:
openssl genpkey -algorithm RSA -out ca.key
# Generate the CA certificate:
openssl req -x509 -new -nodes -key ca.key -sha256 -days 365 -out ca.crt

# Generate the server private key:
openssl genpkey -algorithm RSA -out server.key
# Generate a Certificate Signing Request (CSR) for the server:
openssl req -new -key server.key -out server.csr
# Sign the server CSR with the CA to create the server certificate:
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256
# Remove the passphrase from the server key (optional but recommended for Mosquitto):
openssl rsa -in server.key -out server.key

# Generate the client private key:
openssl genpkey -algorithm RSA -out client.key
# Generate a Certificate Signing Request (CSR) for the client:
openssl req -new -key client.key -out client.csr
# Sign the client CSR with the CA to create the client certificate:
openssl x509 -req -in client.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out client.crt -days 365 -sha256
# Remove the passphrase from the client key (optional but recommended for Mosquitto):
openssl rsa -in client.key -out client.key

# Generate a new CSR with the correct hostname:
openssl req -new -key server.key -out server.csr -subj "/CN=10.0.0.3"
# Sign the CSR with the CA:
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 365 -sha256

```