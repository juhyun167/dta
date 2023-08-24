#ifndef DITTO_H
#define DITTO_H

#include <tee_client_api.h>

#define TEE_NUM_PARAMS 4

#define TEE_ATTR_FLAG_VALUE (1 << 29)

TEEC_Result ditto_invoke_command(TEEC_Session* session,
                                 uint32_t cmd_id,
                                 TEEC_Operation* operation,
                                 uint32_t* error_origin);

#endif