/**
 * @file	only_once_allocator_cfg.h
 * @brief	only once allocator configuration
 */

//#define ONLY_ONCE_ALLOCATOR_MEMORY_POOL_BASE 0x00000000UL

enum { kONLY_ONCE_ALLOCATOR_SIZE_MAX = (1024 * 16) };

static const uintptr_t kALIGNMENT_UNIT = (1UL << 3); /*!< power-of-two */
