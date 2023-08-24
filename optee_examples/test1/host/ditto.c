#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

#include <ditto_ta.h>

#include "ditto.h"
#include "memory.h"
#include "syscall.h"

/* For utee_*() syscall wrappers patch */
TEEC_Result ditto_syscall(uint64_t arg0,
                          uint64_t arg1,
                          uint64_t arg2,
                          uint64_t arg3,
                          uint64_t arg4,
                          uint64_t arg5,
                          uint64_t arg6,
                          uint64_t arg7);
const size_t ditto_syscall_func_addr = (size_t) &ditto_syscall;

/* Host session id from Secure world */
extern uint32_t ta_session;

/* TA addresses from Secure world */
extern size_t ta_stack_addr;
extern size_t ta_entry_func_addr;
extern uint32_t* utee_return_func_addr;

extern void* ditto_shared_addr;
extern const size_t ditto_shared_size;

/* TEE syscall paramters table */
extern tee_syscall_param_info tee_syscall_param_table[TEE_SCN_MAX]
                                                     [TEE_SVC_MAX_ARGS];

/* Established context structure with TA */
extern struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
} ctx;

/**
 * Context switch to and from ditto.
 *
 * This function saves context and moves control to `__ta_entry()`.
 * Ditto will then execute command handlers on behalf of TA.
 *
 * `ditto_return` label acts as a return address for ditto.
 * Patched `_utee_return` will return to that addess afterwards.
 *
 * @param ta_session Host session id.
 * @param ta_stack_addr TA stack address, ditto uses same one.
 * @param cmd_id TA command id.
 * @param ta_entry_func_addr `__ta_entry()` address.
 */
TEEC_Result ditto_entry(uint32_t ta_session,
                        size_t ta_stack_addr,
                        uint32_t cmd_id,
                        size_t ta_entry_func_addr) {
    asm volatile(
        "stp x0,x1,[sp,#-0x10]!			\n\t"
        "stp x2,x3,[sp,#-0x10]!			\n\t"
        "stp x4,x5,[sp,#-0x10]!			\n\t"
        "stp x6,x7,[sp,#-0x10]!			\n\t"
        "stp x8,x9,[sp,#-0x10]!			\n\t"
        "stp x10,x11,[sp,#-0x10]!		\n\t"
        "stp x12,x13,[sp,#-0x10]!		\n\t"
        "stp x14,x15,[sp,#-0x10]!		\n\t"
        "stp x16,x17,[sp,#-0x10]!		\n\t"
        "stp x18,x19,[sp,#-0x10]!		\n\t"
        "stp x20,x21,[sp,#-0x10]!		\n\t"
        "stp x22,x23,[sp,#-0x10]!		\n\t"
        "stp x24,x25,[sp,#-0x10]!		\n\t"
        "stp x26,x27,[sp,#-0x10]!		\n\t"
        "stp x28,x29,[sp,#-0x10]!		\n\t"
        "str x30,[sp,#-0x10]!			\n\t"

        "mov x21,x3						\n\t"
        "mov x3,x2						\n\t"
        "mov x2,x1						\n\t"
        "mov x1,x0						\n\t"
        "mov x0,#2						\n\t"

        "mov x4,sp						\n\t"
        "adr x5,ditto_return			\n\t"
        "add x6,x2,#0x50				\n\t"
        "stp x4,x5,[x6]					\n\t"

        "eor x4,x4,x4					\n\t"
        "eor x5,x5,x5					\n\t"
        "eor x6,x6,x6					\n\t"
        "eor x7,x7,x7					\n\t"
        "eor x8,x8,x8					\n\t"
        "eor x9,x9,x9					\n\t"
        "eor x10,x10,x10				\n\t"
        "eor x11,x11,x11				\n\t"
        "eor x12,x12,x12				\n\t"
        "mov x13,x2						\n\t"
        "eor x14,x14,x14				\n\t"
        "eor x15,x15,x15				\n\t"
        "eor x16,x16,x16				\n\t"
        "eor x17,x17,x17				\n\t"
        "eor x18,x18,x18				\n\t"
        "eor x19,x19,x19				\n\t"
        "eor x20,x20,x20				\n\t"
        "eor x22,x22,x22				\n\t"
        "eor x23,x23,x23				\n\t"
        "eor x24,x24,x24				\n\t"
        "eor x25,x25,x25				\n\t"
        "eor x26,x26,x26				\n\t"
        "eor x27,x27,x27				\n\t"
        "eor x28,x28,x28				\n\t"
        "eor x29,x29,x29				\n\t"
        "eor x30,x30,x30				\n\t"

        "mov sp,x2						\n\t"
        "br x21							\n\t"

        "ditto_return:					\n\t"
        "ldr x30,[sp],#0x10				\n\t"
        "ldp x28,x29,[sp],#0x10			\n\t"
        "ldp x26,x27,[sp],#0x10			\n\t"
        "ldp x24,x25,[sp],#0x10			\n\t"
        "ldp x22,x23,[sp],#0x10			\n\t"
        "ldp x20,x21,[sp],#0x10			\n\t"
        "ldp x18,x19,[sp],#0x10			\n\t"
        "ldp x16,x17,[sp],#0x10			\n\t"
        "ldp x14,x15,[sp],#0x10			\n\t"
        "ldp x12,x13,[sp],#0x10			\n\t"
        "ldp x10,x11,[sp],#0x10			\n\t"
        "ldp x8,x9,[sp],#0x10			\n\t"
        "ldp x6,x7,[sp],#0x10			\n\t"
        "ldp x4,x5,[sp],#0x10			\n\t"
        "ldp x2,x3,[sp],#0x18			\n\t"
        "ldr x1,[sp],#8			        \n\t");
}

