#ifndef SETUP_H
#define SETUP_H

#include <tee_client_api.h>

#define PAGE_SIZE 0x1000

void setup(TEEC_Session *sess);

#endif /* SETUP_H */