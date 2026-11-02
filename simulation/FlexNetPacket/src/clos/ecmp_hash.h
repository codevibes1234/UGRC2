#ifndef ECMP_HASH_H
#define ECMP_HASH_H

#include <cstddef>
#include <cstdint>

inline size_t ecmp_path_index(int src, int dst, size_t npaths)
{
    if (npaths <= 1) return 0;
    uint64_t x = ((uint64_t)(uint32_t)src << 32) ^ (uint64_t)(uint32_t)dst;
    x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (size_t)(x % (uint64_t)npaths);
}

#endif
