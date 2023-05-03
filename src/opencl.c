#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define CL_TARGET_OPENCL_VERSION	200
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <cl\cl.h>

#include "..\inc\math.h"
#include "..\inc\bluenoise.h"
#include "..\inc\opencl.h"
#include "..\inc\timer.h"
#include "..\inc\rng.h"
#include "..\inc\system.h"

#define OPENCL_BUILD_OPTS			"-cl-mad-enable -cl-unsafe-math-optimizations -cl-fast-relaxed-math"

char *OpenCL_ErrorString(int error)
{
	switch (error)
	{
	case CL_SUCCESS:									return "CL_SUCCESS";
	case CL_DEVICE_NOT_FOUND:							return "CL_DEVICE_NOT_FOUND";
	case CL_DEVICE_NOT_AVAILABLE:						return "CL_DEVICE_NOT_AVAILABLE";
	case CL_COMPILER_NOT_AVAILABLE:						return "CL_COMPILER_NOT_AVAILABLE";
	case CL_MEM_OBJECT_ALLOCATION_FAILURE:				return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case CL_OUT_OF_RESOURCES:							return "CL_OUT_OF_RESOURCES";
	case CL_OUT_OF_HOST_MEMORY:							return "CL_OUT_OF_HOST_MEMORY";
	case CL_PROFILING_INFO_NOT_AVAILABLE:				return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case CL_MEM_COPY_OVERLAP:							return "CL_MEM_COPY_OVERLAP";
	case CL_IMAGE_FORMAT_MISMATCH:						return "CL_IMAGE_FORMAT_MISMATCH";
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:					return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case CL_BUILD_PROGRAM_FAILURE:						return "CL_BUILD_PROGRAM_FAILURE";
	case CL_MAP_FAILURE:								return "CL_MAP_FAILURE";
	case CL_MISALIGNED_SUB_BUFFER_OFFSET:				return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:	return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case CL_COMPILE_PROGRAM_FAILURE:					return "CL_COMPILE_PROGRAM_FAILURE";
	case CL_LINKER_NOT_AVAILABLE:						return "CL_LINKER_NOT_AVAILABLE";
	case CL_LINK_PROGRAM_FAILURE:						return "CL_LINK_PROGRAM_FAILURE";
	case CL_DEVICE_PARTITION_FAILED:					return "CL_DEVICE_PARTITION_FAILED";
	case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:				return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
	case CL_INVALID_VALUE:								return "CL_INVALID_VALUE";
	case CL_INVALID_DEVICE_TYPE:						return "CL_INVALID_DEVICE_TYPE";
	case CL_INVALID_PLATFORM:							return "CL_INVALID_PLATFORM";
	case CL_INVALID_DEVICE:								return "CL_INVALID_DEVICE";
	case CL_INVALID_CONTEXT:							return "CL_INVALID_CONTEXT";
	case CL_INVALID_QUEUE_PROPERTIES:					return "CL_INVALID_QUEUE_PROPERTIES";
	case CL_INVALID_COMMAND_QUEUE:						return "CL_INVALID_COMMAND_QUEUE";
	case CL_INVALID_HOST_PTR:							return "CL_INVALID_HOST_PTR";
	case CL_INVALID_MEM_OBJECT:							return "CL_INVALID_MEM_OBJECT";
	case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:			return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case CL_INVALID_IMAGE_SIZE:							return "CL_INVALID_IMAGE_SIZE";
	case CL_INVALID_SAMPLER:							return "CL_INVALID_SAMPLER";
	case CL_INVALID_BINARY:								return "CL_INVALID_BINARY";
	case CL_INVALID_BUILD_OPTIONS:						return "CL_INVALID_BUILD_OPTIONS";
	case CL_INVALID_PROGRAM:							return "CL_INVALID_PROGRAM";
	case CL_INVALID_PROGRAM_EXECUTABLE:					return "CL_INVALID_PROGRAM_EXECUTABLE";
	case CL_INVALID_KERNEL_NAME:						return "CL_INVALID_KERNEL_NAME";
	case CL_INVALID_KERNEL_DEFINITION:					return "CL_INVALID_KERNEL_DEFINITION";
	case CL_INVALID_KERNEL:								return "CL_INVALID_KERNEL";
	case CL_INVALID_ARG_INDEX:							return "CL_INVALID_ARG_INDEX";
	case CL_INVALID_ARG_VALUE:							return "CL_INVALID_ARG_VALUE";
	case CL_INVALID_ARG_SIZE:							return "CL_INVALID_ARG_SIZE";
	case CL_INVALID_KERNEL_ARGS:						return "CL_INVALID_KERNEL_ARGS";
	case CL_INVALID_WORK_DIMENSION:						return "CL_INVALID_WORK_DIMENSION";
	case CL_INVALID_WORK_GROUP_SIZE:					return "CL_INVALID_WORK_GROUP_SIZE";
	case CL_INVALID_WORK_ITEM_SIZE:						return "CL_INVALID_WORK_ITEM_SIZE";
	case CL_INVALID_GLOBAL_OFFSET:						return "CL_INVALID_GLOBAL_OFFSET";
	case CL_INVALID_EVENT_WAIT_LIST:					return "CL_INVALID_EVENT_WAIT_LIST";
	case CL_INVALID_EVENT:								return "CL_INVALID_EVENT";
	case CL_INVALID_OPERATION:							return "CL_INVALID_OPERATION";
	case CL_INVALID_GL_OBJECT:							return "CL_INVALID_GL_OBJECT";
	case CL_INVALID_BUFFER_SIZE:						return "CL_INVALID_BUFFER_SIZE";
	case CL_INVALID_MIP_LEVEL:							return "CL_INVALID_MIP_LEVEL";
	case CL_INVALID_GLOBAL_WORK_SIZE:					return "CL_INVALID_GLOBAL_WORK_SIZE";
	case CL_INVALID_PROPERTY:							return "CL_INVALID_PROPERTY";
	case CL_INVALID_IMAGE_DESCRIPTOR:					return "CL_INVALID_IMAGE_DESCRIPTOR";
	case CL_INVALID_COMPILER_OPTIONS:					return "CL_INVALID_COMPILER_OPTIONS";
	case CL_INVALID_LINKER_OPTIONS:						return "CL_INVALID_LINKER_OPTIONS";
	case CL_INVALID_DEVICE_PARTITION_COUNT:				return "CL_INVALID_DEVICE_PARTITION_COUNT";
	case CL_INVALID_PIPE_SIZE:							return "CL_INVALID_PIPE_SIZE";
	case CL_INVALID_DEVICE_QUEUE:						return "CL_INVALID_DEVICE_QUEUE";
#ifdef CL_VERSION_2_2
	case CL_INVALID_SPEC_ID:							return "CL_INVALID_SPEC_ID";
	case CL_MAX_SIZE_RESTRICTION_EXCEEDED:				return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif
	default:											return "<UNKNOWN_ERROR>";
	}
}

