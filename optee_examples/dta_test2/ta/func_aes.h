#ifndef FUNC_AES_H
#define FUNC_AES_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#define AES128_KEY_BIT_SIZE 128
#define AES128_KEY_BYTE_SIZE (AES128_KEY_BIT_SIZE / 8)
#define AES256_KEY_BIT_SIZE 256
#define AES256_KEY_BYTE_SIZE (AES256_KEY_BIT_SIZE / 8)

struct aes_cipher {
    uint32_t algo;                 /* AES flavour */
    uint32_t mode;                 /* Encode or decode */
    uint32_t key_size;             /* AES key size in byte */
    TEE_OperationHandle op_handle; /* AES ciphering operation */
    TEE_ObjectHandle key_handle;   /* transient object to load the key */
};

TEE_Result func_prepare(void *session, uint32_t param_types,
                        TEE_Param params[4]);
TEE_Result func_set_key(void *session, uint32_t param_types,
                        TEE_Param params[4]);
TEE_Result func_set_iv(void *session, uint32_t param_types,
                       TEE_Param params[4]);
TEE_Result func_cipher(void *session, uint32_t param_types,
                       TEE_Param params[4]);

#endif /* FUNC_AES_H */