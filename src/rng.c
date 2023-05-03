#include <inttypes.h>

#define XXH_INLINE_ALL
#include "../inc/xxhash.h"
#include "../inc/math.h"

#define FP32_MANTISSA_BITS	23
#define FP32_EXPONENT_BITS	8
#define FP32_EXPONENT_MASK	0x7F800000
#define FP32_MANTISSA_MASK	0x007FFFFF

#define FP64_MANTISSA_MASK	0x000FFFFFFFFFFFFFULL
#define FP64_MANTISSA_BITS	52
#define FP64_EXPONENT_BITS	11

#define RNG_HASH_BITS   64

float RNG_Randomf32(void *state, uint32_t state_bytes)
{
    uint64_t current;
    int32_t cnt;
    uint32_t pw2;    
    uint32_t m;

    current = XXH64(state, state_bytes, 0);
    cnt = (uint32_t)Math_LZCnt64(current);
    pw2 = cnt;

    if (cnt == RNG_HASH_BITS)
    {
        current = XXH64(state, state_bytes, 1);
        cnt = (uint32_t)Math_LZCnt64(current);
        pw2 += cnt;
    }

    // if less than "FP32_MANTISSA_BITS" bits left, we need to generate a new hash to fill the mantissa
    if (RNG_HASH_BITS - cnt - 1 < FP32_MANTISSA_BITS)
    {
        current = XXH64(state, state_bytes, 2);
    }

    if (pw2 < (uint32_t)((1 << (FP32_EXPONENT_BITS - 1)) - 2))
        m = ((1 << (FP32_EXPONENT_BITS - 1)) - 2 - pw2) << FP32_MANTISSA_BITS;
    else // subnormal
        m = 0;

    m |= FP32_MANTISSA_MASK & current;

    return *((float*)&m);
}

uint64_t RNG_Randomu64(void *state, uint32_t state_bytes)
{
    return XXH64(state, state_bytes, 0);
}