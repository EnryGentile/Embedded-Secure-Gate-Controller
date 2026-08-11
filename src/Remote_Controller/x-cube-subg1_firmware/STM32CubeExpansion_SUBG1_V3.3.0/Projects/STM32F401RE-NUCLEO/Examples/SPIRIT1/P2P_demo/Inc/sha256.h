#ifndef HMAC_SHA256_H
#define HMAC_SHA256_H

#include <stdint.h>
#include <stddef.h>

// Struttura per il contesto SHA256
typedef struct {
    uint32_t data[64];
    uint32_t datalen;
    unsigned long long bitlen;
    uint32_t state[8];
} SHA256_CTX;

// Funzioni base SHA256
void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t data[], size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t hash[]);

// La funzione "Magica" per il tuo cancello
// Prende i dati in chiaro, la tua chiave segreta e sputa fuori la firma da 32 byte
void hmac_sha256(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t *mac_out);

#endif /* HMAC_SHA256_H */
