#include <windows.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#define CL_TARGET_OPENCL_VERSION	200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <cl\cl.h>

#include "..\inc\bluenoise.h"
#include "..\inc\rng.h"
#include "..\inc\timer.h"
#include "..\inc\image.h"

#include "..\inc\opencl.h"

#include "..\inc\IL\il.h"
#include "..\inc\IL\ilu.h"
#include "..\inc\IL\ilut.h"

#define BN_DEFAULT_HALFWIDTH	5
#define BN_MAX_HALFWIDTH		13
#define BN_MIN_HALFWIDTH		3

#define BN_DEFAULT_IMAGE_SIZE	256
#define BN_MAX_IMAGE_SIZE		8192
#define BN_MIN_IMAGE_SIZE		2

#define BN_DEFAULT_PERCENT		1.0
#define BN_MIN_PERCENT			0.1
#define BN_MAX_PERCENT			100.0

static int g_precise = 0;
static char *g_output[2] = {0};
static int g_channels = 1;
static int g_halfwidth = BN_DEFAULT_HALFWIDTH;
static uint64_t g_seed = 0;
static double g_percent = BN_DEFAULT_PERCENT; // 1.0 = 200 iterations, 0.5 = 400 iterations, 0.25 = 1200 iterations, 0.1 = 3500 iterations
static int g_ktx = 0;
static int g_ktx_only = 0;
static int g_max = 0;
static int g_size = BN_DEFAULT_IMAGE_SIZE;
static int g_iterations = -1;
static char g_last_error[2048] = "";
static int g_useopencl = 0;
static int g_cl_device_index = 0;

