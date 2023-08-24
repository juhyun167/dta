#include <dta_test_ta.h>

#include "func_extended.h"
#include "func_handlers.h"
#include "func_handlers_acipher.h"
#include "func_handlers_random.h"
#include "func_handlers_storage.h"
#include "func_handlers_time.h"

TEE_Result TA_CreateEntryPoint(void) { return TEE_SUCCESS; }

void TA_DestroyEntryPoint(void) {}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __unused params[4], void **sess) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    struct acipher *state;

    state = TEE_Malloc(sizeof(*state), 0);
    if (!state)
        return TEE_ERROR_OUT_OF_MEMORY;

    state->key = TEE_HANDLE_NULL;
    *sess = state;

    return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __unused *sess) {}

TEE_Result __TA_InvokeCommandEntryPoint(void *sess, uint32_t cmd_id,
                                        uint32_t param_types,
                                        TEE_Param params[4]) {
    switch (cmd_id) {
    case CMD_ADD:
        return func_add(param_types, params);
    case CMD_STRLEN:
        return func_strlen(param_types, params);
    case CMD_GET_SYSTEM_TIME:
        return func_get_system_time(param_types, params);
    case CMD_GENERATE_RANDOM:
        return func_generate_random(param_types, params);
    case CMD_WRITE_OBJECT:
        return func_write_object(param_types, params);
    case CMD_READ_OBJECT:
        return func_read_object(param_types, params);
    case CMD_DELETE_OBJECT:
        return func_delete_object(param_types, params);
    case CMD_GEN_KEY:
        return func_gen_key(sess, param_types, params);
    case CMD_ENC:
        return func_enc(sess, param_types, params);
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