int OpenCL_Init(int print_names, int device_index, cl_platform_id *target_platform, cl_device_id *target_device)
{
	int i;
	int j;
	cl_int err;
	cl_platform_id *cpPlatform;
	cl_uint num_platforms;
	cl_device_id *device_id_combined = 0;
	cl_platform_id *device_platform_combined = 0;
	int num_device_ids_total = 0;
	char **name_list = 0;

	err = clGetPlatformIDs(0, 0, &num_platforms);

	if (!(num_platforms && err == CL_SUCCESS))
		return -1;

	cpPlatform = malloc(sizeof(cl_platform_id) * num_platforms);

	err = clGetPlatformIDs(num_platforms, cpPlatform, NULL);

	for (i = 0; i < (int)num_platforms; i++)
	{
		cl_uint num_devices;
		void *temp_ptr;
		cl_device_id *device_id = 0;

		err = clGetDeviceIDs(cpPlatform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

		if (err)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}

		device_id = malloc(sizeof(cl_device_id) * num_devices);

		if (!device_id)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}

		err = clGetDeviceIDs(cpPlatform[i], CL_DEVICE_TYPE_ALL, num_devices, device_id, NULL);

		if (err)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}

		temp_ptr = realloc(device_id_combined, sizeof(cl_device_id) * (num_device_ids_total + num_devices));

		if (!temp_ptr)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}
		device_id_combined = temp_ptr;

		temp_ptr = realloc(device_platform_combined, sizeof(cl_platform_id) * (num_device_ids_total + num_devices));

		if (!temp_ptr)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}
		device_platform_combined = temp_ptr;

		temp_ptr = realloc(name_list, sizeof(char*) * (num_device_ids_total + num_devices));

		if (!temp_ptr)
		{
			err = CL_OUT_OF_RESOURCES;
			free(device_id);
			goto INIT_EXIT;
		}
		name_list = temp_ptr;

		for (j = 0; j < (int)num_devices; j++)
		{
			size_t param_value_size_ret;
			uint64_t mem = 0;
			char *name_ptr;

			cl_device_info device_info_id[] = 
			{
				CL_DEVICE_NAME
			};

			err = clGetDeviceInfo(device_id[j], device_info_id[0], 0, NULL, &param_value_size_ret);
			if (err)
			{
				err = CL_OUT_OF_RESOURCES;
				free(device_id);
				goto INIT_EXIT;
			}
			name_ptr = malloc(param_value_size_ret + 1);
			if (!name_ptr)
			{
				err = CL_OUT_OF_RESOURCES;
				free(device_id);
				goto INIT_EXIT;
			}
			err = clGetDeviceInfo(device_id[j], device_info_id[0], param_value_size_ret, name_ptr, NULL);
			if (err)
			{
				err = CL_OUT_OF_RESOURCES;
				free(name_ptr);
				free(device_id);
				goto INIT_EXIT;
			}
			name_ptr[param_value_size_ret] = 0;

			device_id_combined[num_device_ids_total] = device_id[j];
			device_platform_combined[num_device_ids_total] = cpPlatform[i];
			name_list[num_device_ids_total] = name_ptr;
			num_device_ids_total++;
		}

		free(device_id);
	}