int System_ParseOptions(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-output"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_output[0] = malloc(strlen(argv[i]) + 5);
			g_output[1] = malloc(strlen(argv[i]) + 5);

			if (!g_output[0] || !g_output[1])
			{
				free(g_output[0]);
				free(g_output[1]);
				sprintf_s(g_last_error, 2048, "Error allocating \"g_output\" (size %zu)", strlen(argv[i]) + 4);
				return 1;
			}
			strcpy_s(g_output[0], strlen(argv[i]) + 1, argv[i]);
			strcpy_s(g_output[1], strlen(argv[i]) + 1, argv[i]);
			if (!strrchr(g_output[0], '.'))
			{
				char *ptr = &g_output[0][strlen(argv[i])];
				ptr[0] = '.';
				ptr[1] = 'p';
				ptr[2] = 'n';
				ptr[3] = 'g';
				ptr[4] = 0;
			}
			if (strrchr(g_output[1], '.'))
			{
				char *ptr = strrchr(g_output[1], '.');
				ptr[1] = 'k';
				ptr[2] = 't';
				ptr[3] = 'x';
				ptr[4] = 0;
			}
			else
			{
				char *ptr = &g_output[1][strlen(argv[i])];
				ptr[0] = '.';
				ptr[1] = 'k';
				ptr[2] = 't';
				ptr[3] = 'x';
				ptr[4] = 0;
			}

			if (strlen(g_output[0]) >= 4)
			{
				if (!_strnicmp(&g_output[0][strlen(g_output[0]) - 4], ".ktx", 4))
				{
					g_ktx_only = 1;
					g_ktx = 1;
				}
			}
		}
		else if (!_stricmp(argv[i], "-size"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_size = atoi(argv[i]);
			g_size = max(min(g_size, BN_MAX_IMAGE_SIZE), BN_MIN_IMAGE_SIZE);
		}
		else if (!_stricmp(argv[i], "-iterations"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_iterations = atoi(argv[i]);
		}
		else if (!_stricmp(argv[i], "-percent"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_percent = atof(argv[i]);
			g_percent = max(min(g_percent, BN_MAX_PERCENT), BN_MIN_PERCENT);
		}
		else if (!_stricmp(argv[i], "-filterhalfwidth"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_halfwidth = atoi(argv[i]);
			g_halfwidth = max(min(g_halfwidth, BN_MAX_HALFWIDTH), BN_MIN_HALFWIDTH);
		}
		else if (!_stricmp(argv[i], "-precise"))
		{
			g_precise = 1;
		}
		else if (!_stricmp(argv[i], "-channels"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_channels = atoi(argv[i]);
			g_channels = max(min(g_channels, 4), 1);
		}
		else if (!_stricmp(argv[i], "-seed"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_seed = (uint64_t)_atoi64(argv[i]);
		}
		else if (!_stricmp(argv[i], "-max"))
		{
			g_max = 1;
		}
		else if (!_stricmp(argv[i], "-ktx"))
		{
			g_ktx = 1;
		}
		else if (!_stricmp(argv[i], "-cl_device"))
		{
			i++;
			if (i == argc)
			{
				sprintf_s(g_last_error, 2048, "Missing argument for option \"%s\"", argv[i - 1]);
				return 1;
			}
			g_useopencl = 1;
			g_cl_device_index = atoi(argv[i]);
		}
		else
		{
			sprintf_s(g_last_error, 2048, "Unknown option \"%s\"", argv[i]);
			return 1;
		}
	}
	return 0;
}

void System_PrintUsage()
{
	printf("\n");
	printf("=====\n");
	printf("Usage\n");
	printf("=====\n");
	printf("\n");
	printf("BlueNoise [options]\n");
	printf("\n");
	printf("Example: BlueNoise -size 1024 -percent 0.15 -output \"noise\" -precise\n");
	printf("\n");
	printf("=======\n");
	printf("Options\n");
	printf("=======\n");
	printf("\n");
	printf("-cl_device [integer value]\n");
	printf("\n");
	printf("\tUse OpenCL device [index]. By default, this value is not set. OpenCL devices (if any) are listed at the end of this text.\n");
	printf("\n");
	printf("-size [integer value]\n");
	printf("\n");
	printf("\tSet the size of the noise image to generate [value] x [value]. Silently clamped to the range [%i, %i].\n", BN_MIN_IMAGE_SIZE, BN_MAX_IMAGE_SIZE);
	printf("\t[value] will be rounded to the closest power-of-two that is equal to or less than [value]. Default value is %i.\n", BN_DEFAULT_IMAGE_SIZE);
	printf("\n");
	printf("-iterations [integer value]\n");
	printf("\n");
	printf("\tPerform [value] number of iterations then save output. If this option is set, it overrides the \"percent\" option.\n");
	printf("\n");
	printf("-percent [value]\n");
	printf("\n");
	printf("\tSave output file when less than [value] percent of values have been swapped during an iteration.\n");
	printf("\tSilently clamped to the range [%.2f, %.2f]. Default value is %.2f.\n", BN_MIN_PERCENT, BN_MAX_PERCENT, BN_DEFAULT_PERCENT);
	printf("\n");
	printf("-filterhalfwidth [integer value]\n");
	printf("\n");
	printf("\tSet the half-width of the Gaussian filter used to sample the energy function. Higher values give better quality\n");
	printf("\tresults, but cause lower performance. Silently clamped to the range [%i, %i]. Default value is %i.\n", BN_MIN_HALFWIDTH, BN_MAX_HALFWIDTH, BN_DEFAULT_HALFWIDTH);
	printf("\n");
	printf("-precise\n");
	printf("\n");
	printf("\tUse slow, high-precision exponential function instead of default linear approximation. Default is linear approximation.\n");
	printf("\n");
	printf("-output \"[filename]\"\n");
	printf("\n");
	printf("\tSave output to [filename]. Image type is autodetected from extension. If extension is unsupported or unspecified, file will be saved as PNG.\n");
	printf("\tImage is by default quantised to 8 bits per channel. If filename contains the extension \".ktx\", a 32 bit per channel floating-point\n");
	printf("\tKTX image will be saved instead, according to the \"-ktx\" option detailed below.\n");
	printf("\n");
	printf("-channels [value]\n");
	printf("\n");
	printf("\tGenerate [value] channels of independent blue noise. Silently clamped to the range [1, 4]. Default value is 1.\n");
	printf("\n");
	printf("-seed [integer value]\n");
	printf("\n");
	printf("\tUse [value] as a seed for the random number generator. Default value is 0.\n");
	printf("\n");
	printf("-max\n");
	printf("\n");
	printf("\tGenerate a different type of noise by maximising the energy function instead of minimising it.\n");
	printf("\n");
	printf("-ktx\n");
	printf("\n");
	printf("\tAdditionally save 32-bit float output to [filename].ktx as either GL_RGB32F or GLRGBA32. Resulting KTX image will be GL_RGB32F\n");
	printf("\tif [channels] <= 3, with unused channels containing the value 0, and GL_RGBA32F if [channels] = 4.\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	int i;
	int retval = 0;
	bluenoise_t bn[4];
	HWND consolehwnd = GetConsoleWindow();
	DWORD dwprocessid;
	uint64_t current_seed;
	cl_platform_id platform = 0;
	cl_device_id device = 0;
	cl_context context = 0;
	cl_command_queue queue = 0;
	cl_program program = 0;
	cl_kernel kernel = 0;
	bluenoise_cl_params_t parameters = {0};
	uint64_t timer[2];
	double elapsed;

	if (argc == 1)
	{
		System_PrintUsage();
		printf("==============\n");
		printf("OpenCL devices\n");
		printf("==============\n");
		printf("\n");
		OpenCL_Init(1, 0, &platform, &device);
		printf("\n");
		goto MAIN_EXIT;
	}
	if (System_ParseOptions(argc, argv))
	{
		System_PrintUsage();
		printf("==============\n");
		printf("OpenCL devices\n");
		printf("==============\n");
		printf("\n");
		OpenCL_Init(1, 0, &platform, &device);
		printf("\n");
		retval = -1;
		if (strlen(g_last_error))
		{
			printf("*************************\n");
			printf("Error: %s\n", g_last_error);
			printf("*************************\n");
		}
		goto MAIN_EXIT;
	}
	if (!g_output[0] || !g_output[1])
	{
		System_PrintUsage();
		printf("==============\n");
		printf("OpenCL devices\n");
		printf("==============\n");
		printf("\n");
		OpenCL_Init(1, 0, &platform, &device);
		printf("\n");
		retval = -1;
		printf("****************************\n");
		printf("No output filename specified\n");
		printf("****************************\n");
		goto MAIN_EXIT;
	}

	ilInit();
	iluInit();
	ilutInit();

	ilEnable(IL_FILE_OVERWRITE);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	// always exit when less than 3 pixels are swapped during an iteration, this has most effect on very small image sizes
	if (g_percent < 5.0 * 100.0 / ((double)g_size * (double)g_size))
		g_percent = 5.0 * 100.0 / ((double)g_size * (double)g_size);

	current_seed = g_seed;

	for (i = 0; i < g_channels; i++)
	{
		bn[i] = BlueNoise_Create(g_size, g_halfwidth, current_seed, g_precise, g_max, 0, 0);
		current_seed = RNG_Randomu64(&current_seed, sizeof(uint64_t));
	}

	if (g_useopencl)
	{
		int state = 0;
		for (state = 0; state < 5; state++)
		{
			switch (state)
			{
				case 0:
					if (OpenCL_Init(-1, g_cl_device_index, &platform, &device))
						retval = -1;
					break;
				case 1:
					if (OpenCL_CreateContextQueueDevice(platform, device, &context, &queue))
						retval = -1;
					break;
				case 2:
					if (OpenCL_CompileProgram(device, context, queue, &program, g_bluenoise_cl_source))
						retval = -1;
					break;
				case 3:
					if (OpenCL_CreateKernel(device, program, &kernel))
						retval = -1;
					break;
				case 4:
					if (OpenCL_ComputeParameters(device, context, queue, kernel, &bn[0], &parameters))
						retval = -1;
					break;

			}
			if (retval)
				break;
		}
		if (!retval)
		{
			timer[0] = Timer_GetTicks();
			for (i = 0; i < g_channels; i++)
			{
				retval = OpenCL_PrepareToIterate(&bn[i], queue, kernel, parameters);

				if (retval)
					break;

				if (g_iterations >= 0)
				{
					int iteration;
					int next_print = 0;
					int next_fraction = 1;

					for (iteration = 0; iteration < g_iterations; iteration++)
					{
						double swap_percentage;

						retval = OpenCL_IterateBlueNoise(&bn[i], queue, kernel, parameters, &swap_percentage, 0);

						if (retval)
							break;

						if (iteration == next_print)
						{
							printf("Progress channel %i: %.0f%%...\r", i, 100.0 * iteration / (double)g_iterations);
							while (next_print == iteration)
							{
								next_print = (int)((double)g_iterations * next_fraction / 100.0);
								next_fraction++;
							}
						}
					}
					printf("Progress channel %i: 100%%...\n", i);

					if (retval)
						break;
					retval = OpenCL_FinishIterating(&bn[i], queue, parameters);
				}
				else
				{
					int lower_count = 0;
					uint64_t iteration;
					double moving_average_arr[64];
					int ma_elements = 0;
					int ma_position = 0;
					uint64_t start_time = Timer_GetTicks();

					retval = OpenCL_PrepareToIterate(&bn[i], queue, kernel, parameters);

					if (retval)
						break;

					for (iteration = 0; ; iteration++)
					{
						double curval;

						retval = OpenCL_IterateBlueNoise(&bn[i], queue, kernel, parameters, &curval, 0);

						if (retval)
							break;

						moving_average_arr[ma_position] = curval;

						ma_position = (ma_position + 1) % 64;
						ma_elements = min(ma_elements + 1, 64);

						if (Timer_TicksToSecondsf64(Timer_GetTicks() - start_time) > 0.1)
						{
							int j;
							double ma = 0.0f;
							for (j = 0; j < ma_elements; j++)
								ma += moving_average_arr[(ma_position - ma_elements + j + 64) % 64];
							ma /= ma_elements;
							printf("Progress channel %i: %.0f%%...\r", i, floorf((float)min(g_percent / ma, 99.0)));
							start_time = Timer_GetTicks();
						}

						if (curval*100.0 < g_percent)
							lower_count++;
						else
							lower_count = 0;

						if (lower_count == 20)
							break;
					}
					printf("Progress channel %i: 100%%...\n", i);
					if (retval)
						break;
					retval = OpenCL_FinishIterating(&bn[i], queue, parameters);
				}
			}
			timer[1] = Timer_GetTicks();
		}
		OpenCL_FreeMemory(&parameters);
		OpenCL_Destroy(&context, &queue, &program, &kernel, &parameters);
		if (!retval)
			goto ENCODING_COMPLETE;
	}

	if (retval)
	{
		printf("OpenCL error, reverting to software.\n");
		for (i = 0; i < g_channels; i++)
			BlueNoise_Destroy(&bn[i]);
		for (i = 0; i < g_channels; i++)
		{
			bn[i] = BlueNoise_Create(g_size, g_halfwidth, current_seed, g_precise, g_max, 0, 0);
			current_seed = RNG_Randomu64(&current_seed, sizeof(uint64_t));
		}
	}

	timer[0] = Timer_GetTicks();
	for (i = 0; i < g_channels; i++)
	{
		if (g_iterations >= 0)
		{
			int iteration;
			int next_print = 0;
			int next_fraction = 1;

			for (iteration = 0; iteration < g_iterations; iteration++)
			{
				BlueNoise_Iterate(&bn[i]);

				if (iteration == next_print)
				{
					printf("Progress channel %i: %.0f%%...\r", i, 100.0 * iteration / (double)g_iterations);
					while(next_print == iteration)
					{
						next_print = (int)((double)g_iterations * next_fraction / 100.0);
						next_fraction++;
					}
				}
			}
			printf("Progress channel %i: 100%%...\n", i);
		}
		else
		{
			int lower_count = 0;
			uint64_t iteration;
			double moving_average_arr[64];
			int ma_elements = 0;
			int ma_position = 0;
			uint64_t start_time = Timer_GetTicks();

			for (iteration = 0; ; iteration++)
			{
				double curval = BlueNoise_Iterate(&bn[i]);

				moving_average_arr[ma_position] = curval;

				ma_position = (ma_position + 1) % 64;
				ma_elements = min(ma_elements + 1, 64);

				if (Timer_TicksToSecondsf64(Timer_GetTicks() - start_time) > 0.1)
				{
					int j;
					double ma = 0.0f;
					for (j = 0; j < ma_elements; j++)
						ma += moving_average_arr[(ma_position - ma_elements + j + 64) % 64];
					ma /= ma_elements;
					printf("Progress channel %i: %.0f%%...\r", i, floorf((float)min(g_percent / ma, 99.0)));
					start_time = Timer_GetTicks();
				}

				if (curval*100.0 < g_percent)
					lower_count++;
				else
					lower_count = 0;

				if (lower_count == 20)
					break;
			}
			printf("Progress channel %i: 100%%...\n", i);
		}
	}
	timer[1] = Timer_GetTicks();

ENCODING_COMPLETE:

	elapsed = Timer_TicksToSecondsf64(timer[1] - timer[0]);
	printf("Finished in %.2f %s\n", elapsed < 1.0 ? 1000.0 * elapsed : elapsed, elapsed < 1.0 ? "ms" : "sec");

	if (!g_ktx_only)
	{
		printf("Saving...\n");

		retval = Image_SaveBlueNoise(bn, g_channels, g_output[0], 0);

		if (!retval)
			printf("Saved file \"%s\"\n", g_output[0]);

		if (retval)
		{
			char *local_fname = malloc(strlen(g_output[0]) + 5);

			if (!local_fname)
				return -1;

			strcpy_s(local_fname, strlen(g_output[0]) + 1, g_output[0]);

			if (strrchr(local_fname, '.'))
			{
				char *ptr = strrchr(local_fname, '.');
				ptr[1] = 'p';
				ptr[2] = 'n';
				ptr[3] = 'g';
				ptr[4] = 0;
			}
			else
			{
				char *ptr = &local_fname[strlen(argv[i])];
				ptr[0] = '.';
				ptr[1] = 'p';
				ptr[2] = 'n';
				ptr[3] = 'g';
				ptr[4] = 0;
			}

			printf("****************************\n");
			printf("Error saving file \"%s\", trying \"%s\"\n", g_output[0], local_fname);
			printf("****************************\n");

			retval = Image_SaveBlueNoise(bn, g_channels, local_fname, 0);

			if (retval)
			{
				printf("****************************\n");
				printf("Error saving file \"%s\"\n", local_fname);
				printf("****************************\n");
			}
			else
				printf("Saved file \"%s\"\n", local_fname);

			free(local_fname);
		}
	}

	if (g_ktx)
	{
		int local_retval;

		printf("Saving...\n");

		local_retval = Image_SaveBlueNoise(bn, g_channels, g_output[1], 1);

		if (local_retval)
		{
			printf("****************************\n");
			printf("Error saving file \"%s\"\n", g_output[1]);
			printf("****************************\n");
		}
		else
			printf("Saved file \"%s\"\n", g_output[1]);

		if (local_retval || retval)
			retval = -1;
	}

	for (i = 0; i < g_channels; i++)
		BlueNoise_Destroy(&bn[i]);

MAIN_EXIT:

	GetWindowThreadProcessId(consolehwnd, &dwprocessid);
	if (GetCurrentProcessId() == dwprocessid)
	{
		printf("\n\nPress any key to exit.\n");
		_getch();
	}

	return retval;
}