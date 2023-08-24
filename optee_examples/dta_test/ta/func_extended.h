#ifndef FUNC_EXTENDED_H
#define FUNC_EXTENDED_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

TEE_Result func_fetch_ta_addrs(uint32_t param_types, TEE_Param params[4]);
TEE_Result func_set_ditto_printf(uint32_t param_types, TEE_Param params[4]);
TEE_Result func_copy_from_ta(uint32_t param_types, TEE_Param params[4]);
TEE_Result func_copy_to_ta(uint32_t param_types, TEE_Param params[4]);
TEE_Result func_proxy_syscall(uint32_t param_types, TEE_Param params[4]);

#endif /* FUNC_EXTENDED_H */