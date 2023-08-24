#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <test1_ta.h>

#include "func_extended.h"

TEE_Result TA_CreateEntryPoint(void) {
    return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void) {}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __maybe_unused params[4],
                                    void __maybe_unused** sess_ctx) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused* sess_ctx) {}

TEE_Result func_crashme(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    char* buf = (char*) params[0].memref.buffer;

    if (buf[0] != 'A' && buf[0] != 'a') goto out;
    if (buf[1] != 'B' && buf[1] != 'b') goto out;
    if (buf[2] != 'C' && buf[2] != 'c') goto out;
    if (buf[3] != 'D' && buf[3] != 'd') goto out;

    int *addr = (int *) 0;
    *addr = 0xdeadbeef;

out:
    return TEE_SUCCESS;
}

TEE_Result __TA_InvokeCommandEntryPoint(void __maybe_unused* sess_ctx,
                                        uint32_t cmd_id,
                                        uint32_t param_types,
                                        TEE_Param params[4]) {
    switch (cmd_id) {
        case CMD_CRASHME:
            return func_crashme(param_types, params);
        default:
            return TEE_ERROR_BAD_PARAMETERS;
    }
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused* sess,
                                      uint32_t cmd_id,
                                      uint32_t param_types,
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
            res =
                __TA_InvokeCommandEntryPoint(sess, cmd_id, param_types, params);
            asm volatile("add sp,sp,0x100");

            return res;
    }
}
