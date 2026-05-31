#ifdef _MSC_VER
    #include <intrin.h>
#endif
#include <cstdint>
#include <cstring>

inline uint8_t lsbIndex(uint64_t x) 
{
    if(!x) return 64; // Return 64 for an empty bitboard, as there are no set bits
    #if defined(_MSC_VER)
        unsigned long index;
        _BitScanForward64(&index, x);
        return static_cast<uint8_t>(index);
    #elif defined(__GNUC__) || defined(__clang__)
        return __builtin_ctzll(x);

    #else 
        // Fallback for compilers without built-in functions
        static const int index[64] -
        {
            0,   1, 48,  2, 57, 49, 28, 3,
            61, 58, 50, 42, 38, 29, 17, 4,
            62, 55, 59, 36, 53, 51, 43, 22, 
            45, 39, 33, 30, 24, 18, 12, 5,
            63, 47, 56, 27, 60, 41, 37, 16, 
            54, 35, 52, 21, 44, 32, 23, 11, 
            46, 26, 40, 15, 34, 20, 31, 10,
            25, 14, 19,  9, 13,  8,  7,  6
        };
        static const uint64_t debruijn64 = 0x03f79d71b4cb0a89;
        return index[((x & -x) * debruijn64) >> 58];       
    #endif
}

inline uint8_t msbIndex(uint64_t x) 
{
    #if defined(_MSC_VER)
        unsigned long index;
        _BitScanReverse64(&index, x);
        return static_cast<uint8_t>(index);
    #elif defined(__GNUC__) || defined(__clang__)
        return 63 - __builtin_clzll(x);
    #else
        // Fallback for compilers without built-in functions
        double d = static_cast<double>(x);
        uint64_t bits;
        memcpy(&bits, &d, sizeof(bits));
        return (bits >> 52) - 1023; // Extract exponent and adjust for

    #endif
}