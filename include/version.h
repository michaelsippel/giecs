
#pragma once

#include <stdint.h>

#define MAKE_VERSION(major, minor, patch) ((uint32_t)((major&0xff) << 0x10) | ((minor&0xff) << 0x8) | (patch&0xff))
#define VERSION_MAJOR(version) ((((uint32_t)version) >> 0x10) & 0xff)
#define VERSION_MINOR(version) ((((uint32_t)version) >> 0x08) & 0xff)
#define VERSION_PATCH(version) ((((uint32_t)version) >> 0x00) & 0xff)

#define VERSION MAKE_VERSION(0,0,0)