INIT_EXIT:

	free(cpPlatform);

	if (err != CL_SUCCESS)
	{
		for (i = 0; i < num_device_ids_total; i++)
			free(name_list[i]);
		free(name_list);
		free(device_id_combined);
		free(device_platform_combined);
		return -1;
	}

	if (print_names > 0)
	{
		for (i = 0; i < num_device_ids_total; i++)
			printf("\tDevice %i: \"%s\"\n", i, name_list[i]);
	}

	if (target_device && target_platform)
	{
		if ((device_index < num_device_ids_total) && (device_index >= 0))
		{
			*target_device = device_id_combined[device_index];
			*target_platform = device_platform_combined[device_index];
			if (print_names < 0)
				printf("Using CL device \"%s\"\n", name_list[device_index]);
		}
		else
		{
			*target_device = 0;
			*target_platform = 0;
			err = CL_DEVICE_NOT_FOUND;
			DEBUG_PRINT("CL device index %i not found.\n", device_index);
		}
	}

	for (i = 0; i < num_device_ids_total; i++)
		free(name_list[i]);
	free(name_list);
	free(device_id_combined);
	free(device_platform_combined);
	
	return err;
}

int OpenCL_CreateContextQueueDevice(cl_platform_id platform, cl_device_id device, cl_context *context, cl_command_queue *queue)
{
	cl_int err;
	cl_context_properties prop[3];

	if (!(context && queue))
		return -1;
	if ((platform == 0) || (device == 0))
		return -1;

	*context = 0;
	*queue = 0;

	prop[0] = CL_CONTEXT_PLATFORM;
	prop[1] = (cl_context_properties)platform;
	prop[2] = 0;

	*context = clCreateContext(&prop[0], 1, &device, NULL, NULL, &err);
	if (err) 
	{
		DEBUG_PRINT("clCreateContext result: %i (%s)\n", err, OpenCL_ErrorString(err));
		return -1;
	}

	*queue = clCreateCommandQueueWithProperties(*context, device, 0, &err);
	if (err) 
	{
		DEBUG_PRINT("clCreateCommandQueueWithProperties result: %i (%s)\n", err, OpenCL_ErrorString(err));
		*queue = clCreateCommandQueue(*context, device, 0, &err);
		if (err) 
		{
			DEBUG_PRINT("clCreateCommandQueue result: %i (%s)\n", err, OpenCL_ErrorString(err));
			clReleaseContext(*context);
			*context = 0;
			*queue = 0;
			return -1;
		}
	}
	return 0;
}

