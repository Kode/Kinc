#include "pch.h"

#if defined(KORE_WINDOWS) && defined(KORE_VULKAN)
#include <windows.h>
#endif

#include "Image.h"
#include <Kinc/IO/lz4/lz4.h>

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/IO/FileReader.h>
#include <Kinc/Log.h>
#include <Kinc/Math/Core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <string.h>

#include <stdint.h>

uint8_t buffer[4096 * 4096 * 4];

static _Bool endsWith(const char *str, const char *suffix) {
	if (str == NULL || suffix == NULL) return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix > lenstr) return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static bool loadImage(kinc_file_reader_t *file, const char *filename, uint8_t *output, int *outputSize, int *width, int *height,
                      kinc_image_compression_t *compression, kinc_image_format_t *format, unsigned *internalFormat) {
	*format = KINC_IMAGE_FORMAT_RGBA32;
	if (endsWith(filename, "k")) {
		uint8_t data[4];
		kinc_file_reader_read(file, data, 4);
		*width = Kinc_ReadS32LE(data);
		kinc_file_reader_read(file, data, 4);
		*height = Kinc_ReadS32LE(data);

		char fourcc[5];
		kinc_file_reader_read(file, fourcc, 4);
		fourcc[4] = 0;

		int compressedSize = (int)kinc_file_reader_size(file) - 12;

		if (strcmp(fourcc, "LZ4 ") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_NONE;
			*internalFormat = 0;
			*outputSize = *width * *height * 4;
			if (output == NULL) {
				return false;
			}
			kinc_file_reader_read(file, buffer, compressedSize);
			LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, *outputSize);
			return true;
		}
		else if (strcmp(fourcc, "LZ4F") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_NONE;
			*internalFormat = 0;
			*outputSize = *width * *height * 16;
			if (output == NULL) {
				return false;
			}
			kinc_file_reader_read(file, buffer, compressedSize);
			LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, *outputSize);
			*format = KINC_IMAGE_FORMAT_RGBA128;
			return true;
		}
		else if (strcmp(fourcc, "ASTC") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_ASTC;
			*outputSize = *width * *height * 4;
			if (output == NULL) {
				return false;
			}
			kinc_file_reader_read(file, buffer, compressedSize);
			*outputSize = LZ4_decompress_safe((char *)buffer, (char *)output, compressedSize, *outputSize);

			uint8_t blockdim_x = 6;
			uint8_t blockdim_y = 6;
			*internalFormat = (blockdim_x << 8) + blockdim_y;

			return true;

			/*int index = 0;
			index += 4; // magic
			u8 blockdim_x = astcdata[index++];
			u8 blockdim_y = astcdata[index++];
			++index; // blockdim_z
			internalFormat = (blockdim_x << 8) + blockdim_y;
			u8 xsize[4];
			xsize[0] = astcdata[index++];
			xsize[1] = astcdata[index++];
			xsize[2] = astcdata[index++];
			xsize[3] = 0;
			this->width = *(unsigned*)&xsize[0];
			u8 ysize[4];
			ysize[0] = astcdata[index++];
			ysize[1] = astcdata[index++];
			ysize[2] = astcdata[index++];
			ysize[3] = 0;
			this->height = *(unsigned*)&ysize[0];
			u8 zsize[3];
			zsize[0] = astcdata[index++];
			zsize[1] = astcdata[index++];
			zsize[2] = astcdata[index++];
			u8* all = (u8*)astcdata[index];
			dataSize -= 16;
			this->data = new u8[dataSize];
			for (int i = 0; i < dataSize; ++i) {
			data[i] = all[16 + i];
			}
			free(astcdata);*/
		}
		else if (strcmp(fourcc, "DXT5") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_DXT5;
			*outputSize = *width * *height;
			if (output == NULL) {
				return false;
			}
			kinc_file_reader_read(file, buffer, compressedSize);
			*outputSize = LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, *outputSize);
			*internalFormat = 0;
			return true;
		}
		else {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Unknown fourcc in .k file.");
			return false;
		}
	}
	else if (endsWith(filename, "pvr")) {
		uint8_t data[4];
		kinc_file_reader_read(file, data, 4); // version
		kinc_file_reader_read(file, data, 4); // flags
		kinc_file_reader_read(file, data, 4); // pixelFormat1
		kinc_file_reader_read(file, data, 4); // colourSpace
		kinc_file_reader_read(file, data, 4); // channelType
		kinc_file_reader_read(file, data, 4);
		uint32_t hh = Kinc_ReadU32LE(data);
		kinc_file_reader_read(file, data, 4);
		uint32_t ww = Kinc_ReadU32LE(data);
		kinc_file_reader_read(file, data, 4); // depth
		kinc_file_reader_read(file, data, 4); // numSurfaces
		kinc_file_reader_read(file, data, 4); // numFaces
		kinc_file_reader_read(file, data, 4); // mipMapCount
		kinc_file_reader_read(file, data, 4);
		uint32_t metaDataSize = Kinc_ReadU32LE(data);

		kinc_file_reader_read(file, data, 4);
		uint32_t meta1fourcc = Kinc_ReadU32LE(data);
		kinc_file_reader_read(file, data, 4); // meta1key
		kinc_file_reader_read(file, data, 4); // meta1size
		kinc_file_reader_read(file, data, 4);
		uint32_t meta1data = Kinc_ReadU32LE(data);

		kinc_file_reader_read(file, data, 4);
		uint32_t meta2fourcc = Kinc_ReadU32LE(data);
		kinc_file_reader_read(file, data, 4); // meta2key
		kinc_file_reader_read(file, data, 4); // meta2size
		kinc_file_reader_read(file, data, 4);
		uint32_t meta2data = Kinc_ReadU32LE(data);

		int w = 0;
		int h = 0;

		if (meta1fourcc == 0) w = meta1data;
		if (meta1fourcc == 1) h = meta1data;
		if (meta2fourcc == 0) w = meta2data;
		if (meta2fourcc == 1) h = meta2data;

		*width = w;
		*height = h;
		*compression = KINC_IMAGE_COMPRESSION_PVRTC;
		*internalFormat = 0;

		*outputSize = ww * hh / 2;
		kinc_file_reader_read(file, output, *outputSize);
		return true;
	}
	else if (endsWith(filename, "png")) {
		*compression = KINC_IMAGE_COMPRESSION_NONE;
		*internalFormat = 0;
		if (output == NULL) {
			return false;
		}
		int size = (int)kinc_file_reader_size(file);
		kinc_file_reader_read(file, buffer, size);
		int comp;
		uint8_t *uncompressed = stbi_load_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			return false;
		}
		for (int y = 0; y < *height; ++y) {
			for (int x = 0; x < *width; ++x) {
				float r = uncompressed[y * *width * 4 + x * 4 + 0] / 255.0f;
				float g = uncompressed[y * *width * 4 + x * 4 + 1] / 255.0f;
				float b = uncompressed[y * *width * 4 + x * 4 + 2] / 255.0f;
				float a = uncompressed[y * *width * 4 + x * 4 + 3] / 255.0f;
				r *= a;
				g *= a;
				b *= a;
				output[y * *width * 4 + x * 4 + 0] = (uint8_t)Kinc_Round(r * 255.0f);
				output[y * *width * 4 + x * 4 + 1] = (uint8_t)Kinc_Round(g * 255.0f);
				output[y * *width * 4 + x * 4 + 2] = (uint8_t)Kinc_Round(b * 255.0f);
			}
		}
		*outputSize = *width * *height * 4;
		return true;
	}
	else if (endsWith(filename, "hdr")) {
		*compression = KINC_IMAGE_COMPRESSION_NONE;
		*internalFormat = 0;
		if (output == NULL) {
			return false;
		}
		int size = (int)kinc_file_reader_size(file);
		kinc_file_reader_read(file, buffer, size);
		int comp;
		float *uncompressed = stbi_loadf_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			return false;
		}
		*outputSize = *width * *height * 16;
		memcpy(output, uncompressed, *outputSize);
		*format = KINC_IMAGE_FORMAT_RGBA128;
		return true;
	}
	else {
		*compression = KINC_IMAGE_COMPRESSION_NONE;
		*internalFormat = 0;
		if (output == NULL) {
			return false;
		}
		int size = (int)kinc_file_reader_size(file);
		kinc_file_reader_read(file, buffer, size);
		int comp;
		uint8_t *uncompressed = stbi_load_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			kinc_log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			return false;
		}
		*outputSize = *width * *height * 4;
		memcpy(output, uncompressed, *outputSize);
		return true;
	}
}

