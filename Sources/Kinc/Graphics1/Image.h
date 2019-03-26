#pragma once

typedef enum {
	KINC_IMAGE_COMPRESSION_NONE,
	KINC_IMAGE_COMPRESSION_DXT5,
	KINC_IMAGE_COMPRESSION_ASTC,
	KINC_IMAGE_COMPRESSION_PVRTC
} Kinc_ImageCompression;

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
	u8 *data;
	float *hdrData;
	int dataSize;
	unsigned internalFormat;

} Kinc_Image;

Kinc_Image *Kinc_Image_Create(int width, int height, Kinc_ImageFormat format, bool readable);
Kinc_Image *Kinc_Image_Create3D(int width, int height, int depth, Kinc_ImageFormat format, bool readable);
Kinc_Image *Kinc_Image_CreateFromFile(const char *filename, bool readable);
//Image(Kore::Reader &reader, const char *format, bool readable);
Kinc_Image *Kinc_Image_CreateFromBytes(void *data, int width, int height, Kinc_ImageFormat format, bool readable);
Kinc_Image *Kinc_Image_CreateFromBytes3D(void *data, int width, int height, int depth, Kinc_ImageFormat format, bool readable);
void Kinc_Image_Destroy(Kinc_Image *image);
int Kinc_Image_At(Kinc_Image *image, int x, int y);
u8 *Kinc_Image_GetPixels(Kinc_Image *image);

int Kinc_ImageFormat_SizeOf(Kinc_ImageFormat format);