int OpenCL_CompileProgram(cl_device_id device, cl_context context, cl_command_queue queue, cl_program *program, char *source)
{
	cl_int err;
	char *buildlog = 0;
	size_t blen = 0;
	int error = -1;

	*program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
	if (err != CL_SUCCESS)
	{
		DEBUG_PRINT("clCreateProgramWithSource result: %i (%s)\n", err, OpenCL_ErrorString(err));
		goto FINAL_STAGE;
	}

	err = clBuildProgram(*program, 0, NULL, OPENCL_BUILD_OPTS, NULL, NULL);
	if (err)
	{
		DEBUG_PRINT("clBuildProgram result: %i (%s)\n", err, OpenCL_ErrorString(err));
		err = clBuildProgram(*program, 0, NULL, NULL, NULL, NULL);
		if (err)
		{
			DEBUG_PRINT("clBuildProgram result: %i (%s)\n", err, OpenCL_ErrorString(err));
		}
	}

	if (err)
	{
		err = clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, 0, 0, &blen);
		if (err != CL_SUCCESS)
			goto FINAL_STAGE;
		DEBUG_PRINT("clGetProgramBuildInfo result: %i (%s)\n", err, OpenCL_ErrorString(err));
		buildlog = malloc(blen + 1);
		err = clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, blen, buildlog, 0);
		if (err != CL_SUCCESS)
			goto FINAL_STAGE;
		DEBUG_PRINT("clGetProgramBuildInfo result: %i (%s)\n", err, OpenCL_ErrorString(err));
		if (blen > 2)
			DEBUG_PRINT("Build log:\n==========\nLength: %i\n\n%s\n", (int)blen, buildlog);
		goto FINAL_STAGE;
	}

	error = 0;

FINAL_STAGE:

	free(buildlog);

	return error;
}

int OpenCL_CreateKernel(cl_device_id device, cl_program program, cl_kernel *kernel)
{
	cl_int err = CL_SUCCESS;

	*kernel = clCreateKernel(program, "BlueNoise_Iterate", &err);

	if (err != CL_SUCCESS)
	{
		DEBUG_PRINT("clCreateKernel result: %i (%s), kernel ID %I64u, function \"%s\"\n", err, OpenCL_ErrorString(err), (uint64_t)(*kernel), "BlueNoise_Iterate");
		goto FINAL_STAGE;
	}

FINAL_STAGE:
	if (err != CL_SUCCESS)
		return -1;

	return 0;
}

int OpenCL_FinishIterating(bluenoise_t *bn, cl_command_queue queue, bluenoise_cl_params_t parameters)
{
	cl_int err = CL_SUCCESS;
	
	err = clEnqueueReadBuffer(queue, parameters.data_buf_cl[bn->iteration & 1], CL_TRUE, 0, bn->width * bn->width * sizeof(float), bn->data[bn->iteration & 1], 0, NULL, NULL);
	
	if (err)
		DEBUG_PRINT("clEnqueueReadBuffer result on %I64i bytes: %i (%s)\n", bn->width * bn->width * sizeof(float), err, OpenCL_ErrorString(err));
	if (err)
		goto ENCODE_CLEANUP;
	clFinish(queue);

ENCODE_CLEANUP:

	return err;
}

int OpenCL_PrepareToIterate(bluenoise_t *bn, cl_command_queue queue, cl_kernel kernel, bluenoise_cl_params_t parameters)
{
	cl_int err = CL_SUCCESS;

	unsigned int nu32[] = 
	{
		bn->maximise_energy,
		bn->precise,
		bn->gaussian_halfwidth,
		bn->width,
		parameters.block_size,
	};

	err = clEnqueueWriteBuffer(queue, parameters.data_buf_cl[0], CL_TRUE, 0, bn->width * bn->width * sizeof(float), bn->data[0], 0, NULL, NULL);
	if (err)
	{
		DEBUG_PRINT("clEnqueueWriteBuffer result on %I64i bytes: %i (%s)\n", bn->width * bn->width * sizeof(float), err, OpenCL_ErrorString(err));
		return -1;
	}
	err = clEnqueueWriteBuffer(queue, parameters.data_buf_cl[1], CL_TRUE, 0, bn->width * bn->width * sizeof(float), bn->data[0], 0, NULL, NULL);
	if (err)
	{
		DEBUG_PRINT("clEnqueueWriteBuffer result on %I64i bytes: %i (%s)\n", bn->width * bn->width * sizeof(float), err, OpenCL_ErrorString(err));
		return -1;
	}

	clSetKernelArg(kernel, 2, sizeof(uint64_t), &bn->seed);
	clSetKernelArg(kernel, 3, sizeof(unsigned int), &nu32[0]);
	clSetKernelArg(kernel, 4, sizeof(unsigned int), &nu32[1]);
	clSetKernelArg(kernel, 5, sizeof(unsigned int), &nu32[2]);
	clSetKernelArg(kernel, 6, sizeof(unsigned int), &nu32[3]);
	clSetKernelArg(kernel, 7, sizeof(unsigned int), &nu32[4]);
	clSetKernelArg(kernel, 10, sizeof(cl_mem), &parameters.atomic_val);
	clSetKernelArg(kernel, 11, sizeof(cl_mem), &parameters.data_buf_cl[0]);
	clSetKernelArg(kernel, 12, sizeof(cl_mem), &parameters.data_buf_cl[1]);

	return 0;
}

