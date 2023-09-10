#pragma once

#include <Arduino.h>

#define ST(A) #A
#define STR(A) ST(A)

#ifdef DEBUG
#define D true
#else
#define D false
#endif

#define D_Println(...)                   \
    do                                   \
    {                                    \
        if (D)                           \
            Serial.println(__VA_ARGS__); \
    } while (0)

#define D_Printf(...)                   \
    do                                  \
    {                                   \
        if (D)                          \
            Serial.printf(__VA_ARGS__); \
    } while (0)
