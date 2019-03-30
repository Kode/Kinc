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

static bool loadImage(Kinc_FileReader *file, const char *filename, uint8_t *output, int *outputSize, int *width, int *height,
                      Kinc_ImageCompression *compression, Kinc_ImageFormat *format, unsigned *internalFormat) {
	*format = KINC_IMAGE_FORMAT_RGBA32;
	if (endsWith(filename, "k")) {
		uint8_t data[4];
		Kinc_FileReader_Read(file, data, 4);
		*width = Kinc_ReadS32LE(data);
		Kinc_FileReader_Read(file, data, 4);
		*height = Kinc_ReadS32LE(data);

		char fourcc[5];
		Kinc_FileReader_Read(file, fourcc, 4);
		fourcc[4] = 0;

		int compressedSize = Kinc_FileReader_Size(file) - 12;

		if (strcmp(fourcc, "LZ4 ") == 0) {
			*compression = KINC_IMAGE_COMPRESSION_NONE;
			*internalFormat = 0;
			*outputSize = *width * *height * 4;
			if (output == NULL) {
				return false;
			}
			Kinc_FileReader_Read(file, buffer, compressedSize);
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
			Kinc_FileReader_Read(file, buffer, compressedSize);
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
			Kinc_FileReader_Read(file, buffer, compressedSize);
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
			Kinc_FileReader_Read(file, buffer, compressedSize);
			*outputSize = LZ4_decompress_safe((char *)(data + 12), (char *)output, compressedSize, *outputSize);
			*internalFormat = 0;
			return true;
		}
		else {
			Kinc_Log(KINC_LOG_LEVEL_ERROR, "Unknown fourcc in .k file.");
			return false;
		}
	}
	else if (endsWith(filename, "pvr")) {
		uint8_t data[4];
		Kinc_FileReader_Read(file, data, 4); // version
		Kinc_FileReader_Read(file, data, 4); // flags
		Kinc_FileReader_Read(file, data, 4); // pixelFormat1
		Kinc_FileReader_Read(file, data, 4); // colourSpace
		Kinc_FileReader_Read(file, data, 4); // channelType
		Kinc_FileReader_Read(file, data, 4);
		uint32_t hh = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4);
		uint32_t ww = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // depth
		Kinc_FileReader_Read(file, data, 4); // numSurfaces
		Kinc_FileReader_Read(file, data, 4); // numFaces
		Kinc_FileReader_Read(file, data, 4); // mipMapCount
		Kinc_FileReader_Read(file, data, 4);
		uint32_t metaDataSize = Kinc_ReadU32LE(data);

		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta1fourcc = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // meta1key
		Kinc_FileReader_Read(file, data, 4); // meta1size
		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta1data = Kinc_ReadU32LE(data);

		Kinc_FileReader_Read(file, data, 4);
		uint32_t meta2fourcc = Kinc_ReadU32LE(data);
		Kinc_FileReader_Read(file, data, 4); // meta2key
		Kinc_FileReader_Read(file, data, 4); // meta2size
		Kinc_FileReader_Read(file, data, 4);
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
		Kinc_FileReader_Read(file, output, *outputSize);
		return true;
	}
	else if (endsWith(filename, "png")) {
		*compression = KINC_IMAGE_COMPRESSION_NONE;
		*internalFormat = 0;
		if (output == NULL) {
			return false;
		}
		int size = Kinc_FileReader_Size(file);
		Kinc_FileReader_Read(file, buffer, size);
		int comp;
		uint8_t *uncompressed = stbi_load_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			Kinc_Log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
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
		int size = Kinc_FileReader_Size(file);
		Kinc_FileReader_Read(file, buffer, size);
		int comp;
		float *uncompressed = stbi_loadf_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			Kinc_Log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
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
		int size = Kinc_FileReader_Size(file);
		Kinc_FileReader_Read(file, buffer, size);
		int comp;
		uint8_t *uncompressed = stbi_load_from_memory(buffer, size, width, height, &comp, 4);
		if (uncompressed == NULL) {
			Kinc_Log(KINC_LOG_LEVEL_ERROR, stbi_failure_reason());
			return false;
		}
		*outputSize = *width * *height * 4;
		memcpy(output, uncompressed, *outputSize);
		return true;
	}
}

int Kinc_ImageFormat_SizeOf(Kinc_ImageFormat format) {
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

static bool formatIsFloatingPoint(Kinc_ImageFormat format) {
	return format == KINC_IMAGE_FORMAT_RGBA128 || format == KINC_IMAGE_FORMAT_RGBA64 || format == KINC_IMAGE_FORMAT_A32 || format == KINC_IMAGE_FORMAT_A16;
}

void Kinc_Image_Create(Kinc_Image *image, int width, int height, Kinc_ImageFormat format, bool readable) {
	Kinc_Image_Create3D(image, width, height, 1, format, readable);
}

void Kinc_Image_Create3D(Kinc_Image *image, int width, int height, int depth, Kinc_ImageFormat format, bool readable) {
	image->width = width;
	image->height = height;
	image->depth = depth;
	image->format = format;
	image->readable = readable;
	image->compression = KINC_IMAGE_COMPRESSION_NONE;

	if (formatIsFloatingPoint(format)) {
		image->hdrData = (float *)malloc(width * height * depth * Kinc_ImageFormat_SizeOf(format));
		image->data = NULL;
	}
	else {
		image->data = malloc(width * height * depth * Kinc_ImageFormat_SizeOf(format));
		image->hdrData = NULL;
	}
}
void Kinc_Image_CreateFromBytes(Kinc_Image *image, void *data, int width, int height, Kinc_ImageFormat format, bool readable) {
	Kinc_Image_CreateFromBytes3D(image, data, width, height, 1, format, readable);
}

void Kinc_Image_CreateFromBytes3D(Kinc_Image *image, void *data, int width, int height, int depth, Kinc_ImageFormat format, bool readable) {
	image->compression = KINC_IMAGE_COMPRESSION_NONE;
	if (formatIsFloatingPoint(format)) {
		image->hdrData = (float*)data;
	}
	else {
		image->data = (uint8_t*)data;
	}
}

void Kinc_Image_CreateFromFile(Kinc_Image *image, const char *filename, bool readable) {
	uint8_t *imageData = NULL;
	Kinc_FileReader reader;
	Kinc_FileReader_Open(&reader, filename, KINC_FILE_TYPE_ASSET);
	int dataSize;
	loadImage(&reader, filename, imageData, &dataSize, &image->width, &image->height, &image->compression, &image->format, &image->internalFormat);
	Kinc_FileReader_Close(&reader);
	if (formatIsFloatingPoint(image->format)) {
		image->hdrData = (float*)imageData;
		image->data = NULL;
	}
	else {
		image->data = imageData;
		image->hdrData = NULL;
	}
}

void Kinc_Image_Destroy(Kinc_Image *image) {
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

int Kinc_Image_At(Kinc_Image *image, int x, int y) {
	if (image->data == NULL)
		return 0;
	else
		return *(int *)&((uint8_t *)image->data)[image->width * Kinc_ImageFormat_SizeOf(image->format) * y + x * Kinc_ImageFormat_SizeOf(image->format)];
}

uint8_t *Kinc_Image_GetPixels(Kinc_Image *image) {
	return image->data != NULL ? image->data : (uint8_t*)image->hdrData;
}
