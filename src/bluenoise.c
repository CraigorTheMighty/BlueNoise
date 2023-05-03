#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

// #include <xmmintrin.h>
// #include <emmintrin.h>

#include "..\inc\rng.h"
#include "..\inc\math.h"
#include "..\inc\bluenoise.h"

#define NOISE_USE_MAX

// experimentally-derived results, this ensures that less than 50% of potential swaps will take place. 
// In practice, the actual value MUST be considerably lower than this for the algorithm to converge.

// Allowing 25% of swaps to take place seems to be the sweet-spot regarding both convergence speed and iteration performance.

#ifdef NOISE_USE_MAX
// the median of "max(uniform_rand(), uniform_rand())" is ~0.7, and the mean is 0.666
#define CHANCE_LIMIT	0.5 // perform ~25% of potential swaps
#else
// the median of "min(uniform_rand(), uniform_rand())" is ~0.29, and the mean is 0.333
#define CHANCE_LIMIT	0.13 // perform ~25% of potential swaps
#endif

#ifdef NOISE_USE_MAX
#define CHANCE_COMPARE_FUNC max
#else
#define CHANCE_COMPARE_FUNC min
#endif

static float BlueNoise_GenerateDefaultf32(const uint32_t x, const uint32_t y, const uint64_t seed, const void *context)
{
	uint8_t buffer[sizeof(uint32_t)*2 + sizeof(uint64_t)];

	*(uint64_t*)(&buffer[0]) = seed;
	*(uint32_t*)(&buffer[sizeof(uint64_t)]) = x;
	*(uint32_t*)(&buffer[sizeof(uint64_t) + sizeof(uint32_t)]) = y;

	return RNG_Randomf32(buffer, sizeof(uint32_t)*2 + sizeof(uint64_t));
}

static float Gaussian(float x, float sigma)
{
	float h0 = x / sigma;
	float h = h0 * h0 * -0.5f;
	return expf(h);
}

static __forceinline float FastExp(float x)
{
	// first order least squares approximation on the range [-1, 0]
	return 0.6218299410859621f * x + 0.9430355293715387f;
}
/*
// 5th order Taylor approximation on two values simultaneously. Very accurate on [-1, 0], faster than expf(), but still quite slow.
static __forceinline void FastExp2(float x0, float x1, float *x0out, float *x1out)
{
	__m128 xin = {x0, x1, x1, x1};
	__m128 calc;
	const __m128 additive[5] = 
	{
		{6331.0f, 6331.0f, 6331.0f, 6331.0f},
		{3165.0f, 3165.0f, 3165.0f, 3165.0f},
		{395.0f, 395.0f, 395.0f, 395.0f},
		{65.0f, 65.0f, 65.0f, 65.0f},
		{15.0f, 15.0f, 15.0f, 15.0f},
	};
	const __m128 multiplicative[6] = 
	{
		{2.0f, 2.0f, 2.0f, 2.0f},
		{4.0f, 4.0f, 4.0f, 4.0f},
		{2.0f, 2.0f, 2.0f, 2.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{2.0f, 2.0f, 2.0f, 2.0f},
		{0.0001579506926334983f, 0.0001579506926334983f, 0.0001579506926334983f, 0.0001579506926334983f},
	};

	calc = _mm_mul_ps(xin, multiplicative[4]);
	calc = _mm_add_ps(calc, additive[4]);
	calc = _mm_mul_ps(calc, xin);
	calc = _mm_mul_ps(calc, multiplicative[3]);
	calc = _mm_add_ps(calc, additive[3]);
	calc = _mm_mul_ps(calc, xin);
	calc = _mm_mul_ps(calc, multiplicative[2]);
	calc = _mm_add_ps(calc, additive[2]);
	calc = _mm_mul_ps(calc, xin);
	calc = _mm_mul_ps(calc, multiplicative[1]);
	calc = _mm_add_ps(calc, additive[1]);
	calc = _mm_mul_ps(calc, xin);
	calc = _mm_mul_ps(calc, multiplicative[0]);
	calc = _mm_add_ps(calc, additive[0]);
	calc = _mm_mul_ps(calc, multiplicative[5]);

	*x0out = calc.m128_f32[0];
	*x1out = calc.m128_f32[1];
}
*/
static void BlueNoise_MeasureError(bluenoise_t *bn, float *current_data, uint32_t x, uint32_t y, float src0, float src1, float *err0, float *err1)
{
	float err[2] = {0.0f, 0.0f};
	int32_t lx;
	int32_t ly;

	for (ly = -(int32_t)bn->gaussian_halfwidth; ly <= (int32_t)bn->gaussian_halfwidth; ly++)
	{
		for (lx = -(int32_t)bn->gaussian_halfwidth; lx <= (int32_t)bn->gaussian_halfwidth; lx++) 
		{
			uint32_t pos[2];
			float v;
			float d0;
			float d1;
			float gauss;

			if ((lx == 0) && (ly == 0))
				continue;

			pos[0] = ((uint32_t)(x + lx + bn->width)) & (bn->width - 1);
			pos[1] = ((uint32_t)(y + ly + bn->width)) & (bn->width - 1);

			v = current_data[pos[1] * bn->width + pos[0]];

			d0 = fabsf(v - src0);
			d1 = fabsf(v - src1);

			gauss = bn->gaussian[lx + bn->gaussian_halfwidth] * bn->gaussian[ly + bn->gaussian_halfwidth];

			if (bn->precise)
			{
				err[0] += expf(-d0) * gauss;
				err[1] += expf(-d1) * gauss;
			}
			else
			{
				err[0] += FastExp(-d0) * gauss;
				err[1] += FastExp(-d1) * gauss;
			}
		}
	}

	*err0 = err[0];
	*err1 = err[1];
}

