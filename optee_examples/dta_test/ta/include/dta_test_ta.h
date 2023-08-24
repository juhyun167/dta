#ifndef DTA_TEST_TA_H
#define DTA_TEST_TA_H

#include <ditto_ta.h>

#define DTA_TEST_UUID                                                          \
    {                                                                          \
        0xa3463650, 0x8b86, 0x4662, {                                          \
            0xa8, 0xd5, 0x02, 0x3f, 0x2b, 0x1b, 0x03, 0x6f                     \
        }                                                                      \
    }

#define CMD_ADD 1
#define CMD_STRLEN 2
#define CMD_GET_SYSTEM_TIME 3
#define CMD_GENERATE_RANDOM 4
#define CMD_WRITE_OBJECT 5
#define CMD_READ_OBJECT 6
#define CMD_DELETE_OBJECT 7
#define CMD_GEN_KEY 8
#define CMD_ENC 9

#endif /* DTA_TEST_TA_H */