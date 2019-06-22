#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_image_compression {
	KINC_IMAGE_COMPRESSION_NONE,
	KINC_IMAGE_COMPRESSION_DXT5,
	KINC_IMAGE_COMPRESSION_ASTC,
	KINC_IMAGE_COMPRESSION_PVRTC
} kinc_image_compression_t;

typedef enum kinc_image_format {
	KINC_IMAGE_FORMAT_RGBA32,
	KINC_IMAGE_FORMAT_GREY8,
	KINC_IMAGE_FORMAT_RGB24,
	KINC_IMAGE_FORMAT_RGBA128,
	KINC_IMAGE_FORMAT_RGBA64,
	KINC_IMAGE_FORMAT_A32,
	KINC_IMAGE_FORMAT_BGRA32,
	KINC_IMAGE_FORMAT_A16
} kinc_image_format_t;

typedef struct kinc_image {
	int width, height, depth;
	kinc_image_format_t format;
	unsigned internal_format;
	kinc_image_compression_t compression;
	void *data;
	int data_size;
} kinc_image_t;

size_t kinc_image_init(kinc_image_t *image, void *memory, int width, int height, kinc_image_format_t format);
size_t kinc_image_init3d(kinc_image_t *image, void *memory, int width, int height, int depth, kinc_image_format_t format);
size_t kinc_image_init_from_file(kinc_image_t *image, void *memory, const char *filename);
void kinc_image_init_from_bytes(kinc_image_t *image, void *data, int width, int height, kinc_image_format_t format);
void kinc_image_init_from_bytes3d(kinc_image_t *image, void *data, int width, int height, int depth, kinc_image_format_t format);
void kinc_image_destroy(kinc_image_t *image);
int kinc_image_at(kinc_image_t *image, int x, int y);
uint8_t *kinc_image_get_pixels(kinc_image_t *image);

int kinc_image_format_sizeof(kinc_image_format_t format);

#ifdef __cplusplus
}
#endif
