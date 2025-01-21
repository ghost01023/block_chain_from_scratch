#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void handle_errors()
{
    ERR_print_errors_fp(stderr);
    exit(1);
}

void save_public_key(EVP_PKEY *pkey, const char *filename)
{
    FILE *pubkey_file = fopen(filename, "w");
    if (!pubkey_file)
    {
        perror("Failed to open file for writing public key");
        return;
    }

    // Write the public key in PEM format
    if (PEM_write_PUBKEY(pubkey_file, pkey) != 1)
    {
        fprintf(stderr, "Failed to write public key\n");
    }
    else
    {
        printf("Public key saved to %s\n", filename);
    }

    fclose(pubkey_file);
}

void create_transaction(char *sender_address, char *receiver_address, double amount, double raw_amount)
{
    char *transaction_string = malloc(sizeof(char) * 2048);
    *(transaction_string + 0) = '\0';
    strcat(transaction_string, sender_address);
    strcat(transaction_string, "^");
    strcat(transaction_string, receiver_address);
    strcat(transaction_string, "^");
    char amount_string[32];
    sprintf(amount_string, "%f", amount);
    strcat(transaction_string, amount_string);
    strcat(transaction_string, "^");
    char raw_amount_string[32];
    sprintf(raw_amount_string, "%f", raw_amount);
    strcat(transaction_string, raw_amount_string);
    printf("\nTransaction string till now is %s", transaction_string);
}

EVP_PKEY *generate_key_pair();
unsigned char *sign_hash(EVP_PKEY *pkey, unsigned char *hash, size_t *sig_len);
int verify_signature(EVP_PKEY *pubkey, unsigned char *hash, unsigned char *signature, size_t sig_len);

int main()
{
    const char *message = "transaction message...";
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char *signature = NULL;
    size_t sig_len = 0;
    // generate key pair
    EVP_PKEY *pkey = generate_key_pair();
    EVP_PKEY *pubkey = EVP_PKEY_new();
    RSA *rsa_pubkey = EVP_PKEY_get1_RSA(pkey);
    if (!rsa_pubkey)
    {
        handle_errors();
    }
    EVP_PKEY_assign_RSA(pubkey, rsa_pubkey);

    // hash message
    SHA256((unsigned char *)message, strlen(message), hash);
    // sign the hash
    signature = sign_hash(pkey, hash, &sig_len);
    printf("\nSignature created successfully!\n");
    if (verify_signature(pubkey, hash, signature, sig_len))
    {
        printf("Signature verification successful.\n");
    }
    else
    {
        printf("Signature verification failed.\n");
    }

    free(signature);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(pubkey);
    create_transaction("\\sender_wallet", "\\receiver_wallet", 65.9, 99.4);
    return 0;
}

EVP_PKEY *generate_key_pair()
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pkey_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    if (!pkey_ctx || EVP_PKEY_keygen_init(pkey_ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(pkey_ctx, 2048) <= 0 ||
        EVP_PKEY_keygen(pkey_ctx, &pkey) <= 0)
    {
        handle_errors();
    }

    EVP_PKEY_CTX_free(pkey_ctx);
    return pkey;
}

unsigned char *sign_hash(EVP_PKEY *pkey, unsigned char *hash, size_t *sig_len)
{
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    unsigned char *signature = NULL;

    if (!md_ctx || EVP_DigestSignInit(md_ctx, NULL, EVP_sha256(), NULL, pkey) <= 0 ||
        EVP_DigestSignUpdate(md_ctx, hash, SHA256_DIGEST_LENGTH) <= 0 ||
        EVP_DigestSignFinal(md_ctx, NULL, sig_len) <= 0)
    {
        handle_errors();
    }

    signature = malloc(*sig_len);
    if (!signature)
    {
        perror("malloc");
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(pkey);
        exit(1);
    }

    if (EVP_DigestSignFinal(md_ctx, signature, sig_len) <= 0)
    {
        handle_errors();
    }

    EVP_MD_CTX_free(md_ctx);
    return signature;
}

int verify_signature(EVP_PKEY *pubkey, unsigned char *hash, unsigned char *signature, size_t sig_len)
{
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    int result = 0;

    if (!md_ctx || EVP_DigestVerifyInit(md_ctx, NULL, EVP_sha256(), NULL, pubkey) <= 0 ||
        EVP_DigestVerifyUpdate(md_ctx, hash, SHA256_DIGEST_LENGTH) <= 0 ||
        EVP_DigestVerifyFinal(md_ctx, signature, sig_len) != 1)
    {
        result = 0;
    }
    else
    {
        result = 1;
    }

    EVP_MD_CTX_free(md_ctx);
    return result;
}