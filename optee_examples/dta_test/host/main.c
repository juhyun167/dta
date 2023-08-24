#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include <time.h>

#include <dta_test_ta.h>

#include "ditto.h"
#include "setup.h"

#define SENDTO_TA 0
#define SENDTO_DITTO 1

#define ITERS 10

#define INVOKE_COMMAND(session, cmd_id, operation, error_origin)               \
    ((dest == SENDTO_TA) ? TEEC_InvokeCommand((session), (cmd_id),             \
                                              (operation), (error_origin))     \
                         : ditto_invoke_command((session), (cmd_id),           \
                                                (operation), (error_origin)))

int dest = SENDTO_TA;

/* TEE and TA context structure */
struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
} ctx;

/**
 * Start session with TEE and TA.
 *
 * @param ctx Empty context structure.
 */
void prepare_tee_session(struct test_ctx *ctx) {
    TEEC_UUID uuid = DTA_TEST_UUID;
    uint32_t origin;
    TEEC_Result res;

    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS)
        log_error("TEEC_InitializeContext failed. (res=0x%x)\n", res);

    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC,
                           NULL, NULL, &origin);
    if (res != TEEC_SUCCESS)
        log_error("TEEC_OpenSession failed. (res=0x%x, origin=0x%u\n)", res,
                  origin);
}

/**
 * End session with TEE and TA.
 *
 * @param ctx Context structure with open connection.
 */
void terminate_tee_session(struct test_ctx *ctx) {
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
}

TEEC_Result get_system_time(struct test_ctx *ctx);

TEEC_Result generate_random(struct test_ctx *ctx, char *buf, uint32_t len);
TEEC_Result read_secure_object(struct test_ctx *ctx, char *id, char *data,
                               size_t data_len);
TEEC_Result write_secure_object(struct test_ctx *ctx, char *id, char *data,
                                size_t data_len);
TEEC_Result delete_secure_object(struct test_ctx *ctx, char *id);
TEEC_Result gen_key(struct test_ctx *ctx, size_t key_size);
TEEC_Result enc(struct test_ctx *ctx, char *inbuf, size_t inbuf_len);

/**
 * Execute TA commands.
 *
 * @param ctx Context structure with open connection.
 */
void execute(struct test_ctx *ctx) {
    // time test
    for (int i = 0; i < ITERS; i++)
        get_system_time(ctx);

    // random test
    char buf[64] = { 0 };

    for (int i = 0; i < ITERS; i++)
        generate_random(ctx, buf, sizeof(buf));

    // acipher test
    size_t key_size = 256;
    char inbuf[] = "helloworld";

    for (int i = 0; i < ITERS; i++) {
        gen_key(ctx, key_size);
        enc(ctx, inbuf, sizeof(inbuf));
    }

    // secure storage test
    char obj_id[] = "object#1";
    char obj_data[32] = { 0 };
    char read_data[32] = { 0 };

    for (int i = 0; i < ITERS; i++) {
        memset(obj_data, 'a', sizeof(obj_data) - 1);
        write_secure_object(ctx, obj_id, obj_data, sizeof(obj_data));
        read_secure_object(ctx, obj_id, read_data, sizeof(read_data));
        delete_secure_object(ctx, obj_id);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1 && !strcmp(argv[1], "-d"))
        dest = SENDTO_DITTO;

    prepare_tee_session(&ctx);

    if (dest == SENDTO_DITTO)
        setup(&ctx.sess);

    execute(&ctx);

    terminate_tee_session(&ctx);

    return 0;
}

TEEC_Result get_system_time(struct test_ctx *ctx) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes =
        TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    res = INVOKE_COMMAND(&ctx->sess, CMD_GET_SYSTEM_TIME, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_GET_SYSTEM_TIME failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    uint32_t seconds = op.params[0].value.a;
    uint32_t millis = op.params[0].value.b;

    log_host("[time] seconds: %u, millis: %u\n", seconds, millis);

    return TEEC_SUCCESS;
}

TEEC_Result generate_random(struct test_ctx *ctx, char *buf, uint32_t len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT, TEEC_VALUE_INPUT,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = buf;
    op.params[0].tmpref.size = len;
    op.params[1].value.a = len;

    res = INVOKE_COMMAND(&ctx->sess, CMD_GENERATE_RANDOM, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_GENERATE_RANDOM failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    log_host("[random] buf: ");
    for (int i = 0; i < len; i++)
        printf("%02x", buf[i]);
    printf("\n");

    return res;
}

TEEC_Result read_secure_object(struct test_ctx *ctx, char *id, char *data,
                               size_t data_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;
    op.params[1].tmpref.buffer = data;
    op.params[1].tmpref.size = data_len;

    res = INVOKE_COMMAND(&ctx->sess, CMD_READ_OBJECT, &op, &origin);

    switch (res) {
    case TEEC_SUCCESS:
    case TEEC_ERROR_SHORT_BUFFER:
    case TEEC_ERROR_ITEM_NOT_FOUND:
        break;
    default:
        log_error("CMD_READ_OBJECT failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    log_host("[storage] {\"%s\": \"%s\"}\n", id, data);

    return res;
}

TEEC_Result write_secure_object(struct test_ctx *ctx, char *id, char *data,
                                size_t data_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;
    op.params[1].tmpref.buffer = data;
    op.params[1].tmpref.size = data_len;

    res = INVOKE_COMMAND(&ctx->sess, CMD_WRITE_OBJECT, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_WRITE_OBJECT failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    return res;
}

TEEC_Result delete_secure_object(struct test_ctx *ctx, char *id) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;
    size_t id_len = strlen(id);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    res = INVOKE_COMMAND(&ctx->sess, CMD_DELETE_OBJECT, &op, &origin);

    switch (res) {
    case TEEC_SUCCESS:
    case TEEC_ERROR_ITEM_NOT_FOUND:
        break;
    default:
        log_error("CMD_DELETE_OBJECT failed. (res=0x%x, origin=0x%x)\n", res,
                  origin);
        exit(0);
    }

    return res;
}

TEEC_Result gen_key(struct test_ctx *ctx, size_t key_size) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes =
        TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    op.params[0].value.a = key_size;

    res = INVOKE_COMMAND(&ctx->sess, CMD_GEN_KEY, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_GEN_KEY failed. (res=0x%x, origin=0x%x)\n", res, origin);
        exit(0);
    }

    return res;
}

TEEC_Result enc(struct test_ctx *ctx, char *inbuf, size_t inbuf_len) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = inbuf;
    op.params[0].tmpref.size = inbuf_len;

    res = INVOKE_COMMAND(&ctx->sess, CMD_ENC, &op, &origin);

    if (origin != TEEC_ORIGIN_TRUSTED_APP || res != TEEC_ERROR_SHORT_BUFFER) {
        log_error("CMD_ENC failed. (res=0x%x, origin=0x%x)\n", res, origin);
        exit(0);
    }

    size_t outbuf_len = op.params[1].tmpref.size;
    char *outbuf = malloc(outbuf_len);

    if (!outbuf) {
        log_error("Cannot allocate out buffer of size %lu.\n", outbuf_len);
        exit(0);
    }

    op.params[1].tmpref.buffer = outbuf;

    res = INVOKE_COMMAND(&ctx->sess, CMD_ENC, &op, &origin);

    if (res != TEEC_SUCCESS) {
        log_error("CMD_ENC failed. (res=0x%x, origin=0x%x)\n", res, origin);
        exit(0);
    }

    log_host("[acipher] encrypted: ");
    for (int i = 0; i < outbuf_len; i++)
        printf("%02x", outbuf[i]);
    printf("\n");
}