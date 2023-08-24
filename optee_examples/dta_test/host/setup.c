#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <ditto_ta.h>

#include "memory.h"
#include "setup.h"

/* Host session id from Secure world */
uint32_t ta_session;

/* TA addresses from Secure world */
const size_t ta_code_addr = 0x40015000;
size_t ta_data_addr;
size_t ta_stack_addr;
size_t ta_entry_func_addr;
uint32_t *utee_return_func_addr;

/* Memory mapped ditto pages in Normal world */
void *ditto_code_addr;
void *ditto_data_addr;
void *ditto_shared_addr;

/* Ditto page sizes in Normal world */
size_t ditto_code_size;
size_t ditto_data_size;
const size_t ditto_shared_size = PAGE_SIZE;

/* For `utee_*()` code patch at runtime */
extern const size_t ditto_syscall_func_addr;

/**
 * Retrieve TA addresses from Secure world.
 *
 * @param sess Session handle.
 */
void fetch_ta_addrs(TEEC_Session *sess) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_VALUE_OUTPUT,
                                     TEEC_VALUE_OUTPUT, TEEC_VALUE_OUTPUT);
    res = TEEC_InvokeCommand(sess, CMD_FETCH_TA_ADDRS, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_FETCH_TA_ADDRS failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    ta_data_addr = p64(op.params[0].value.a, op.params[0].value.b);
    ta_stack_addr = p64(op.params[1].value.a, op.params[1].value.b);

    ta_entry_func_addr = p64(op.params[2].value.a, op.params[2].value.b);
    ta_session = op.params[3].value.a;

    ditto_shared_addr = (void *) ta_data_addr + op.params[3].value.b;
}

/**
 * Send libc `printf()` address to TA.
 *
 * TA `printf()` in libutee is incompatible with ditto due to custom syscalls.
 * We resolve libc `printf()` address and send to TA for later use in ditto.
 *
 * @param sess Session handle.
 */
void set_ditto_printf(TEEC_Session *sess) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    void *handle = dlopen("/lib64/libc.so.6", RTLD_LOCAL | RTLD_LAZY);
    int (*ditto_printf)(const char *format, ...) = dlsym(handle, "printf");

    memset(&op, 0, sizeof(op));
    op.paramTypes =
        TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = (size_t) ditto_printf & 0xffffffff;
    op.params[0].value.b = (size_t) ditto_printf >> 32;
    res = TEEC_InvokeCommand(sess, CMD_SET_DITTO_PRINTF, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_SET_DITTO_PRINTF failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }
}

/**
 * Map ditto pages in Normal world user-land.
 *
 * @param sess Session handle.
 */
void alloc_ditto_pages(TEEC_Session *sess) {
    ditto_code_size = ta_data_addr - ta_code_addr;
    ditto_data_size = round_up(ta_stack_addr - ta_data_addr, PAGE_SIZE);

    ditto_code_addr =
        mmap((void *) ta_code_addr, ditto_code_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    ditto_data_addr =
        mmap((void *) ta_data_addr, ditto_data_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

/**
 * Patch `_utee_return()` function in ditto code page.
 *
 * We resolve `_utee_return()` address by runtime disass-ing `__ta_entry()`
 * code. Then we patch `svc #0` to make it return like proper function.
 */
void patch_utee_return() {
    uint32_t *p = (uint32_t *) ta_entry_func_addr;

    utee_return_func_addr =
        (uint32_t *) ((size_t) &p[4] + (p[4] & 0x03ffffff) * 4);
    utee_return_func_addr[0] = 0xa9c67bfd; // ldp x29,x30,[sp,#0x60]!
    utee_return_func_addr[1] = 0x910003bf; // mov sp,x29
}

/**
 * Patch `_utee_*()` functions in ditto code page.
 *
 * `_utee_*()` syscall wrappers are 3 lines of instructions each
 * and the line only we are able to patch is that single `svc #0`.
 * (e.g., _utee_get_time: mov x8,#13 ; svc #0; ret")
 *
 * As a result of this restriction, we placed a trampoline near code page
 * and made all `_utee_*()` wrappers branch with relative offsets instead of
 * `svc`. The trampoline moves control to ditto_syscall, an actual launch pad
 * for syscall proxy which is implemented as a part of Host code in `ditto.c`.
 */
void patch_utee_syscalls() {
    const size_t near_addr = 0x40000000;
    uint32_t *ditto_syscall_entry_addr;

    ditto_syscall_entry_addr =
        mmap((void *) near_addr, PAGE_SIZE, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    uint16_t bit0_16 = ditto_syscall_func_addr & 0xffff;
    uint16_t bit16_32 = (ditto_syscall_func_addr >> 16) & 0xffff;
    uint16_t bit32_48 = (ditto_syscall_func_addr >> 32) & 0xffff;

    ditto_syscall_entry_addr[0] = 0xa9bf7bfd; // stp x29,x30,[sp,#-0x10]!
    ditto_syscall_entry_addr[1] = 0xa9bf73fb; // stp x27,x28,[sp,#-0x10]!
    ditto_syscall_entry_addr[2] = (0b110100101) << 23 | (bit0_16) << 5 |
                                  27; // movz x27,ditto_syscall_func_addr[0:16]
    ditto_syscall_entry_addr[3] = (0b111100101) << 23 | (0b01) << 21 |
                                  (bit16_32) << 5 |
                                  27; // movk x27,ditto_syscall_func_addr[16:32]
    ditto_syscall_entry_addr[4] = (0b111100101) << 23 | (0b10) << 21 |
                                  (bit32_48) << 5 |
                                  27; // movk x27,ditto_syscall_func_addr[32:48]
    ditto_syscall_entry_addr[5] = 0xd63f0360; // blr x27
    ditto_syscall_entry_addr[6] = 0xa8c173fb; // ldp x27,x28,[sp],#0x10
    ditto_syscall_entry_addr[7] = 0xa8c17bfd; // ldp x29,x30,[sp],#0x10
    ditto_syscall_entry_addr[8] = 0xd65f03c0; // ret

    mprotect(ditto_syscall_entry_addr, PAGE_SIZE, PROT_READ | PROT_EXEC);

    char *p = (char *) &utee_return_func_addr[3]; // _utee_log() address
    const char pattern[] = "\x80\xd2\x01\x00\x00\xd4\xc0\x03\x5f\xd6";

    while (true) {
        if (memcmp(p + 2, pattern, sizeof(pattern) - 1))
            break;

        size_t patch_loc = (size_t) p + 4;
        size_t offset = (size_t) ditto_syscall_entry_addr - patch_loc;
        *(uint32_t *) patch_loc =
            (0b000101 << 26) |
            ((offset >> 2) & ((1 << 26) - 1)); // b ditto_syscall_entry_addr

        p += 12;
    }
}

/**
 * Creates ditto instance.
 *
 * This function and subroutines are responsible for ditto creation.
 * Should be called ahead of all ditto command invokes.
 *
 * @param sess Session handle.
 */
void setup(TEEC_Session *sess) {
    fetch_ta_addrs(sess);
    set_ditto_printf(sess);
    alloc_ditto_pages(sess);

    copy_from_ta(sess, ta_code_addr, ditto_code_addr, ditto_code_size);
    copy_from_ta(sess, ta_data_addr, ditto_data_addr, ditto_data_size);

    patch_utee_return();
    patch_utee_syscalls();

    mprotect(ditto_code_addr, ditto_code_size, PROT_READ | PROT_EXEC);
}