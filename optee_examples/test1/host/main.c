#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

#include <test1_ta.h>

#include "ditto.h"
#include "setup.h"

#define SENDTO_TA 0
#define SENDTO_DITTO 1

#define INVOKE_COMMAND(session, cmd_id, operation, error_origin)               \
    ((dest == SENDTO_TA) ? TEEC_InvokeCommand((session), (cmd_id),             \
                                              (operation), (error_origin))     \
                         : ditto_invoke_command((session), (cmd_id),           \
                                                (operation), (error_origin)))

struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
} ctx;

int dest;

void prepare_tee_session(struct test_ctx *ctx) {
    TEEC_UUID uuid = TEST1_UUID;
    uint32_t origin;
    TEEC_Result res;

    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS)
        errx(EXIT_FAILURE, "TEEC_InitializeContext() failed.");

    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC,
                           NULL, NULL, &origin);
    if (res != TEEC_SUCCESS)
        errx(EXIT_FAILURE, "TEEC_OpenSession() failed.");
}

void terminate_tee_session(struct test_ctx *ctx) {
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
}

void execute(struct test_ctx *ctx) {
    TEEC_Operation op;
    uint32_t origin;
    char buf[32] = { 0 };

    fgets(buf, sizeof(buf), stdin);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE, TEEC_NONE);
    op.params[0].tmpref.buffer = buf;
    op.params[0].tmpref.size = sizeof(buf);

    INVOKE_COMMAND(&ctx->sess, CMD_CRASHME, &op, &origin);
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
