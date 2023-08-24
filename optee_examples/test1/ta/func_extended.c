#include <ditto_ta.h>

#include "func_extended.h"

/* Normal world printf address */
extern int (*ditto_printf)(const char *format, ...);

extern char ditto_shared[0x1000];

TEE_Result func_fetch_ta_addrs(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_VALUE_OUTPUT,
        TEE_PARAM_TYPE_VALUE_OUTPUT, TEE_PARAM_TYPE_VALUE_OUTPUT);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    size_t ta_data_addr;
    size_t ta_stack_addr;
    size_t ta_entry_func_addr;
    uint32_t ta_session;

    asm volatile(
        "mov %[output_data],x21             \n\t"
        "mov %[output_stack],x13            \n\t"
        "stp x27,x28,[sp,#-0x10]!           \n\t"
        "ldr x28,[x29]                      \n\t"
        "ldr x28,[x28,#8]                   \n\t"
        "sub x28,x28,#0xc                   \n\t"
        "mov %[output_entry],x28            \n\t"
        "ldr x28,[x29,#0x10]                \n\t"
        "ldr x28,[x28]                      \n\t"
        "mov %[output_sess],x28             \n\t"
        "ldp x27,x28,[sp],#0x10             \n\t"
        : [output_data] "=r"(ta_data_addr), [output_stack] "=r"(ta_stack_addr),
          [output_entry] "=r"(ta_entry_func_addr),
          [output_sess] "=r"(ta_session));

    params[0].value.a = low32(ta_data_addr);
    params[0].value.b = high32(ta_data_addr);
    params[1].value.a = low32(ta_stack_addr);
    params[1].value.b = high32(ta_stack_addr);
    params[2].value.a = low32(ta_entry_func_addr);
    params[2].value.b = high32(ta_entry_func_addr);
    params[3].value.a = ta_session;
    params[3].value.b = (size_t) &ditto_shared - ta_data_addr;

    return TEE_SUCCESS;
}

TEE_Result func_set_ditto_printf(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_NONE,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    ditto_printf = (void *) p64(params[0].value.a, params[0].value.b);

    return TEE_SUCCESS;
}

TEE_Result func_copy_from_ta(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    void *from = (void *) p64(params[0].value.a, params[0].value.b);
    void *to = params[1].memref.buffer;
    size_t n = params[1].memref.size;

    TEE_MemMove(to, from, n);

    return TEE_SUCCESS;
}

TEE_Result func_copy_to_ta(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    void *to = (void *) p64(params[0].value.a, params[0].value.b);
    void *from = params[1].memref.buffer;
    size_t n = params[1].memref.size;

    TEE_MemMove(to, from, n);

    return TEE_SUCCESS;
}

asm(".text                              \n\t"
    ".global syscall                    \n\t"
    ".type syscall @function            \n\t"
    "syscall:                           \n\t"
    "uxtw x8,w0                         \n\t"
    "mov x27,x1                         \n\t"
    "ldp x0,x1,[x27]                    \n\t"
    "ldp x2,x3,[x27,#0x10]              \n\t"
    "ldp x4,x5,[x27,#0x20]              \n\t"
    "ldp x6,x7,[x27,#0x30]              \n\t"
    "svc #0                             \n\t"
    "ret                                \n\t");

TEE_Result syscall(int number, void *args);

TEE_Result func_proxy_syscall(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types =
        TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT, TEE_PARAM_TYPE_MEMREF_INPUT,
                        TEE_PARAM_TYPE_NONE, TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;
    
    int number = params[0].value.a;
    void *args = params[1].memref.buffer;

    return syscall(number, args);
}