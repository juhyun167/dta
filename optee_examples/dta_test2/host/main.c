#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include <time.h>

#include <dta_test2_ta.h>

#include "ditto.h"
#include "setup.h"

#define AES_TEST_BUFFER_SIZE 32
#define AES_TEST_KEY_SIZE 16
#define AES_BLOCK_SIZE 16

#define DECODE 0
#define ENCODE 1

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
    TEEC_UUID uuid = DTA_TEST2_UUID;
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

void prepare_aes(struct test_ctx *ctx, int encode);
void set_key(struct test_ctx *ctx, char *key, size_t key_sz);
void set_iv(struct test_ctx *ctx, char *iv, size_t iv_sz);
void cipher_buffer(struct test_ctx *ctx, char *in, char *out, size_t sz);

/**
 * Execute TA commands.
 *
 * @param ctx Context structure with open connection.
 */
void execute(struct test_ctx *ctx) {
    char key[AES_TEST_KEY_SIZE];
    char iv[AES_BLOCK_SIZE];
    char clear[AES_TEST_BUFFER_SIZE];
    char ciph[AES_TEST_BUFFER_SIZE];
    char temp[AES_TEST_BUFFER_SIZE];

    for (int i = 0; i < ITERS; i++) {
        memset(key, 0x37, sizeof(key));
        memset(iv, 0, sizeof(iv));
        memset(clear, 0x5a, sizeof(clear));

        prepare_aes(ctx, ENCODE);
        set_key(ctx, key, AES_TEST_KEY_SIZE);
        set_iv(ctx, iv, AES_BLOCK_SIZE);
        cipher_buffer(ctx, clear, ciph, AES_TEST_BUFFER_SIZE);
        log_host("[acipher] cipher: ");
        for (int i = 0; i < sizeof(ciph); i++)
            printf("%02x", ciph[i]);
        printf("\n");

        memset(key, 0x37, sizeof(key));
        memset(iv, 0, sizeof(iv));

        prepare_aes(ctx, DECODE);
        set_key(ctx, key, AES_TEST_KEY_SIZE);
        set_iv(ctx, iv, AES_BLOCK_SIZE);
        cipher_buffer(ctx, ciph, temp, AES_TEST_BUFFER_SIZE);
        log_host("[acipher] plaintext: ");
        for (int i = 0; i < sizeof(temp); i++)
            printf("%02x", temp[i]);
        printf("\n");
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

void prepare_aes(struct test_ctx *ctx, int encode) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                     TEEC_VALUE_INPUT, TEEC_NONE);

    op.params[0].value.a = TA_AES_ALGO_CTR;
    op.params[1].value.a = TA_AES_SIZE_128BIT;
    op.params[2].value.a = encode ? TA_AES_MODE_ENCODE : TA_AES_MODE_DECODE;

    res = INVOKE_COMMAND(&ctx->sess, CMD_PREPARE, &op, &origin);
    if (res != TEEC_SUCCESS)
        log_error("CMD_PREPARE failed. (res=0x%x, origin=0x%x)\n", res, origin);
}

void set_key(struct test_ctx *ctx, char *key, size_t key_sz) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);

    op.params[0].tmpref.buffer = key;
    op.params[0].tmpref.size = key_sz;

    res = INVOKE_COMMAND(&ctx->sess, CMD_SET_KEY, &op, &origin);
    if (res != TEEC_SUCCESS)
        log_error("CMD_SET_KEY failed. (res=0x%x, origin=0x%x)\n", res, origin);
}

void set_iv(struct test_ctx *ctx, char *iv, size_t iv_sz) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = iv;
    op.params[0].tmpref.size = iv_sz;

    res = INVOKE_COMMAND(&ctx->sess, CMD_SET_IV, &op, &origin);
    if (res != TEEC_SUCCESS)
        log_error("CMD_SET_IV failed. (res=0x%x, origin=0x%x)\n", res, origin);
}

void cipher_buffer(struct test_ctx *ctx, char *in, char *out, size_t sz) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = in;
    op.params[0].tmpref.size = sz;
    op.params[1].tmpref.buffer = out;
    op.params[1].tmpref.size = sz;

    res = INVOKE_COMMAND(&ctx->sess, CMD_CIPHER, &op, &origin);
    if (res != TEEC_SUCCESS)
        log_error("CMD_CIPHER failed. (res=0x%x, origin=0x%x)\n", res, origin);
}