#include <dta_test2_ta.h>

#include "func_aes.h"
#include "func_extended.h"

TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }

void TA_DestroyEntryPoint(void) {}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __unused params[4], void **sess) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    struct aes_cipher *state;

    state = TEE_Malloc(sizeof(*state), 0);
    if (!state)
        return TEE_ERROR_OUT_OF_MEMORY;

    state->key_handle = TEE_HANDLE_NULL;
    state->op_handle = TEE_HANDLE_NULL;
    *sess = state;

    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess) {
    struct aes_cipher *state = (struct aes_cipher *) sess;

    if (state->key_handle != TEE_HANDLE_NULL)
        TEE_FreeTransientObject(state->key_handle);
    if (state->op_handle != TEE_HANDLE_NULL)
        TEE_FreeOperation(state->op_handle);

    TEE_Free(sess);
}

TEE_Result __TA_InvokeCommandEntryPoint(void *sess, uint32_t cmd_id,
                                        uint32_t param_types,
                                        TEE_Param params[4]) {
    switch (cmd_id) {
    case CMD_PREPARE:
        return func_prepare(sess, param_types, params);
    case CMD_SET_KEY:
        return func_set_key(sess, param_types, params);
    case CMD_SET_IV:
        return func_set_iv(sess, param_types, params);
    case CMD_CIPHER:
        return func_cipher(sess, param_types, params);
    default:
        return TEE_ERROR_BAD_PARAMETERS;
    }
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess,
                                      uint32_t cmd_id, uint32_t param_types,
                                      TEE_Param params[4]) {
    TEE_Result res;

    switch (cmd_id) {
    case CMD_FETCH_TA_ADDRS:
        return func_fetch_ta_addrs(param_types, params);
    case CMD_SET_DITTO_PRINTF:
        return func_set_ditto_printf(param_types, params);
    case CMD_COPY_FROM_TA:
        return func_copy_from_ta(param_types, params);
    case CMD_COPY_TO_TA:
        return func_copy_to_ta(param_types, params);
    case CMD_PROXY_SYSCALL:
        return func_proxy_syscall(param_types, params);

    default:
        asm volatile("sub sp,sp,0x100");
        res = __TA_InvokeCommandEntryPoint(sess, cmd_id, param_types, params);
        asm volatile("add sp,sp,0x100");

        return res;
    }
}