int OpenCL_IterateBlueNoise(bluenoise_t *bn, cl_command_queue queue, cl_kernel kernel, bluenoise_cl_params_t parameters, double *swap_percentage, int time_run_only)
{
	size_t local[2];
	size_t global[2];
	uint64_t hash_buffer[2];
	cl_int err = CL_SUCCESS;
	uint64_t hash;
	int blocks_x;
	int blocks_y;
	int x;
	int y;
	uint32_t swaps = 0;
	uint32_t src_index = bn->iteration & 1;
	uint32_t dst_index = (bn->iteration + 1) & 1;

	/*
	// Randomly Break for testing purposes
	if (!(rand()%64))
	{
		dst_index = 2;
		parameters.data_buf_cl[0] = 0;
		clSetKernelArg(kernel, 10, sizeof(cl_mem), &parameters.data_buf_cl[0]);
	}
	*/

	parameters.block_size;

	blocks_x = bn->width / parameters.block_size;
	blocks_y = bn->width / parameters.block_size;

	local[0] = parameters.block_size < parameters.work_group_size ? parameters.block_size : parameters.work_group_size;
	local[1] = parameters.block_size < parameters.work_group_size ? parameters.block_size : parameters.work_group_size;

	global[0] = parameters.block_size;
	global[1] = parameters.block_size;

	hash_buffer[0] = bn->seed;
	hash_buffer[1] = bn->iteration;

	hash = RNG_Randomu64(hash_buffer, sizeof(uint64_t) * 2);

	clSetKernelArg(kernel, 0, sizeof(uint64_t), &hash);
	clSetKernelArg(kernel, 1, sizeof(uint64_t), &bn->iteration);
	clSetKernelArg(kernel, 13, sizeof(unsigned int), &src_index);
	clSetKernelArg(kernel, 14, sizeof(unsigned int), &dst_index);

	err = clEnqueueWriteBuffer(queue, parameters.atomic_val, CL_TRUE, 0, sizeof(uint32_t), &swaps, 0, NULL, NULL);
	if (err)
		DEBUG_PRINT("clEnqueueWriteBuffer result on %I64i bytes: %i (%s)\n", bn->width * bn->width * sizeof(float), err, OpenCL_ErrorString(err));
	if (err)
		goto ENCODE_CLEANUP;
	clFinish(queue);

	for (y = 0; y < blocks_y; y++)
	{
		for (x = 0; x < blocks_x; x++)
		{
			clSetKernelArg(kernel, 8, sizeof(unsigned int), &x);
			clSetKernelArg(kernel, 9, sizeof(unsigned int), &y);

			err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
			if (err)
				DEBUG_PRINT("clEnqueueNDRangeKernel result: %i (%s)\n", err, OpenCL_ErrorString(err));
			if (err)
				goto ENCODE_CLEANUP;
		}
	}
	err = clFinish(queue);
	if (err)
		DEBUG_PRINT("clFinish result: %i (%s)\n", err, OpenCL_ErrorString(err));

	err = clEnqueueReadBuffer(queue, parameters.atomic_val, CL_TRUE, 0, sizeof(uint32_t), &swaps, 0, NULL, NULL);
	if (err)
		DEBUG_PRINT("clEnqueueReadBuffer result on %I64i bytes: %i (%s)\n", parameters.block_size * parameters.block_size * sizeof(float), err, OpenCL_ErrorString(err));
	if (err)
		goto ENCODE_CLEANUP;
	clFinish(queue);

	if (!time_run_only)
		bn->iteration++;

	if (swap_percentage)
		*swap_percentage = swaps / ((double)bn->width * bn->width);

ENCODE_CLEANUP:
	return err;
}