int kinc_image_format_sizeof(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA32:
	case KINC_IMAGE_FORMAT_BGRA32:
		return 4;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_RGB24:
		return 3;
	}
	return -1;
}

static bool formatIsFloatingPoint(kinc_image_format_t format) {
	return format == KINC_IMAGE_FORMAT_RGBA128 || format == KINC_IMAGE_FORMAT_RGBA64 || format == KINC_IMAGE_FORMAT_A32 || format == KINC_IMAGE_FORMAT_A16;
}

void kinc_image_init(kinc_image_t *image, int width, int height, kinc_image_format_t format, bool readable) {
	kinc_image_init3d(image, width, height, 1, format, readable);
}

void kinc_image_init3d(kinc_image_t *image, int width, int height, int depth, kinc_image_format_t format, bool readable) {
	image->width = width;
	image->height = height;
	image->depth = depth;
	image->format = format;
	image->readable = readable;
	image->compression = KINC_IMAGE_COMPRESSION_NONE;

	if (formatIsFloatingPoint(format)) {
		image->hdrData = (float *)malloc(width * height * depth * kinc_image_format_sizeof(format));
		image->data = NULL;
	}
	else {
		image->data = malloc(width * height * depth * kinc_image_format_sizeof(format));
		image->hdrData = NULL;
	}
}
void kinc_image_init_from_bytes(kinc_image_t *image, void *data, int width, int height, kinc_image_format_t format, bool readable) {
	kinc_image_init_from_bytes3d(image, data, width, height, 1, format, readable);
}

