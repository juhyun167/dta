#ifndef DITTO_TA_H
#define DITTO_TA_H

#define __round_mask(x, y) ((__typeof__(x)) ((y) -1))
#define round_up(x, y) ((((x) -1) | __round_mask(x, y)) + 1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define high32(x) ((size_t) (x) >> 32);
#define low32(x) ((size_t) (x) & (((size_t) 1 << 32) - 1))

#define p64(l, h) ((l) | (size_t) (h) << 32)

#define RED "\x1B[31m"
#define YELLOW "\x1B[33m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

#define log_host(args...)                                                      \
    {                                                                          \
        printf(CYAN "host    | " RESET);                                       \
        printf(args);                                                          \
    }
#define log_ta(args...)                                                        \
    {                                                                          \
        printf(YELLOW "ta      | " RESET);                                     \
        printf(args);                                                          \
    }
#define log_error(args...)                                                     \
    {                                                                          \
        printf(RED "error   | " RESET);                                        \
        printf(args);                                                          \
    }

#define CMD_FETCH_TA_ADDRS 0xff000001
#define CMD_SET_DITTO_PRINTF 0xff000002
#define CMD_COPY_FROM_TA 0xff000003
#define CMD_COPY_TO_TA 0xff000004
#define CMD_PROXY_SYSCALL 0xff000005

#endif /* DITTO_TA_H */