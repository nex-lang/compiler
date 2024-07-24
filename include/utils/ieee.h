/*
The IEEE Standard for Floating-Point Arithmetic (IEEE 754)
*/

#include <stdint.h>
#include <stdio.h>

#define DOUBLE_TO_IEEE_HEX(value, hex_str) do { \
    sprintf((hex_str), "0x%016lX", value); \
} while (0)

#define FLOAT_TO_IEEE_HEX(value, hex_str) do { \
    union { \
        float f; \
        uint32_t u; \
    } float_union; \
    float_union.f = (value); \
    sprintf((hex_str), "0x%08X", float_union.u); \
} while (0)
