#include <dta_test_ta.h>

#include "func_handlers_time.h"

TEE_Result func_get_system_time(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    TEE_Time time;
    TEE_GetSystemTime(&time);

    params[0].value.a = time.seconds;
    params[0].value.b = time.millis;

    return TEE_SUCCESS;
}