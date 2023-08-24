#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ditto_ta.h>

#include "memory.h"

/**
 * Copy block of data from TA to Host.
 * 
 * @param sess Session handle.
 * @param from Source address in TA.
 * @param to Destination address in Host.
 * @param n Number of bytes to copy.
 */
void copy_from_ta(TEEC_Session *sess, size_t from, void *to, size_t n) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = low32(from);
    op.params[0].value.b = high32(from);
    op.params[1].tmpref.buffer = to;
    op.params[1].tmpref.size = n;
    res = TEEC_InvokeCommand(sess, CMD_COPY_FROM_TA, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_COPY_FROM_TA failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }
}

/**
 * Copy block of data from Host to TA.
 * 
 * @param sess Session handle.
 * @param to Destination address in TA.
 * @param from Source address in Host.
 * @param n Number of bytes to copy.
 */
void copy_to_ta(TEEC_Session *sess, size_t to, void *from, size_t n) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = low32(to);
    op.params[0].value.b = high32(to);
    op.params[1].tmpref.buffer = from;
    op.params[1].tmpref.size = n;
    res = TEEC_InvokeCommand(sess, CMD_COPY_TO_TA, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_COPY_TO_TA failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }
}