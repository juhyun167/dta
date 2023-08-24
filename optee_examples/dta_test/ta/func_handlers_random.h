#ifndef FUNC_HANDLERS_RANDOM_H
#define FUNC_HANDLERS_RANDOM_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

TEE_Result func_generate_random(uint32_t param_types, TEE_Param params[4]);

#endif /* FUNC_HANDLERS_RANDOM_H */