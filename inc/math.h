#define USE_BUILTIN_FUNCTIONS

#ifdef USE_BUILTIN_FUNCTIONS

#define LZCNT8(x)	return (int)__lzcnt16((uint16_t)x);
#define LZCNT16(x)	return (int)__lzcnt16(x);
#define LZCNT32(x)	return (int)__lzcnt(x);
#define LZCNT64(x)	return (int)__lzcnt64(x);

#define POPCNT8(x)	return (int)__popcnt16((uint16_t)x);
#define POPCNT16(x)	return (int)__popcnt16(x);
#define POPCNT32(x)	return (int)__popcnt(x);
#define POPCNT64(x)	return (int)__popcnt64(x);

#define ROTL8(x, shift)		(x << shift) | (x >> (8 - shift))
#define ROTL16(x, shift)	(x << shift) | (x >> (16 - shift))
#define ROTL32(x, shift)	_rotl(x, shift)
#define ROTL64(x, shift)	_rotl64(x, shift)

#else

#define POPCNT8(x)	x = (x & 0x55) + ((x & 0xAA) >> 1);\
					x = (x & 0x33) + ((x & 0xCC) >> 2);\
					x = (x & 0x0F) + ((x & 0xF0) >> 4);\
					return (int)x;

#define POPCNT16(x)	x = (x & 0x5555) + ((x & 0xAAAA) >> 1);\
					x = (x & 0x3333) + ((x & 0xCCCC) >> 2);\
					x = (x & 0x0F0F) + ((x & 0xF0F0) >> 4);\
					x = (x & 0x00FF) + ((x & 0xFF00) >> 8);\
					return (int)x;

#define POPCNT32(x)	x = (x & 0x55555555) + ((x & 0xAAAAAAAA) >> 1);\
					x = (x & 0x33333333) + ((x & 0xCCCCCCCC) >> 2);\
					x = (x & 0x0F0F0F0F) + ((x & 0xF0F0F0F0) >> 4);\
					x = (x & 0x00FF00FF) + ((x & 0xFF00FF00) >> 8);\
					x = (x & 0x0000FFFF) + ((x & 0xFFFF0000) >> 16);\
					return (int)x;

#define POPCNT64(x)	x = (x & 0x5555555555555555ULL) + ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);\
					x = (x & 0x3333333333333333ULL) + ((x & 0xCCCCCCCCCCCCCCCCULL) >> 2);\
					x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x & 0xF0F0F0F0F0F0F0F0ULL) >> 4);\
					x = (x & 0x00FF00FF00FF00FFULL) + ((x & 0xFF00FF00FF00FF00ULL) >> 8);\
					x = (x & 0x0000FFFF0000FFFFULL) + ((x & 0xFFFF0000FFFF0000ULL) >> 16);\
					x = (x & 0x00000000FFFFFFFFULL) + ((x & 0xFFFFFFFF00000000ULL) >> 32);\
					return (int)x;

#define LZCNT8(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					return 8 - Math_PopCnt8(x);

#define LZCNT16(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					return 16 - Math_PopCnt16(x);

#define LZCNT32(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					x = x | (x >> 16);\
					return 32 - Math_PopCnt32(x);

#define LZCNT64(x)	x = x | (x >> 1);\
				    x = x | (x >> 2);\
					x = x | (x >> 4);\
					x = x | (x >> 8);\
					x = x | (x >> 16);\
					x = x | (x >> 32);\
					return 64 - Math_PopCnt64(x);

#define ROTL8(x, shift)		(x << shift) | (x >> (8 - shift))
#define ROTL16(x, shift)	(x << shift) | (x >> (16 - shift))
#define ROTL32(x, shift)	(x << shift) | (x >> (32 - shift))
#define ROTL64(x, shift)	(x << shift) | (x >> (64 - shift))

#endif

static __forceinline int Math_LZCnt64(uint64_t x)
{
	LZCNT64(x);
}

static __forceinline uint32_t Math_FloorPow2u32(uint32_t x)
{
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);

	return x - (x >> 1);
}

static __forceinline uint64_t Math_FloorPow2u64(uint64_t x)
{
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	x = x | (x >> 32);

	return x - (x >> 1);
}

#define M_PI			3.1415926535897932384626433832795
#define M_PI_F			3.1415926535897932384626433832795f

#define M_SQRT2			1.4142135623730950488016887242097
#define M_SQRT2_F		1.4142135623730950488016887242097f

#define M_SQRT_2PI		2.506628274631000502415765284811
#define M_SQRT_2PI_F	2.506628274631000502415765284811f