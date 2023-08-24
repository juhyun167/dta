#include <dta_test_ta.h>

#include "func_handlers_random.h"

TEE_Result func_generate_random(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_OUTPUT, TEE_PARAM_TYPE_VALUE_INPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    uint32_t len = params[1].value.a;
    void *buf = TEE_Malloc(len, 0);

    if (!buf)
        return TEE_ERROR_OUT_OF_MEMORY;

    TEE_GenerateRandom(buf, len);

    TEE_MemMove(params[0].memref.buffer, buf, len);
    params[0].memref.size = len;

    TEE_Free(buf);

    return TEE_SUCCESS;
}