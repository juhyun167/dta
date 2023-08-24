#ifndef FUNC_HANDLERS_ACIPHER_H
#define FUNC_HANDLERS_ACIPHER_H

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

struct acipher {
    TEE_ObjectHandle key;
};

TEE_Result func_gen_key(struct acipher *state, uint32_t param_types,
                        TEE_Param params[4]);
TEE_Result func_enc(struct acipher *state, uint32_t param_types,
                    TEE_Param params[4]);

#endif /* FUNC_HANDLERS_ACIPHER_H */