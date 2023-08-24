#ifndef MEMORY_H
#define MEMORY_H

#include <tee_client_api.h>

void copy_from_ta(TEEC_Session *sess, size_t from, void *to, size_t n);

void copy_to_ta(TEEC_Session *sess, size_t to, void *from, size_t n);

#endif /* MEMORY_H */