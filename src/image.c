#include <windows.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "..\inc\bluenoise.h"
#include "..\inc\system.h"

#include "..\inc\ktx\ktx.h"

#include "..\inc\IL\il.h"
#include "..\inc\IL\ilu.h"
#include "..\inc\IL\ilut.h"

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <gl\wglext.h>

static char *KTXErrToString(KTX_error_code err)
{
	switch(err)
	{
		case KTX_SUCCESS:					return "KTX_SUCCESS";
		case KTX_FILE_DATA_ERROR:			return "KTX_FILE_DATA_ERROR";
		case KTX_FILE_ISPIPE:				return "KTX_FILE_ISPIPE";
		case KTX_FILE_OPEN_FAILED:			return "KTX_FILE_OPEN_FAILED";
		case KTX_FILE_OVERFLOW:				return "KTX_FILE_OVERFLOW";
		case KTX_FILE_READ_ERROR:			return "KTX_FILE_READ_ERROR";
		case KTX_FILE_SEEK_ERROR:			return "KTX_FILE_SEEK_ERROR";
		case KTX_FILE_UNEXPECTED_EOF:		return "KTX_FILE_UNEXPECTED_EOF";
		case KTX_FILE_WRITE_ERROR:			return "KTX_FILE_WRITE_ERROR";
		case KTX_GL_ERROR:					return "KTX_GL_ERROR";
		case KTX_INVALID_OPERATION:			return "KTX_INVALID_OPERATION";
		case KTX_INVALID_VALUE:				return "KTX_INVALID_VALUE";
		case KTX_NOT_FOUND:					return "KTX_NOT_FOUND";
		case KTX_OUT_OF_MEMORY:				return "KTX_OUT_OF_MEMORY";
		case KTX_TRANSCODE_FAILED:			return "KTX_TRANSCODE_FAILED";
		case KTX_UNKNOWN_FILE_FORMAT:		return "KTX_UNKNOWN_FILE_FORMAT";
		case KTX_UNSUPPORTED_TEXTURE_TYPE:	return "KTX_UNSUPPORTED_TEXTURE_TYPE";
		case KTX_UNSUPPORTED_FEATURE:		return "KTX_UNSUPPORTED_FEATURE";
		case KTX_LIBRARY_NOT_LINKED:		return "KTX_LIBRARY_NOT_LINKED";
		default:							return "<UNKNOWN>";
	}
}


static int Image_SaveBlueNoiseKTX(bluenoise_t *bn, int num_channels, char *filename)
{
	ktxTexture1* texture;
	ktxTextureCreateInfo createInfo = {0};
	KTX_error_code result;
	ktx_uint32_t level, layer, faceSlice;
	int i;
	int j;
	int dst_channels = num_channels < 4 ? 3 : 4;
	float *temp_data = malloc((size_t)bn[0].width * (size_t)bn[0].width * sizeof(float) * dst_channels);

	if (!temp_data)
		return -1;

	createInfo.glInternalformat = num_channels < 4 ? GL_RGB32F : GL_RGBA32F;
	createInfo.baseWidth = bn[0].width;
	createInfo.baseHeight = bn[0].width;
	createInfo.baseDepth = 1;
	createInfo.numDimensions = 2;
	createInfo.numLevels = 1;
	createInfo.numLayers = 1;
	createInfo.numFaces = 1;
	createInfo.isArray = KTX_FALSE;
	createInfo.generateMipmaps = KTX_FALSE;
	result = ktxTexture1_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
	if (result != KTX_SUCCESS)
		DEBUG_PRINT("ktxTexture1_Create() result: %i (%s)\n", (int)result, KTXErrToString(result));

	if (result != KTX_SUCCESS)
	{
		free(temp_data);
		return -1;
	}


	level = 0;
	layer = 0;
	faceSlice = 0;

	for (i = 0; i < (int)bn[0].width * (int)bn[0].width; i++)
	{
		for (j = 0; j < num_channels; j++)
		{
			float valf = bn[j].data[bn[j].iteration & 1][i];

			valf = min(max(valf, 0.0f), 1.0f);

			temp_data[i*dst_channels + j] = valf;
		}
		for (j = num_channels; j < 3; j++)
			temp_data[i*dst_channels + j] = 0.0f;
	}

	result = ktxTexture_SetImageFromMemory((ktxTexture*)texture, 0, layer, faceSlice, (const ktx_uint8_t*)temp_data, (size_t)bn[0].width * (size_t)bn[0].width * sizeof(float) * dst_channels);
	if (result != KTX_SUCCESS)
		DEBUG_PRINT("ktxTexture_SetImageFromMemory() result: %i (%s)\n", (int)result, KTXErrToString(result));

	free(temp_data);

	if (result != KTX_SUCCESS)
	{
		ktxTexture_Destroy((ktxTexture*)texture);
		return -1;
	}

	result = ktxTexture_WriteToNamedFile((ktxTexture*)texture, filename);
	if (result != KTX_SUCCESS)
		DEBUG_PRINT("ktxTexture_WriteToNamedFile() result: %i (%s)\n", (int)result, KTXErrToString(result));

	if (result != KTX_SUCCESS)
	{
		ktxTexture_Destroy((ktxTexture*)texture);
		return -1;
	}
	ktxTexture_Destroy((ktxTexture*)texture);
	return 0;
}
static int Image_SaveBlueNoiseDevIL(bluenoise_t *bn, int num_channels, char *filename)
{
	int result;
	ILuint image;
	int i;
	int j;
	int dst_channels = num_channels < 4 ? 3 : 4;
	uint8_t *temp_data = malloc((size_t)bn[0].width * (size_t)bn[0].width * sizeof(uint8_t) * dst_channels);
	wchar_t *filename_w = malloc((strlen(filename) + 1) * sizeof(wchar_t));

	if (!temp_data)
		return -1;

	if (!filename_w)
	{
		free(temp_data);
		return -1;
	}

	swprintf(filename_w, strlen(filename) + 1, L"%S", filename);

	ilGenImages(1, &image);
	ilBindImage(image);

	for (i = 0; i < (int)bn[0].width * (int)bn[0].width; i++)
	{
		for (j = 0; j < num_channels; j++)
		{
			float valf = bn[j].data[bn[j].iteration & 1][i] * 255.0f;
			uint8_t val;

			valf = min(max(valf, 0.0f), 255.0f);
			val = (uint8_t)valf;

			temp_data[i*dst_channels + j] = val;
		}
		for (j = num_channels; j < 3; j++)
			temp_data[i*dst_channels + j] = 0;
	}

	result = ilTexImage(bn[0].width, bn[0].width, 1, dst_channels, dst_channels == 3 ? IL_RGB : IL_RGBA, IL_UNSIGNED_BYTE, temp_data);

	if (result == FALSE)
	{
		ilBindImage(0);
		ilDeleteImage(image);
		free(filename_w);
		free(temp_data);
		return -1;
	}

	result = ilSaveImage(filename_w);

	if (result == FALSE)
	{
		ilBindImage(0);
		ilDeleteImage(image);
		free(filename_w);
		free(temp_data);
		return -1;
	}

	ilBindImage(0);
	ilDeleteImage(image);

	free(filename_w);
	free(temp_data);

	return 0;
}

int Image_SaveBlueNoise(bluenoise_t *bn, int num_channels, char *filename, int is_ktx)
{
	if (is_ktx)
		return Image_SaveBlueNoiseKTX(bn, num_channels, filename);
	else
		return Image_SaveBlueNoiseDevIL(bn, num_channels, filename);
}