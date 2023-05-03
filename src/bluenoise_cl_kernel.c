char *g_bluenoise_cl_source = "#define NOISE_USE_MAX\n"\
"\n"\
"// experimentally-derived results, this ensures that less than 50% of potential swaps will take place. \n"\
"// In practice, the actual value MUST be considerably lower than this for the algorithm to converge.\n"\
"\n"\
"// Allowing 25% of swaps to take place seems to be the sweet-spot regarding both convergence speed and iteration performance.\n"\
"\n"\
"#ifdef NOISE_USE_MAX\n"\
"// the median of \"max(uniform_rand(), uniform_rand())\" is ~0.7, and the mean is 0.666\n"\
"#define CHANCE_LIMIT	0.5 // perform ~25% of potential swaps\n"\
"#else\n"\
"// the median of \"min(uniform_rand(), uniform_rand())\" is ~0.29, and the mean is 0.333\n"\
"#define CHANCE_LIMIT	0.13 // perform ~25% of potential swaps\n"\
"#endif\n"\
"\n"\
"#ifdef NOISE_USE_MAX\n"\
"#define CHANCE_COMPARE_FUNC max\n"\
"#else\n"\
"#define CHANCE_COMPARE_FUNC min\n"\
"#endif\n"\
"\n"\
"#define CONST_64BIT(x)	x##UL\n"\
"\n"\
"#define PRIME64_1	CONST_64BIT(11400714785074694791)\n"\
"#define PRIME64_2	CONST_64BIT(14029467366897019727)\n"\
"#define PRIME64_3	CONST_64BIT(1609587929392839161)\n"\
"#define PRIME64_4	CONST_64BIT(9650029242287828579)\n"\
"#define PRIME64_5	CONST_64BIT(2870177450012600261)\n"\
"\n"\
"#define FP32_MANTISSA_BITS	23\n"\
"#define FP32_EXPONENT_BITS	8\n"\
"#define FP32_EXPONENT_MASK	0x7F800000\n"\
"#define FP32_MANTISSA_MASK	0x007FFFFF\n"\
"\n"\
"#define RNG_HASH_BITS   64\n"\
"\n"\
"#ifndef M_SQRT2_F\n"\
"#define M_SQRT2_F		1.4142135623730950488016887242097f\n"\
"#endif\n"\
"\n"\
"#define GAUSSIAN_MAX_HALFWIDTH	13\n"\
"\n"\
"__constant float g_gaussian_lookup[27] = {0.00000000000000000045, 0.00000000000000023195, 0.00000000000007287710, 0.00000000001388794375, 0.00000000160522495296, 0.00000011253528242605, 0.00000478511719848029, 0.00012340968532953411, 0.00193045416381210089, 0.01831564307212829590, 0.10539919883012771606, 0.36787945032119750977, 0.77880078554153442383, 1.00000000000000000000, 0.77880078554153442383, 0.36787945032119750977, 0.10539919883012771606, 0.01831564307212829590, 0.00193045416381210089, 0.00012340968532953411, 0.00000478511719848029, 0.00000011253528242605, 0.00000000160522495296, 0.00000000001388794375, 0.00000000000007287710, 0.00000000000000023195, 0.00000000000000000045};\n"\
"\n"\
"ulong rotl64(ulong x, int shift)\n"\
"{\n"\
"	return (x << shift) | (x >> (64 - shift));\n"\
"}\n"\
"\n"\
"int Math_PopCnt64(ulong x)\n"\
"{\n"\
"	x = (x & 0x5555555555555555UL) + ((x & 0xAAAAAAAAAAAAAAAAUL) >> 1);\n"\
"	x = (x & 0x3333333333333333UL) + ((x & 0xCCCCCCCCCCCCCCCCUL) >> 2);\n"\
"	x = (x & 0x0F0F0F0F0F0F0F0FUL) + ((x & 0xF0F0F0F0F0F0F0F0UL) >> 4);\n"\
"	x = (x & 0x00FF00FF00FF00FFUL) + ((x & 0xFF00FF00FF00FF00UL) >> 8);\n"\
"	x = (x & 0x0000FFFF0000FFFFUL) + ((x & 0xFFFF0000FFFF0000UL) >> 16);\n"\
"	x = (x & 0x00000000FFFFFFFFUL) + ((x & 0xFFFFFFFF00000000UL) >> 32);\n"\
"	return (int)x;\n"\
"}\n"\
"\n"\
"int Math_LZCnt64(ulong x)\n"\
"{\n"\
"	x = x | (x >> 1);\n"\
"	x = x | (x >> 2);\n"\
"	x = x | (x >> 4);\n"\
"	x = x | (x >> 8);\n"\
"	x = x | (x >> 16);\n"\
"	x = x | (x >> 32);\n"\
"	return 64 - Math_PopCnt64(x);\n"\
"}\n"\
"\n"\
"\n"\
"ulong XXH64_Outer(ulong h64, ulong *v)\n"\
"{\n"\
"	ulong h64_l = h64;\n"\
"\n"\
"	v[0] *= PRIME64_2;\n"\
"	v[0] = rotl64(v[0], 31);\n"\
"	v[0] *= PRIME64_1;\n"\
"	h64_l ^= v[0];\n"\
"	h64_l = h64_l * PRIME64_1 + PRIME64_4;\n"\
"\n"\
"	v[1] *= PRIME64_2;\n"\
"	v[1] = rotl64(v[1], 31);\n"\
"	v[1] *= PRIME64_1;\n"\
"	h64_l ^= v[1];\n"\
"	h64_l = h64_l * PRIME64_1 + PRIME64_4;\n"\
"\n"\
"	v[2] *= PRIME64_2;\n"\
"	v[2] = rotl64(v[2], 31);\n"\
"	v[2] *= PRIME64_1;\n"\
"	h64_l ^= v[2];\n"\
"	h64_l = h64_l * PRIME64_1 + PRIME64_4;\n"\
"\n"\
"	v[3] *= PRIME64_2;\n"\
"	v[3] = rotl64(v[3], 31);\n"\
"	v[3] *= PRIME64_1;\n"\
"	h64_l ^= v[3];\n"\
"	h64_l = h64_l * PRIME64_1 + PRIME64_4;\n"\
"\n"\
"	return h64_l;\n"\
"}\n"\
"void XXH64_Inner(const uchar *p, ulong *v)\n"\
"{\n"\
"	ulong *p64 = (ulong*)p;\n"\
"\n"\
"	v[0] += (*(p64 + 0)) * PRIME64_2;\n"\
"	v[0] = rotl64(v[0], 31);\n"\
"	v[0] *= PRIME64_1;\n"\
"\n"\
"	v[1] += (*(p64 + 1)) * PRIME64_2;\n"\
"	v[1] = rotl64(v[1], 31);\n"\
"	v[1] *= PRIME64_1;\n"\
"\n"\
"	v[2] += (*(p64 + 2)) * PRIME64_2;\n"\
"	v[2] = rotl64(v[2], 31);\n"\
"	v[2] *= PRIME64_1;\n"\
"\n"\
"	v[3] += (*(p64 + 3)) * PRIME64_2;\n"\
"	v[3] = rotl64(v[3], 31);\n"\
"	v[3] *= PRIME64_1;\n"\
"}\n"\
"\n"\
"void XXH64_InitV(ulong *v, ulong seed)\n"\
"{\n"\
"    v[0] = seed + PRIME64_1 + PRIME64_2;\n"\
"    v[1] = seed + PRIME64_2;\n"\
"    v[2] = seed + 0;\n"\
"    v[3] = seed - PRIME64_1;\n"\
"}\n"\
"\n"\
"ulong Hash_XX64_32(const void* input, ulong seed)\n"\
"{\n"\
"    const uchar* p = (const uchar*)input;\n"\
"    ulong h64;\n"\
"\n"\
"    ulong v[4];\n"\
"\n"\
"	XXH64_InitV(v, seed);\n"\
"\n"\
"	XXH64_Inner(p + 0, v);\n"\
"\n"\
"    h64 = rotl64(v[0], 1) + rotl64(v[1], 7) + rotl64(v[2], 12) + rotl64(v[3], 18);\n"\
"\n"\
"	h64 = XXH64_Outer(h64, v);\n"\
"\n"\
"    h64 += (ulong)32;\n"\
"\n"\
"    h64 ^= h64 >> 33;\n"\
"    h64 *= PRIME64_2;\n"\
"    h64 ^= h64 >> 29;\n"\
"    h64 *= PRIME64_3;\n"\
"    h64 ^= h64 >> 32;\n"\
"\n"\
"    return h64;\n"\
"}\n"\
"\n"\
"float RNG_Randomf32(void *state)\n"\
"{\n"\
"    ulong current;\n"\
"    int cnt;\n"\
"    uint pw2;    \n"\
"    uint m;\n"\
"\n"\
"    current = Hash_XX64_32(state, 0);\n"\
"    cnt = (uint)Math_LZCnt64(current);\n"\
"    pw2 = cnt;\n"\
"\n"\
"    if (cnt == RNG_HASH_BITS)\n"\
"    {\n"\
"        current = Hash_XX64_32(state, 1);\n"\
"        cnt = (uint)Math_LZCnt64(current);\n"\
"        pw2 += cnt;\n"\
"    }\n"\
"\n"\
"    // if less than \"FP32_MANTISSA_BITS\" bits left, we need to generate a new hash to fill the mantissa\n"\
"    if (RNG_HASH_BITS - cnt - 1 < FP32_MANTISSA_BITS)\n"\
"    {\n"\
"        current = Hash_XX64_32(state, 2);\n"\
"    }\n"\
"\n"\
"    if (pw2 < (uint)((1 << (FP32_EXPONENT_BITS - 1)) - 2))\n"\
"        m = ((1 << (FP32_EXPONENT_BITS - 1)) - 2 - pw2) << FP32_MANTISSA_BITS;\n"\
"    else // subnormal\n"\
"        m = 0;\n"\
"\n"\
"    m |= FP32_MANTISSA_MASK & current;\n"\
"	\n"\
"    return *((float*)&m);\n"\
"}\n"\
"\n"\
"float Gaussian(float x, float sigma)\n"\
"{\n"\
"	float h0 = x / sigma;\n"\
"	float h = h0 * h0 * -0.5f;\n"\
"	return exp(h);\n"\
"}\n"\
"\n"\
"float FastExp(float x)\n"\
"{\n"\
"	// first order least squares approximation on the range [-1, 0]\n"\
"	return 0.6218299410859621f * x + 0.9430355293715387f;\n"\
"}\n"\
"\n"\
"void BlueNoise_MeasureError(int bn_width, int bn_filter_halfwidth, int bn_precise, __global float *current_data, uint x, uint y, float src0, float src1, float *err0, float *err1)\n"\
"{\n"\
"	float err[2] = {0.0f, 0.0f};\n"\
"	int lx;\n"\
"	int ly;\n"\
"\n"\
"	for (ly = -(int)bn_filter_halfwidth; ly <= (int)bn_filter_halfwidth; ly++)\n"\
"	{\n"\
"		for (lx = -(int)bn_filter_halfwidth; lx <= (int)bn_filter_halfwidth; lx++) \n"\
"		{\n"\
"			uint pos[2];\n"\
"			float v;\n"\
"			float d0;\n"\
"			float d1;\n"\
"			float gauss;\n"\
"\n"\
"			if ((lx == 0) && (ly == 0))\n"\
"				continue;\n"\
"\n"\
"			pos[0] = ((uint)(x + lx + bn_width)) & (bn_width - 1);\n"\
"			pos[1] = ((uint)(y + ly + bn_width)) & (bn_width - 1);\n"\
"\n"\
"			v = current_data[pos[1] * bn_width + pos[0]];\n"\
"			\n"\
"			d0 = fabs(v - src0);\n"\
"			d1 = fabs(v - src1);\n"\
"\n"\
"			//gauss = Gaussian(lx, M_SQRT2_F) * Gaussian(ly, M_SQRT2_F);\n"\
"			gauss = g_gaussian_lookup[GAUSSIAN_MAX_HALFWIDTH + lx]*g_gaussian_lookup[GAUSSIAN_MAX_HALFWIDTH + ly];\n"\
"\n"\
"			if (bn_precise)\n"\
"			{\n"\
"				err[0] += exp(-d0) * gauss;\n"\
"				err[1] += exp(-d1) * gauss;\n"\
"			}\n"\
"			else\n"\
"			{\n"\
"				err[0] += FastExp(-d0) * gauss;\n"\
"				err[1] += FastExp(-d1) * gauss;\n"\
"			}\n"\
"		}\n"\
"	}\n"\
"\n"\
"	*err0 = err[0];\n"\
"	*err1 = err[1];\n"\
"}\n"\
"\n"\
"\n"\
"__kernel void BlueNoise_Iterate(\n"\
"	ulong iteration_hash,\n"\
"	ulong iteration,\n"\
"	ulong option_seed,\n"\
"	int option_maximise,\n"\
"	int option_precise,\n"\
"	int option_halfwidth,\n"\
"	int target_size,\n"\
"	int block_size,\n"\
"	int block_in_x,\n"\
"	int block_in_y,\n"\
"	__global volatile uint *swaps,\n"\
"	__global float *data_buf0,\n"\
"	__global float *data_buf1,\n"\
"	uint src_index,\n"\
"	uint dst_index\n"\
")\n"\
"{\n"\
"	uchar input[32] = {0};\n"\
"	uint hash32[2];\n"\
"	uint swap_pos[2][2];\n"\
"	float chance[2];\n"\
"	float err[2][2];\n"\
"	float current_value[2];\n"\
"	float comb_err[2];\n"\
"	int perform_swap;\n"\
"	uint x;\n"\
"	uint y;\n"\
"	__global float *src_buf = src_index ? data_buf1 : data_buf0;\n"\
"	__global float *dst_buf = dst_index ? data_buf1 : data_buf0;\n"\
"	\n"\
"	x = get_global_id(0) + block_in_x * block_size;\n"\
"	y = get_global_id(1) + block_in_y * block_size;\n"\
"\n"\
"	hash32[0] = iteration_hash & 0xFFFFFFFF;\n"\
"	hash32[1] = (iteration_hash >> 32) & 0xFFFFFFFF;\n"\
"	\n"\
"	swap_pos[0][0] = x;\n"\
"	swap_pos[0][1] = y;\n"\
"	\n"\
"	swap_pos[1][0] = (swap_pos[0][0] ^ hash32[0]) & (target_size - 1);\n"\
"	swap_pos[1][1] = (swap_pos[0][1] ^ hash32[1]) & (target_size - 1);\n"\
"	\n"\
"	*((ulong*)&input[0]) = (ulong)swap_pos[0][0];\n"\
"	*((ulong*)&input[8]) = (ulong)swap_pos[0][1];\n"\
"	*((ulong*)&input[16]) = iteration;\n"\
"	*((ulong*)&input[24]) = option_seed;\n"\
"	\n"\
"	chance[0] = RNG_Randomf32(input);\n"\
"\n"\
"	*((ulong*)&input[0]) = (ulong)swap_pos[1][0];\n"\
"	*((ulong*)&input[8]) = (ulong)swap_pos[1][1];\n"\
"	*((ulong*)&input[16]) = iteration;\n"\
"	*((ulong*)&input[24]) = option_seed;\n"\
"	\n"\
"	chance[1] = RNG_Randomf32(input);\n"\
"	\n"\
"	current_value[0] = src_buf[swap_pos[0][1] * target_size + swap_pos[0][0]];\n"\
"	\n"\
"	if (CHANCE_COMPARE_FUNC(chance[0], chance[1]) > CHANCE_LIMIT)\n"\
"	{\n"\
"		dst_buf[swap_pos[0][1] * target_size + swap_pos[0][0]] = current_value[0];\n"\
"		return;\n"\
"	}\n"\
"	\n"\
"	current_value[1] = src_buf[swap_pos[1][1] * target_size + swap_pos[1][0]];\n"\
"\n"\
"	BlueNoise_MeasureError(target_size, option_halfwidth, option_precise, src_buf, swap_pos[0][0], swap_pos[0][1], current_value[0], current_value[1], &err[0][0], &err[0][1]);\n"\
"	BlueNoise_MeasureError(target_size, option_halfwidth, option_precise, src_buf, swap_pos[1][0], swap_pos[1][1], current_value[1], current_value[0], &err[1][0], &err[1][1]);\n"\
"	\n"\
"	comb_err[0] = err[0][0] + err[1][0];\n"\
"	comb_err[1] = err[0][1] + err[1][1];\n"\
"	\n"\
"	// this makes a very cool image, try it!\n"\
"	if (option_maximise)\n"\
"		perform_swap = comb_err[0] < comb_err[1];\n"\
"	else\n"\
"		perform_swap = comb_err[0] > comb_err[1];\n"\
"\n"\
"	if (perform_swap)\n"\
"	{\n"\
"		dst_buf[swap_pos[0][1] * target_size + swap_pos[0][0]] = current_value[1];\n"\
"		atomic_inc(swaps);\n"\
"	}\n"\
"	else\n"\
"	{\n"\
"		dst_buf[swap_pos[0][1] * target_size + swap_pos[0][0]] = current_value[0];\n"\
"	}\n"\
"}\n";