void* ditto_shared_alloc(void** shared_end, size_t size) {
    size_t ditto_shared_end = (size_t) ditto_shared_addr + ditto_shared_size;
    void* addr;

    if ((size_t) *shared_end + (size << 1) > ditto_shared_end) {
        log_error("Cannot allocate shared memory of size %lu\n", size);
        exit(0);
    }

    addr = *shared_end;
    *shared_end += (size << 1);

    return addr;
}

/**
 * Execute TA command in ditto.
 *
 * This function is exact counterpart of `TEEC_InvokeCommand()` in libteec.
 * Places parameters in ditto stack beforehand and triggers context switch.
 *
 * @param session Session handle.
 * @param cmd_id TA command id.
 * @param operation TEEC_Operation structure.
 * @param error_origin Error originator.
 */
TEEC_Result ditto_invoke_command(TEEC_Session* session,
                                 uint32_t cmd_id,
                                 TEEC_Operation* operation,
                                 uint32_t* error_origin) {
    struct utee_params {
        uint64_t types;
        uint64_t vals[TEE_NUM_PARAMS * 2];
    } up;
    void *shared_end = ditto_shared_addr, *buf;
    size_t size;
    TEEC_Result res;

    memset(&up, 0, sizeof(up));
    for (int n = TEE_NUM_PARAMS - 1; n >= 0; n--) {
        up.types <<= 4;
        uint32_t param_type = TEEC_PARAM_TYPE_GET(operation->paramTypes, n);

        switch (param_type) {
            case TEEC_NONE:
            case TEEC_VALUE_INPUT:
            case TEEC_VALUE_OUTPUT:
            case TEEC_VALUE_INOUT:
                up.types |= param_type;
                up.vals[2 * n] = operation->params[n].value.a;
                up.vals[2 * n + 1] = operation->params[n].value.b;
                break;
            case TEEC_MEMREF_TEMP_INPUT:
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INOUT:
                up.types |= param_type;
                if (!operation->params[n].tmpref.buffer)
                    break;

                size = operation->params[n].tmpref.size;
                buf = ditto_shared_alloc(&shared_end, size);
                memmove(buf, operation->params[n].tmpref.buffer, size);

                up.vals[2 * n] = (uint64_t) buf;
                up.vals[2 * n + 1] = size;
                break;
            default:
                break;
        }
    }
    memmove((void*) ta_stack_addr, &up, sizeof(up));

    res = ditto_entry(ta_session, ta_stack_addr, cmd_id, ta_entry_func_addr);
    *error_origin = TEEC_ORIGIN_TRUSTED_APP;

    struct utee_params* ta_stack = (struct utee_params*) ta_stack_addr;

    for (int n = TEE_NUM_PARAMS - 1; n >= 0; n--) {
        uint32_t param_type = TEEC_PARAM_TYPE_GET(operation->paramTypes, n);

        switch (param_type) {
            case TEEC_VALUE_OUTPUT:
            case TEEC_VALUE_INOUT:
                operation->params[n].value.a = ta_stack->vals[2 * n];
                operation->params[n].value.b = ta_stack->vals[2 * n + 1];
                break;
            case TEEC_MEMREF_TEMP_OUTPUT:
            case TEEC_MEMREF_TEMP_INOUT:
                buf = (void*) ta_stack->vals[2 * n];
                size = ta_stack->vals[2 * n + 1];
                if (buf)
                    memmove(operation->params[n].tmpref.buffer, buf, size);
                operation->params[n].tmpref.size = ta_stack->vals[2 * n + 1];
                break;
            default:
                break;
        }
    }

    return res;
}

