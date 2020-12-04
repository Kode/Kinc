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

typedef struct kinc_image_read_callbacks {
	int (*read)(void *user_data, void *data, size_t size);
	void (*seek)(void *user_data, int pos);
	int (*pos)(void *user_data);
	size_t (*size)(void *user_data);
} kinc_image_read_callbacks_t;

KINC_FUNC size_t kinc_image_init(kinc_image_t *image, void *memory, int width, int height, kinc_image_format_t format);
KINC_FUNC size_t kinc_image_init3d(kinc_image_t *image, void *memory, int width, int height, int depth, kinc_image_format_t format);

KINC_FUNC size_t kinc_image_size_from_file(const char *filename);
KINC_FUNC size_t kinc_image_size_from_callbacks(kinc_image_read_callbacks_t callbacks, void *user_data, const char *format);
KINC_FUNC size_t kinc_image_size_from_encoded_bytes(kinc_image_t *image, void *data, size_t data_size, const char *format);

KINC_FUNC size_t kinc_image_init_from_file(kinc_image_t *image, void *memory, const char *filename);
KINC_FUNC size_t kinc_image_init_from_callbacks(kinc_image_t *image, void *memory, kinc_image_read_callbacks_t callbacks, void *user_data, const char *format);
KINC_FUNC size_t kinc_image_init_from_encoded_bytes(kinc_image_t *image, void *memory, void *data, size_t data_size, const char *format);

KINC_FUNC void kinc_image_init_from_bytes(kinc_image_t *image, void *data, int width, int height, kinc_image_format_t format);
KINC_FUNC void kinc_image_init_from_bytes3d(kinc_image_t *image, void *data, int width, int height, int depth, kinc_image_format_t format);
KINC_FUNC void kinc_image_destroy(kinc_image_t *image);
KINC_FUNC int kinc_image_at(kinc_image_t *image, int x, int y);
KINC_FUNC uint8_t *kinc_image_get_pixels(kinc_image_t *image);

KINC_FUNC int kinc_image_format_sizeof(kinc_image_format_t format);

#ifdef __cplusplus
}
#endif
