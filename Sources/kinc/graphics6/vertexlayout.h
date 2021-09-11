#pragma once

#include <kinc/global.h>

/*! \file vertexlayout.h
    \brief Provides functions for setting up vertex layouts.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_g6_vertex_format {
	KINC_G6_VERTEX_FORMAT_UINT8X2,
	KINC_G6_VERTEX_FORMAT_UINT8X4,
	KINC_G6_VERTEX_FORMAT_SINT8X2,
	KINC_G6_VERTEX_FORMAT_SINT8X4,
	KINC_G6_VERTEX_FORMAT_UNORM8X2,
	KINC_G6_VERTEX_FORMAT_UNORM8X4,
	KINC_G6_VERTEX_FORMAT_SNORM8X2,
	KINC_G6_VERTEX_FORMAT_SNORM8X4,
	KINC_G6_VERTEX_FORMAT_UINT16X2,
	KINC_G6_VERTEX_FORMAT_UINT16X4,
	KINC_G6_VERTEX_FORMAT_SINT16X2,
	KINC_G6_VERTEX_FORMAT_SINT16X4,
	KINC_G6_VERTEX_FORMAT_UNORM16X2,
	KINC_G6_VERTEX_FORMAT_UNORM16X4,
	KINC_G6_VERTEX_FORMAT_SNORM16X2,
	KINC_G6_VERTEX_FORMAT_SNORM16X4,
	KINC_G6_VERTEX_FORMAT_FLOAT16X2,
	KINC_G6_VERTEX_FORMAT_FLOAT16X4,
	KINC_G6_VERTEX_FORMAT_FLOAT32,
	KINC_G6_VERTEX_FORMAT_FLOAT32X2,
	KINC_G6_VERTEX_FORMAT_FLOAT32X3,
	KINC_G6_VERTEX_FORMAT_FLOAT32X4,
	KINC_G6_VERTEX_FORMAT_UINT32,
	KINC_G6_VERTEX_FORMAT_UINT32X2,
	KINC_G6_VERTEX_FORMAT_UINT32X3,
	KINC_G6_VERTEX_FORMAT_UINT32X4,
	KINC_G6_VERTEX_FORMAT_SINT32,
	KINC_G6_VERTEX_FORMAT_SINT32X2,
	KINC_G6_VERTEX_FORMAT_SINT32X3,
	KINC_G6_VERTEX_FORMAT_SINT32X4
} kinc_g6_vertex_format_t;

#define KINC_G6_MAX_VERTEX_ATTRIBUTES 16

typedef struct kinc_g6_vertex_attribute {
	kinc_g6_vertex_format_t format;
	uint64_t offset;
	uint32_t location;
} kinc_g6_vertex_attribute_t;

typedef struct kinc_g6_vertex_layout {
	uint64_t stride;
	bool instanced;
	int attribute_count;
	kinc_g6_vertex_attribute_t attributes[KINC_G6_MAX_VERTEX_ATTRIBUTES];
} kinc_g6_vertex_layout_t;

KINC_FUNC void kinc_g6_vertex_layout_init(kinc_g6_vertex_layout_t *layout);
KINC_FUNC void kinc_g6_vertex_layout_add(kinc_g6_vertex_layout_t *layout, uint64_t offset, uint32_t location, kinc_g6_vertex_format_t format);

#ifdef __cplusplus
}
#endif