void ditto_copy_attributes(struct utee_attribute* ua, uint32_t attrs_count) {
    for (int i = 0; i < attrs_count; i++) {
        if (ua[i].attribute_id & TEE_ATTR_FLAG_VALUE)
            continue;

        void* addr = (void*) ua[i].a;
        size_t length = ua[i].b;

        copy_to_ta(&ctx.sess, (size_t) addr, addr, length);
    }
}

/**
 * Proxy ditto syscalls to TA.
 *
 * Final destination of all patched `_utee_*()` syscall wrappers in ditto.
 * Requests proxy syscall execution to TA using `CMD_PROXY_SYSCALL`.
 *
 * Before and after `CMD_PROXY_SYSCALL`, we needs synchronization for input
 * and output memory references with TA memory. Otherwise TA will not able to
 * fetch input data nor we will not able to fetch TA written output data.
 *
 * For this purpose, we perform `CMD_COPY_TO_TA` for input references and
 * `CMD_COPY_FROM_TA` for output references.
 */
__attribute__((__optimize__("-fno-stack-protector"))) TEEC_Result ditto_syscall(
    uint64_t arg0,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4,
    uint64_t arg5,
    uint64_t arg6,
    uint64_t arg7) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    uint64_t args[TEE_SVC_MAX_ARGS];
    uint64_t scn;

    args[0] = arg0, args[1] = arg1, args[2] = arg2, args[3] = arg3;
    args[4] = arg4, args[5] = arg5, args[6] = arg6, args[7] = arg7;
    asm volatile("mov %[output],x8" : [output] "=r"(scn));

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_NONE, TEEC_NONE);

    for (int i = 0; i < TEE_SVC_MAX_ARGS; i++) {
        volatile tee_syscall_param_info* info =
            &tee_syscall_param_table[scn][i];

        if (info->type == SVC_PARAM_NONE)
            break;
        if (info->type != SVC_PARAM_MEMREF_INPUT &&
            info->type != SVC_PARAM_MEMREF_INOUT)
            continue;

        void* addr = (void*) args[i];
        uint32_t size;

        if (info->size_idx) {
            size = args[info->size_idx];
            if (info->size) {
                size *= info->size;
                ditto_copy_attributes((struct utee_attribute*) addr,
                                      args[info->size_idx]);
            }
        } else if (info->size_ptr_idx)
            size = *(size_t*) args[info->size_ptr_idx];
        else
            size = info->size;

        if (!addr)
            continue;

        copy_to_ta(&ctx.sess, (size_t) addr, addr, size);
    }

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = scn;
    op.params[1].tmpref.buffer = &args;
    op.params[1].tmpref.size = sizeof(args);
    res = TEEC_InvokeCommand(&ctx.sess, CMD_PROXY_SYSCALL, &op, &origin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);

    for (int i = 0; i < TEE_SVC_MAX_ARGS; i++) {
        volatile tee_syscall_param_info* info =
            &tee_syscall_param_table[scn][i];

        if (info->type == SVC_PARAM_NONE)
            break;
        if (info->type != SVC_PARAM_MEMREF_OUTPUT &&
            info->type != SVC_PARAM_MEMREF_INOUT)
            continue;

        void* addr = (void*) args[i];
        uint32_t size;

        if (info->size_idx)
            size = (info->size) ? info->size * args[info->size_idx]
                                : args[info->size_idx];
        else if (info->size_ptr_idx)
            size = *(size_t*) args[info->size_ptr_idx];
        else
            size = info->size;

        if (!addr)
            continue;

        copy_from_ta(&ctx.sess, (size_t) addr, addr, size);
    }

    return res;
}