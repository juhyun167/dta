#ifndef SYSCALL_H
#define SYSCALL_H

#define SVC_PARAM_NONE 0
#define SVC_PARAM_MEMREF_INPUT 1
#define SVC_PARAM_MEMREF_OUTPUT 2
#define SVC_PARAM_MEMREF_INOUT 3
#define SVC_PARAM_VALUE 4

typedef struct tee_syscall_param_info {
    uint32_t type;
    uint32_t size;
    uint8_t size_idx;
    uint8_t size_ptr_idx;
} tee_syscall_param_info;

/* From utee_types.h in optee_os */

typedef struct utee_attribute {
	uint64_t a;	/* also serves as a pointer for references */
	uint64_t b;	/* also serves as a length for references */
	uint32_t attribute_id;
} utee_attribute;

typedef struct utee_object_info {
    uint32_t obj_type;
    uint32_t obj_size;
    uint32_t max_obj_size;
    uint32_t obj_usage;
    uint32_t data_size;
    uint32_t data_pos;
    uint32_t handle_flags;
} utee_object_info;

/* End of utee_types.h */

/* From tee_syscall_numbers.h in optee_os */

#define TEE_SCN_RETURN 0
#define TEE_SCN_LOG 1
#define TEE_SCN_PANIC 2
#define TEE_SCN_GET_PROPERTY 3
#define TEE_SCN_GET_PROPERTY_NAME_TO_INDEX 4
#define TEE_SCN_OPEN_TA_SESSION 5
#define TEE_SCN_CLOSE_TA_SESSION 6
#define TEE_SCN_INVOKE_TA_COMMAND 7
#define TEE_SCN_CHECK_ACCESS_RIGHTS 8
#define TEE_SCN_GET_CANCELLATION_FLAG 9
#define TEE_SCN_UNMASK_CANCELLATION 10
#define TEE_SCN_MASK_CANCELLATION 11
#define TEE_SCN_WAIT 12
#define TEE_SCN_GET_TIME 13
#define TEE_SCN_SET_TA_TIME 14
#define TEE_SCN_CRYP_STATE_ALLOC 15
#define TEE_SCN_CRYP_STATE_COPY 16
#define TEE_SCN_CRYP_STATE_FREE 17
#define TEE_SCN_HASH_INIT 18
#define TEE_SCN_HASH_UPDATE 19
#define TEE_SCN_HASH_FINAL 20
#define TEE_SCN_CIPHER_INIT 21
#define TEE_SCN_CIPHER_UPDATE 22
#define TEE_SCN_CIPHER_FINAL 23
#define TEE_SCN_CRYP_OBJ_GET_INFO 24
#define TEE_SCN_CRYP_OBJ_RESTRICT_USAGE 25
#define TEE_SCN_CRYP_OBJ_GET_ATTR 26
#define TEE_SCN_CRYP_OBJ_ALLOC 27
#define TEE_SCN_CRYP_OBJ_CLOSE 28
#define TEE_SCN_CRYP_OBJ_RESET 29
#define TEE_SCN_CRYP_OBJ_POPULATE 30
#define TEE_SCN_CRYP_OBJ_COPY 31
#define TEE_SCN_CRYP_DERIVE_KEY 32
#define TEE_SCN_CRYP_RANDOM_NUMBER_GENERATE 33
#define TEE_SCN_AUTHENC_INIT 34
#define TEE_SCN_AUTHENC_UPDATE_AAD 35
#define TEE_SCN_AUTHENC_UPDATE_PAYLOAD 36
#define TEE_SCN_AUTHENC_ENC_FINAL 37
#define TEE_SCN_AUTHENC_DEC_FINAL 38
#define TEE_SCN_ASYMM_OPERATE 39
#define TEE_SCN_ASYMM_VERIFY 40
#define TEE_SCN_STORAGE_OBJ_OPEN 41
#define TEE_SCN_STORAGE_OBJ_CREATE 42
#define TEE_SCN_STORAGE_OBJ_DEL 43
#define TEE_SCN_STORAGE_OBJ_RENAME 44
#define TEE_SCN_STORAGE_ENUM_ALLOC 45
#define TEE_SCN_STORAGE_ENUM_FREE 46
#define TEE_SCN_STORAGE_ENUM_RESET 47
#define TEE_SCN_STORAGE_ENUM_START 48
#define TEE_SCN_STORAGE_ENUM_NEXT 49
#define TEE_SCN_STORAGE_OBJ_READ 50
#define TEE_SCN_STORAGE_OBJ_WRITE 51
#define TEE_SCN_STORAGE_OBJ_TRUNC 52
#define TEE_SCN_STORAGE_OBJ_SEEK 53
#define TEE_SCN_CRYP_OBJ_GENERATE_KEY 54
/* Deprecated Secure Element API syscalls return TEE_ERROR_NOT_SUPPORTED */
#define TEE_SCN_SE_SERVICE_OPEN__DEPRECATED 55
#define TEE_SCN_SE_SERVICE_CLOSE__DEPRECATED 56
#define TEE_SCN_SE_SERVICE_GET_READERS__DEPRECATED 57
#define TEE_SCN_SE_READER_GET_PROP__DEPRECATED 58
#define TEE_SCN_SE_READER_GET_NAME__DEPRECATED 59
#define TEE_SCN_SE_READER_OPEN_SESSION__DEPRECATED 60
#define TEE_SCN_SE_READER_CLOSE_SESSIONS__DEPRECATED 61
#define TEE_SCN_SE_SESSION_IS_CLOSED__DEPRECATED 62
#define TEE_SCN_SE_SESSION_GET_ATR__DEPRECATED 63
#define TEE_SCN_SE_SESSION_OPEN_CHANNEL__DEPRECATED 64
#define TEE_SCN_SE_SESSION_CLOSE__DEPRECATED 65
#define TEE_SCN_SE_CHANNEL_SELECT_NEXT__DEPRECATED 66
#define TEE_SCN_SE_CHANNEL_GET_SELECT_RESP__DEPRECATED 67
#define TEE_SCN_SE_CHANNEL_TRANSMIT__DEPRECATED 68
#define TEE_SCN_SE_CHANNEL_CLOSE__DEPRECATED 69
/* End of deprecated Secure Element API syscalls */
#define TEE_SCN_CACHE_OPERATION 70

#define TEE_SCN_MAX 70

/* Maximum number of allowed arguments for a syscall */
#define TEE_SVC_MAX_ARGS 8

/* End of tee_syscall_numbers.h */

#endif /* SYSCALL_H */