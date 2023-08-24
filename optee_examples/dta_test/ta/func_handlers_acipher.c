#include <dta_test_ta.h>

#include "func_handlers_acipher.h"

TEE_Result func_gen_key(struct acipher *state, uint32_t param_types,
                        TEE_Param params[4]) {
    TEE_Result res;
    uint32_t key_size;
    TEE_ObjectHandle key;
    const uint32_t key_type = TEE_TYPE_RSA_KEYPAIR;
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    key_size = params[0].value.a;

    res = TEE_AllocateTransientObject(key_type, key_size, &key);
    if (res) {
        log_error("TEE_AllocateTransientObject() failed. (res=0x%x)\n", res);
        return res;
    }

    res = TEE_GenerateKey(key, key_size, NULL, 0);
    if (res) {
        log_error("TEE_GenerateKey() failed. (res=0x%x)\n", res);
        TEE_FreeTransientObject(key);
        return res;
    }

    TEE_FreeTransientObject(state->key);
    state->key = key;

    return TEE_SUCCESS;
}

TEE_Result func_enc(struct acipher *state, uint32_t param_types,
                    TEE_Param params[4]) {
    TEE_Result res;
    const void *inbuf;
    uint32_t inbuf_len;
    void *outbuf;
    uint32_t outbuf_len;
    TEE_OperationHandle op;
    TEE_ObjectInfo key_info;
    const uint32_t alg = TEE_ALG_RSAES_PKCS1_V1_5;
    const uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;
    if (!state->key)
        return TEE_ERROR_BAD_STATE;

    res = TEE_GetObjectInfo1(state->key, &key_info);
    if (res) {
       log_error("TEE_GetObjectInfo1() failed. (res=0x%x)\n", res);
        return res;
    }

    inbuf = params[0].memref.buffer;
    inbuf_len = params[0].memref.size;
    outbuf = params[1].memref.buffer;
    outbuf_len = params[1].memref.size;

    res = TEE_AllocateOperation(&op, alg, TEE_MODE_ENCRYPT, key_info.keySize);
    if (res) {
        log_error("TEE_AllocateOperation() failed. (res=0x%x)\n", res);
        return res;
    }

    res = TEE_SetOperationKey(op, state->key);
    if (res) {
        log_error("TEE_SetOperationKey() failed. (res=0x%x)\n", res);
        goto out;
    }

    res = TEE_AsymmetricEncrypt(op, NULL, 0, inbuf, inbuf_len, outbuf,
                                &outbuf_len);
    if (res)
        log_error("TEE_AsymmetricEncrypt() failed. (res=0x%x)\n", res);

    params[1].memref.size = outbuf_len;

out:
    TEE_FreeOperation(op);
    return res;
}