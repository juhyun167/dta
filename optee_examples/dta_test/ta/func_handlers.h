#ifndef FUNC_HANDLERS_H
#define FUNC_HANDLERS_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

TEE_Result func_add(uint32_t param_types, TEE_Param params[4]);
TEE_Result func_strlen(uint32_t param_types, TEE_Param params[4]);

#endif /* FUNC_HANDLERS_H */