double BlueNoise_Iterate(bluenoise_t *bn)
{
	uint64_t hash_buffer[2];
	uint64_t rand_buffer[4]; // false C4101 in VS2022 with OpenMP
	uint64_t hash;
	uint32_t hash32[2];
	int32_t x; // false C4101 in VS2022 with OpenMP
	int32_t y; // false C4101 in VS2022 with OpenMP
	uint64_t swaps = 0;
	float *current_data = bn->data[bn->iteration & 1];
	float *out_data = bn->data[(bn->iteration + 1) & 1];

	hash_buffer[0] = bn->seed;
	hash_buffer[1] = bn->iteration;

	hash = RNG_Randomu64(hash_buffer, sizeof(uint64_t) * 2);

	hash32[0] = hash & 0xFFFFFFFF;
	hash32[1] = (hash >> 32) & 0xFFFFFFFF;

#pragma omp parallel for private(y) private(x) private(rand_buffer) reduction(+:swaps) // reduction(+:potential_swaps)
	for (y = 0; y < (int32_t)bn->width; y++)
		for (x = 0; x < (int32_t)bn->width; x++)
		{
			uint32_t swap_pos[2][2];
			float chance[2];
			float err[2][2];
			float current_value[2];
			float comb_err[2];
			int perform_swap;

			swap_pos[0][0] = (uint32_t)x;
			swap_pos[0][1] = (uint32_t)y;

			// this is a one-to-one mapping between pairs of pixels
			swap_pos[1][0] = (swap_pos[0][0] ^ hash32[0]) & (bn->width - 1);
			swap_pos[1][1] = (swap_pos[0][1] ^ hash32[1]) & (bn->width - 1);

			rand_buffer[0] = (uint64_t)swap_pos[0][0];
			rand_buffer[1] = (uint64_t)swap_pos[0][1];
			rand_buffer[2] = (uint64_t)bn->iteration;
			rand_buffer[3] = (uint64_t)bn->seed;

			chance[0] = RNG_Randomf32(rand_buffer, sizeof(uint64_t) * 4);

			rand_buffer[0] = (uint64_t)swap_pos[1][0];
			rand_buffer[1] = (uint64_t)swap_pos[1][1];
			rand_buffer[2] = (uint64_t)bn->iteration;
			rand_buffer[3] = (uint64_t)bn->seed;

			chance[1] = RNG_Randomf32(rand_buffer, sizeof(uint64_t) * 4);

			current_value[0] = current_data[swap_pos[0][1] * bn->width + swap_pos[0][0]];

			// any function that gives output that's order-independent with respect to chance[0] and chance[1] will work here
			if (CHANCE_COMPARE_FUNC(chance[0], chance[1]) > CHANCE_LIMIT)
			{
				out_data[swap_pos[0][1] * bn->width + swap_pos[0][0]] = current_value[0];
				continue;
			}

			current_value[1] = current_data[swap_pos[1][1] * bn->width + swap_pos[1][0]];

			BlueNoise_MeasureError(bn, current_data, swap_pos[0][0], swap_pos[0][1], current_value[0], current_value[1], &err[0][0], &err[0][1]);
			BlueNoise_MeasureError(bn, current_data, swap_pos[1][0], swap_pos[1][1], current_value[1], current_value[0], &err[1][0], &err[1][1]);

			comb_err[0] = err[0][0] + err[1][0];
			comb_err[1] = err[0][1] + err[1][1];

			// this makes a very cool image, try it!
			if (bn->maximise_energy)
				perform_swap = comb_err[0] < comb_err[1];
			else
				perform_swap = comb_err[0] > comb_err[1];

			if (perform_swap)
			{
				out_data[swap_pos[0][1] * bn->width + swap_pos[0][0]] = current_value[1];
				swaps++;
			}
			else
			{
				out_data[swap_pos[0][1] * bn->width + swap_pos[0][0]] = current_value[0];
			}
		}

	bn->iteration = bn->iteration + 1;

	return (double)swaps / ((double)bn->width * (double)bn->width);
}

