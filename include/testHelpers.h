#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#define TEST_ASSERT_STRING_CONTAINS(sub, str) do {                          \
    const char* __sub = (sub);                                              \
    const char* __str = (str);                                              \
    if (!strstr(__str, __sub)) {                                            \
        static char __msg[512];                                             \
        snprintf(__msg, sizeof(__msg),                                      \
                 "Expected substring \"%s\" not found in \"%s\"",           \
                 __sub, __str);                                             \
        TEST_FAIL_MESSAGE(__msg);                                           \
    }                                                                       \
} while(0)

#endif