#pragma once

#ifdef assert
#undef assert
#endif

#include <stdexcept>

#define assert(condition)                                                                  \
    do {                                                                                   \
        if (!(condition)) {                                                                \
            throw std::runtime_error("Assertion failed: " #condition);                    \
        }                                                                                  \
    } while (false)
