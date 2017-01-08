
#pragma once

typedef uint8_t vbyte_t;
typedef uint32_t vword_t;
#define VWORD_SIZE (sizeof(vword_t) / sizeof(vbyte_t))
#define IWORD_SIZE (sizeof(void*) / sizeof(vbyte_t))


