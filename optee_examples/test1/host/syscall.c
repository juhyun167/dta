#include <stdlib.h>
#include <stdint.h>

#include "syscall.h"

tee_syscall_param_info tee_syscall_param_table[TEE_SCN_MAX][TEE_SVC_MAX_ARGS] = {
    // 8
    [TEE_SCN_CHECK_ACCESS_RIGHTS] = {
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
    },
    // 13
    [TEE_SCN_GET_TIME] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(size_t) },
    },
    // 15
    [TEE_SCN_CRYP_STATE_ALLOC] = {
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(uint32_t) },
    },
    // 17
    [TEE_SCN_CRYP_STATE_FREE] = {
        { SVC_PARAM_VALUE },
    },
    // 18
    [TEE_SCN_HASH_INIT] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
    },
    // 21
    [TEE_SCN_CIPHER_INIT] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
    },
    // 22
    [TEE_SCN_CIPHER_UPDATE] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size_ptr_idx = 4 },
        { .type = SVC_PARAM_MEMREF_INOUT, .size = sizeof(uint64_t) },
    },
    // 23
    [TEE_SCN_CIPHER_FINAL] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size_ptr_idx = 4 },
        { .type = SVC_PARAM_MEMREF_INOUT, .size = sizeof(uint64_t) },
    },
    // 24
    [TEE_SCN_CRYP_OBJ_GET_INFO] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(utee_object_info) },
    },
    // 27
    [TEE_SCN_CRYP_OBJ_ALLOC] = {
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(uint32_t) },
    },
    // 28
    [TEE_SCN_CRYP_OBJ_CLOSE] = {
        { SVC_PARAM_VALUE },
    },
    // 29
    [TEE_SCN_CRYP_OBJ_RESET] = {
        { SVC_PARAM_VALUE }
    },
    // 30
    [TEE_SCN_CRYP_OBJ_POPULATE] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size = sizeof(utee_attribute), .size_idx = 2 },
        { SVC_PARAM_VALUE },
    },
    // 31
    [TEE_SCN_CRYP_OBJ_COPY] = {
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
    },
    // 33
    [TEE_SCN_CRYP_RANDOM_NUMBER_GENERATE] = {
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size_idx = 1 },
        { SVC_PARAM_VALUE },
    },
    // 39
    [TEE_SCN_ASYMM_OPERATE] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size = sizeof(utee_attribute), .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 4 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size_ptr_idx = 6 },
        { .type = SVC_PARAM_MEMREF_INOUT, .size = sizeof(size_t) },
    },
    // 41
    [TEE_SCN_STORAGE_OBJ_OPEN] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(uint32_t) },
    },
    // 42
    [TEE_SCN_STORAGE_OBJ_CREATE] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 6 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(uint32_t) },
    },
    // 43
    [TEE_SCN_STORAGE_OBJ_DEL] = {
        { SVC_PARAM_VALUE },
    },
    // 50
    [TEE_SCN_STORAGE_OBJ_READ] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_OUTPUT, .size = sizeof(size_t) },
    },
    // 51
    [TEE_SCN_STORAGE_OBJ_WRITE] = {
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size_idx = 2 },
        { SVC_PARAM_VALUE },
    },
    // 54
    [TEE_SCN_CRYP_OBJ_GENERATE_KEY] = {
        { SVC_PARAM_VALUE },
        { SVC_PARAM_VALUE },
        { .type = SVC_PARAM_MEMREF_INPUT, .size = sizeof(utee_attribute), .size_idx = 3 },
        { SVC_PARAM_VALUE },
    },
};