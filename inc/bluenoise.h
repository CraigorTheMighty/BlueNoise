typedef struct bluenoise_s
{
	int			maximise_energy;
	int			precise;
	float		*gaussian;
	uint32_t	gaussian_halfwidth;
	uint64_t	seed;
	uint64_t	iteration;
	uint32_t	width;
	float		*data[2];
	float		(*generate_func)(const uint32_t x, const uint32_t y, const uint64_t seed, const void *context);
}bluenoise_t;

void BlueNoise_Destroy(bluenoise_t *bn);
bluenoise_t BlueNoise_Create(uint32_t width, int window_halfwidth, uint64_t seed, int is_precise, int maximise_energy, void *context, float (*generate_func)(const uint32_t x, const uint32_t y, const uint64_t seed, const void *context));
double BlueNoise_Iterate(bluenoise_t *bn);

extern char *g_bluenoise_cl_source;