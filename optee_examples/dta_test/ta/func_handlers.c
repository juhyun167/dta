#include <dta_test_ta.h>

#include "func_handlers.h"

/* Normal world printf address */
extern int (*ditto_printf)(const char *format, ...);

TEE_Result func_add(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT, TEE_PARAM_TYPE_VALUE_INPUT,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    params[0].value.a += params[1].value.a;

    return TEE_SUCCESS;
}

TEE_Result func_strlen(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT, TEE_PARAM_TYPE_VALUE_OUTPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    char *s;
    for (s = params[0].memref.buffer; *s; s++)
        ;
    params[1].value.a = s - (char *) params[0].memref.buffer;

    return TEE_SUCCESS;
}