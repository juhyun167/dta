#include <dta_test2_ta.h>

#include "func_aes.h"

/* Normal world printf address */
extern int (*ditto_printf)(const char *format, ...);

TEE_Result ta2tee_algo_id(uint32_t param, uint32_t *algo) {
    switch (param) {
    case TA_AES_ALGO_ECB:
        *algo = TEE_ALG_AES_ECB_NOPAD;
        return TEE_SUCCESS;
    case TA_AES_ALGO_CBC:
        *algo = TEE_ALG_AES_CBC_NOPAD;
        return TEE_SUCCESS;
    case TA_AES_ALGO_CTR:
        *algo = TEE_ALG_AES_CTR;
        return TEE_SUCCESS;
    default:
        EMSG("Invalid algo %u", param);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

TEE_Result ta2tee_key_size(uint32_t param, uint32_t *key_size) {
    switch (param) {
    case AES128_KEY_BYTE_SIZE:
    case AES256_KEY_BYTE_SIZE:
        *key_size = param;
        return TEE_SUCCESS;
    default:
        EMSG("Invalid key size %u", param);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

TEE_Result ta2tee_mode_id(uint32_t param, uint32_t *mode) {
    switch (param) {
    case TA_AES_MODE_ENCODE:
        *mode = TEE_MODE_ENCRYPT;
        return TEE_SUCCESS;
    case TA_AES_MODE_DECODE:
        *mode = TEE_MODE_DECRYPT;
        return TEE_SUCCESS;
    default:
        EMSG("Invalid mode %u", param);
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

/*
 * Process command TA_AES_CMD_PREPARE. API in aes_ta.h
 *
 * Allocate resources required for the ciphering operation.
 * During ciphering operation, when expect client can:
 * - update the key materials (provided by client)
 * - reset the initial vector (provided by client)
 * - cipher an input buffer into an output buffer (provided by client)
 */
TEE_Result func_prepare(void *session, uint32_t param_types,
                        TEE_Param params[4]) {
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_VALUE_INPUT,
                        TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_NONE);
    struct aes_cipher *sess;
    TEE_Attribute attr;
    TEE_Result res;
    char *key;

    /* Get ciphering context from session ID */
    DMSG("Session %p: get ciphering resources", session);
    sess = (struct aes_cipher *) session;

    /* Safely get the invocation parameters */
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    res = ta2tee_algo_id(params[0].value.a, &sess->algo);
    if (res != TEE_SUCCESS)
        return res;

    res = ta2tee_key_size(params[1].value.a, &sess->key_size);
    if (res != TEE_SUCCESS)
        return res;

    res = ta2tee_mode_id(params[2].value.a, &sess->mode);
    if (res != TEE_SUCCESS)
        return res;

    /*
     * Ready to allocate the resources which are:
     * - an operation handle, for an AES ciphering of given configuration
     * - a transient object that will be use to load the key materials
     *   into the AES ciphering operation.
     */

    /* Free potential previous operation */
    if (sess->op_handle != TEE_HANDLE_NULL)
        TEE_FreeOperation(sess->op_handle);

    /* Allocate operation: AES/CTR, mode and size from params */
    res = TEE_AllocateOperation(&sess->op_handle, sess->algo, sess->mode,
                                sess->key_size * 8);
    if (res != TEE_SUCCESS) {
        EMSG("Failed to allocate operation");
        sess->op_handle = TEE_HANDLE_NULL;
        goto err;
    }

    /* Free potential previous transient object */
    if (sess->key_handle != TEE_HANDLE_NULL)
        TEE_FreeTransientObject(sess->key_handle);

    /* Allocate transient object according to target key size */
    res = TEE_AllocateTransientObject(TEE_TYPE_AES, sess->key_size * 8,
                                      &sess->key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("Failed to allocate transient object");
        sess->key_handle = TEE_HANDLE_NULL;
        goto err;
    }

    /*
     * When loading a key in the cipher session, set_aes_key()
     * will reset the operation and load a key. But we cannot
     * reset and operation that has no key yet (GPD TEE Internal
     * Core API Specification – Public Release v1.1.1, section
     * 6.2.5 TEE_ResetOperation). In consequence, we will load a
     * dummy key in the operation so that operation can be reset
     * when updating the key.
     */
    key = TEE_Malloc(sess->key_size, 0);
    if (!key) {
        res = TEE_ERROR_OUT_OF_MEMORY;
        goto err;
    }

    TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, sess->key_size);

    res = TEE_PopulateTransientObject(sess->key_handle, &attr, 1);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_PopulateTransientObject failed, %x", res);
        goto err;
    }

    res = TEE_SetOperationKey(sess->op_handle, sess->key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SetOperationKey failed %x", res);
        goto err;
    }

    return res;

err:
    if (sess->op_handle != TEE_HANDLE_NULL)
        TEE_FreeOperation(sess->op_handle);
    sess->op_handle = TEE_HANDLE_NULL;

    if (sess->key_handle != TEE_HANDLE_NULL)
        TEE_FreeTransientObject(sess->key_handle);
    sess->key_handle = TEE_HANDLE_NULL;

    return res;
}

/*
 * Process command TA_AES_CMD_SET_KEY. API in aes_ta.h
 */
TEE_Result func_set_key(void *session, uint32_t param_types,
                        TEE_Param params[4]) {
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    struct aes_cipher *sess;
    TEE_Attribute attr;
    TEE_Result res;
    uint32_t key_sz;
    char *key;

    /* Get ciphering context from session ID */
    DMSG("Session %p: load key material", session);
    sess = (struct aes_cipher *) session;

    /* Safely get the invocation parameters */
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    key = params[0].memref.buffer;
    key_sz = params[0].memref.size;

    if (key_sz != sess->key_size) {
        EMSG("Wrong key size %" PRIu32 ", expect %" PRIu32 " bytes", key_sz,
             sess->key_size);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    /*
     * Load the key material into the configured operation
     * - create a secret key attribute with the key material
     *   TEE_InitRefAttribute()
     * - reset transient object and load attribute data
     *   TEE_ResetTransientObject()
     *   TEE_PopulateTransientObject()
     * - load the key (transient object) into the ciphering operation
     *   TEE_SetOperationKey()
     *
     * TEE_SetOperationKey() requires operation to be in "initial state".
     * We can use TEE_ResetOperation() to reset the operation but this
     * API cannot be used on operation with key(s) not yet set. Hence,
     * when allocating the operation handle, we load a dummy key.
     * Thus, set_key sequence always reset then set key on operation.
     */

    TEE_InitRefAttribute(&attr, TEE_ATTR_SECRET_VALUE, key, key_sz);

    TEE_ResetTransientObject(sess->key_handle);
    res = TEE_PopulateTransientObject(sess->key_handle, &attr, 1);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_PopulateTransientObject failed, %x", res);
        return res;
    }

    TEE_ResetOperation(sess->op_handle);
    res = TEE_SetOperationKey(sess->op_handle, sess->key_handle);
    if (res != TEE_SUCCESS) {
        EMSG("TEE_SetOperationKey failed %x", res);
        return res;
    }

    return res;
}

/*
 * Process command TA_AES_CMD_SET_IV. API in aes_ta.h
 */
TEE_Result func_set_iv(void *session, uint32_t param_types,
                       TEE_Param params[4]) {
    const uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    struct aes_cipher *sess;
    size_t iv_sz;
    char *iv;

    /* Get ciphering context from session ID */
    DMSG("Session %p: reset initial vector", session);
    sess = (struct aes_cipher *) session;

    /* Safely get the invocation parameters */
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    iv = params[0].memref.buffer;
    iv_sz = params[0].memref.size;

    /*
     * Init cipher operation with the initialization vector.
     */
    TEE_CipherInit(sess->op_handle, iv, iv_sz);

    return TEE_SUCCESS;
}

/*
 * Process command TA_AES_CMD_CIPHER. API in aes_ta.h
 */
TEE_Result func_cipher(void *session, uint32_t param_types,
                       TEE_Param params[4]) {
    const uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);
    struct aes_cipher *sess;

    /* Get ciphering context from session ID */
    DMSG("Session %p: cipher buffer", session);
    sess = (struct aes_cipher *) session;

    /* Safely get the invocation parameters */
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    if (params[1].memref.size < params[0].memref.size) {
        EMSG("Bad sizes: in %d, out %d", params[0].memref.size,
             params[1].memref.size);
        return TEE_ERROR_BAD_PARAMETERS;
    }

    if (sess->op_handle == TEE_HANDLE_NULL)
        return TEE_ERROR_BAD_STATE;

    /*
     * Process ciphering operation on provided buffers
     */
    return TEE_CipherUpdate(sess->op_handle, params[0].memref.buffer,
                            params[0].memref.size, params[1].memref.buffer,
                            &params[1].memref.size);
}