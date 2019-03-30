#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { KINC_IMAGE_COMPRESSION_NONE, KINC_IMAGE_COMPRESSION_DXT5, KINC_IMAGE_COMPRESSION_ASTC, KINC_IMAGE_COMPRESSION_PVRTC } Kinc_ImageCompression;

typedef enum {
	KINC_IMAGE_FORMAT_RGBA32,
	KINC_IMAGE_FORMAT_GREY8,
	KINC_IMAGE_FORMAT_RGB24,
	KINC_IMAGE_FORMAT_RGBA128,
	KINC_IMAGE_FORMAT_RGBA64,
	KINC_IMAGE_FORMAT_A32,
	KINC_IMAGE_FORMAT_BGRA32,
	KINC_IMAGE_FORMAT_A16
} Kinc_ImageFormat;

typedef struct {
	int width, height, depth;
	Kinc_ImageFormat format;
	bool readable;
	Kinc_ImageCompression compression;
	uint8_t *data;
	float *hdrData;
	int dataSize;
	unsigned internalFormat;
} Kinc_Image;

void Kinc_Image_Create(Kinc_Image *image, int width, int height, Kinc_ImageFormat format, bool readable);
void Kinc_Image_Create3D(Kinc_Image *image, int width, int height, int depth, Kinc_ImageFormat format, bool readable);
void Kinc_Image_CreateFromFile(Kinc_Image *image, const char *filename, bool readable);
void Kinc_Image_CreateFromBytes(Kinc_Image *image, void *data, int width, int height, Kinc_ImageFormat format, bool readable);
void Kinc_Image_CreateFromBytes3D(Kinc_Image *image, void *data, int width, int height, int depth, Kinc_ImageFormat format, bool readable);
void Kinc_Image_Destroy(Kinc_Image *image);
int Kinc_Image_At(Kinc_Image *image, int x, int y);
uint8_t *Kinc_Image_GetPixels(Kinc_Image *image);

int Kinc_ImageFormat_SizeOf(Kinc_ImageFormat format);

#ifdef __cplusplus
}
#endif