bluenoise_t BlueNoise_Create(uint32_t width, int window_halfwidth, uint64_t seed, int is_precise, int maximise_energy, void *context, float (*generate_func)(const uint32_t x, const uint32_t y, const uint64_t seed, const void *context))
{
	bluenoise_t bn = {0};
	int32_t x;
	int32_t y; // false C4101 in VS2022 with OpenMP

	if (width > (1 << 15))
		width = 1 << 15;
	if (width < 1)
		width = 1;

	width = Math_FloorPow2u32(width);

	bn.maximise_energy = maximise_energy;
	bn.precise = is_precise;
	bn.gaussian_halfwidth = window_halfwidth;
	if (bn.gaussian_halfwidth < 1)
		bn.gaussian_halfwidth = 1;
	bn.gaussian = malloc(sizeof(float) * (bn.gaussian_halfwidth * 2 + 1));
	if (bn.gaussian == 0)
	{
		memset(&bn, 0, sizeof(bluenoise_t));
		return bn;
	}
	for (x = -(int32_t)bn.gaussian_halfwidth; x <= (int32_t)bn.gaussian_halfwidth; x++)
		bn.gaussian[x + bn.gaussian_halfwidth] = Gaussian((float)x, M_SQRT2_F);
	bn.seed = seed;
	bn.iteration = 0;
	if (!generate_func)
		bn.generate_func = BlueNoise_GenerateDefaultf32;
	else
		bn.generate_func = generate_func;
	bn.width = width;
	bn.data[0] = malloc((size_t)bn.width * (size_t)bn.width * sizeof(float));

	if (bn.data[0] == 0)
	{
		free(bn.gaussian);
		memset(&bn, 0, sizeof(bluenoise_t));
		return bn;
	}

	bn.data[1] = malloc((size_t)bn.width * (size_t)bn.width * sizeof(float));

	if (bn.data[1] == 0)
	{
		free(bn.gaussian);
		free(bn.data[0]);
		memset(&bn, 0, sizeof(bluenoise_t));
		return bn;
	}

	// safe as "bn.width" always fits in an int32_t
#pragma omp parallel for private(y) private(x) shared(bn) shared(seed) shared(context)
	for (y = 0; y < (int32_t)bn.width; y++)
		for (x = 0; x < (int32_t)bn.width; x++)
		{
			size_t offset = (size_t)y * (size_t)bn.width + (size_t)x;
			float *ptr = bn.data[0];

			// false positive C6386 warning in VS2022
			ptr[offset] = bn.generate_func((uint32_t)x, (uint32_t)y, seed, context);
			ptr[offset] = max(min(ptr[offset], 1.0f), 0.0f);
		}

	return bn;
}

void BlueNoise_Destroy(bluenoise_t *bn)
{
	free(bn->gaussian);
	free(bn->data[0]);
	free(bn->data[1]);
	memset(bn, 0, sizeof(bluenoise_t));
}