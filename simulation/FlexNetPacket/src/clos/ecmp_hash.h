#ifndef ECMP_HASH_H
#define ECMP_HASH_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>

inline uint64_t ecmp_salt = 0;
inline bool ecmp_per_flow = false;

inline std::map<std::pair<int, int>, uint64_t>& ecmp_flow_counters()
{
    static std::map<std::pair<int, int>, uint64_t> counters;
    return counters;
}

inline size_t ecmp_path_index(int src, int dst, size_t npaths)
{
    if (npaths <= 1) return 0;
    uint64_t x = ((uint64_t)(uint32_t)src << 32) ^ (uint64_t)(uint32_t)dst;
    if (ecmp_salt) x ^= ecmp_salt * 0x9e3779b97f4a7c15ULL;
    if (ecmp_per_flow) x ^= (++ecmp_flow_counters()[std::make_pair(src, dst)]) * 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (size_t)(x % (uint64_t)npaths);
}

#endif
