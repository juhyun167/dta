#ifndef DTA_TEST2_TA_H
#define DTA_TEST2_TA_H

#include <ditto_ta.h>

#define DTA_TEST2_UUID                                                         \
    {                                                                          \
        0x7f7c2c7b, 0x48ce, 0x451a, {                                          \
            0xaf, 0xa9, 0xb5, 0x94, 0xe5, 0xec, 0xa7, 0x39                     \
        }                                                                      \
    }


#define TA_AES_ALGO_ECB			0
#define TA_AES_ALGO_CBC			1
#define TA_AES_ALGO_CTR			2
#define TA_AES_SIZE_128BIT		(128 / 8)
#define TA_AES_SIZE_256BIT		(256 / 8)
#define TA_AES_MODE_ENCODE		1
#define TA_AES_MODE_DECODE		0

#define CMD_PREPARE 0
#define CMD_SET_KEY 1
#define CMD_SET_IV 2
#define CMD_CIPHER 3

#endif /* DTA_TEST2_TA_H */