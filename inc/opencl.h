typedef struct bluenoise_cl_params_s
{
	int			work_group_size;
	int			block_size;
	cl_mem		data_buf_cl[2];
	cl_mem		atomic_val;
	float		*data_in;
	float		*block_out;
}bluenoise_cl_params_t;

int OpenCL_Init(int print_names, int device_index, cl_platform_id *target_platform, cl_device_id *target_device);
int OpenCL_CreateContextQueueDevice(cl_platform_id platform, cl_device_id device, cl_context *context, cl_command_queue *queue);
int OpenCL_CompileProgram(cl_device_id device, cl_context context, cl_command_queue queue, cl_program *program, char *source);
int OpenCL_CreateKernel(cl_device_id device, cl_program program, cl_kernel *kernel);
int OpenCL_ComputeParameters(cl_device_id device, cl_context context, cl_command_queue queue, cl_kernel kernel, bluenoise_t *bn, bluenoise_cl_params_t *parameters);
int OpenCL_IterateBlueNoise(bluenoise_t *bn, cl_command_queue queue, cl_kernel kernel, bluenoise_cl_params_t parameters, double *swap_percentage, int time_run_only);
int OpenCL_PrepareToIterate(bluenoise_t *bn, cl_command_queue queue, cl_kernel kernel, bluenoise_cl_params_t parameters);
int OpenCL_FinishIterating(bluenoise_t *bn, cl_command_queue queue, bluenoise_cl_params_t parameters);
void OpenCL_FreeMemory(bluenoise_cl_params_t *parameters);
void OpenCL_Destroy(cl_context *context, cl_command_queue *queue, cl_program *program, cl_kernel *kernel, bluenoise_cl_params_t *parameters);