void OpenCL_Destroy(cl_context *context, cl_command_queue *queue, cl_program *program, cl_kernel *kernel, bluenoise_cl_params_t *parameters)
{
	clReleaseMemObject(parameters->data_buf_cl[0]);
	clReleaseMemObject(parameters->data_buf_cl[1]);
	clReleaseMemObject(parameters->atomic_val);
	clReleaseKernel(*kernel);
	clReleaseProgram(*program);
	clReleaseCommandQueue(*queue);
	clReleaseContext(*context);

	*context = 0;
	*queue = 0;
	*program = 0;
	*kernel = 0;

	memset(parameters, 0, sizeof(bluenoise_cl_params_t));
}

void OpenCL_FreeMemory(bluenoise_cl_params_t *parameters)
{
	clReleaseMemObject(parameters->data_buf_cl[0]);
	clReleaseMemObject(parameters->data_buf_cl[1]);
	clReleaseMemObject(parameters->atomic_val);

	parameters->data_buf_cl[0] = 0;
	parameters->data_buf_cl[1] = 0;
	parameters->atomic_val = 0;
}

int OpenCL_ComputeParameters(
	cl_device_id device, 
	cl_context context, 
	cl_command_queue queue, 
	cl_kernel kernel, 
	bluenoise_t *bn, 
	bluenoise_cl_params_t *parameters)
{
	cl_int err = CL_SUCCESS;
	int block_size;
	unsigned int group_size;
	size_t work_group_size = 0;
	bluenoise_t bn_local = *bn;

	memset(parameters, 0, sizeof(bluenoise_cl_params_t));

	clGetKernelWorkGroupInfo(kernel, device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &work_group_size, NULL);

	group_size = (unsigned int)Math_FloorPow2u64(work_group_size);

	while(((size_t)(group_size)) * ((size_t)(group_size)) > work_group_size)
		group_size >>= 1;

	parameters->work_group_size = group_size;

	block_size = min(parameters->work_group_size, (int)bn->width);

	parameters->atomic_val = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t), NULL, &err);
	if (err)
	{
		DEBUG_PRINT("clCreateBuffer result: %i (%s)\n", err, OpenCL_ErrorString(err));
		return -1;
	}

	parameters->data_buf_cl[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, bn->width * bn->width * sizeof(float), NULL, &err);
	if (err)
	{
		DEBUG_PRINT("clCreateBuffer result: %i (%s)\n", err, OpenCL_ErrorString(err));
		return -1;
	}
	parameters->data_buf_cl[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, bn->width * bn->width * sizeof(float), NULL, &err);
	if (err)
	{
		DEBUG_PRINT("clCreateBuffer result: %i (%s)\n", err, OpenCL_ErrorString(err));
		return -1;
	}

	while (block_size <= (int)bn->width)
	{
		uint64_t timer[2];

		bn_local.width = block_size;

		parameters->block_size = block_size;

		if (OpenCL_PrepareToIterate(bn, queue, kernel, *parameters))
		{
			OpenCL_FreeMemory(parameters);
			return -1;
		}
		timer[0] = Timer_GetTicks();
		double percent;
		err = OpenCL_IterateBlueNoise(&bn_local, queue, kernel, *parameters, &percent, 1);
		timer[1] = Timer_GetTicks();

		if (err)
		{
			DEBUG_PRINT("OpenCL_IterateBlueNoise result: %i (%s)\n", err, OpenCL_ErrorString(err));
			OpenCL_FreeMemory(parameters);
			return -1;
		}

		if (Timer_TicksToSecondsf64(timer[1] - timer[0]) > 0.25)
			break;

		block_size <<= 1;
	}

	if (block_size > (int)bn->width)
		block_size = (int)bn->width;

	parameters->block_size = block_size;

	return 0;
}