void kinc_image_init_from_bytes3d(kinc_image_t *image, void *data, int width, int height, int depth, kinc_image_format_t format, bool readable) {
	image->compression = KINC_IMAGE_COMPRESSION_NONE;
	if (formatIsFloatingPoint(format)) {
		image->hdrData = (float*)data;
	}
	else {
		image->data = (uint8_t*)data;
	}
}

void kinc_image_init_from_file(kinc_image_t *image, const char *filename, bool readable) {
	uint8_t *imageData = NULL;
	kinc_file_reader_t reader;
	kinc_file_reader_open(&reader, filename, KINC_FILE_TYPE_ASSET);
	int dataSize;
	loadImage(&reader, filename, imageData, &dataSize, &image->width, &image->height, &image->compression, &image->format, &image->internalFormat);
	kinc_file_reader_close(&reader);
	if (formatIsFloatingPoint(image->format)) {
		image->hdrData = (float*)imageData;
		image->data = NULL;
	}
	else {
		image->data = imageData;
		image->hdrData = NULL;
	}
}

void kinc_image_destroy(kinc_image_t *image) {
	if (image->readable) {
		if (formatIsFloatingPoint(image->format)) {
			free(image->hdrData);
			image->hdrData = NULL;
		}
		else {
			free(image->data);
			image->data = NULL;
		}
	}
}

int kinc_image_at(kinc_image_t *image, int x, int y) {
	if (image->data == NULL)
		return 0;
	else
		return *(int *)&((uint8_t *)image->data)[image->width * kinc_image_format_sizeof(image->format) * y + x * kinc_image_format_sizeof(image->format)];
}

uint8_t *kinc_image_get_pixels(kinc_image_t *image) {
	return image->data != NULL ? image->data : (uint8_t*)image->hdrData;
}
