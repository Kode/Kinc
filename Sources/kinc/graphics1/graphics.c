#include "graphics.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/io/filereader.h>
#include <kinc/log.h>

#ifndef KOPE

#ifdef KINC_KONG
#include <kong.h>
#endif

#ifndef KINC_KONG
static kinc_g4_shader_t vertexShader;
static kinc_g4_shader_t fragmentShader;
static kinc_g4_pipeline_t pipeline;
static kinc_g4_texture_unit_t tex;
kinc_g1_texture_filter_t kinc_internal_g1_texture_filter_min = KINC_G1_TEXTURE_FILTER_LINEAR;
kinc_g1_texture_filter_t kinc_internal_g1_texture_filter_mag = KINC_G1_TEXTURE_FILTER_LINEAR;
kinc_g1_mipmap_filter_t kinc_internal_g1_mipmap_filter = KINC_G1_MIPMAP_FILTER_NONE;
#endif
static kinc_g4_vertex_buffer_t vb;
static kinc_g4_index_buffer_t ib;
static kinc_g4_texture_t texture;

uint32_t *kinc_internal_g1_image;
int kinc_internal_g1_w, kinc_internal_g1_h, kinc_internal_g1_tex_width;

void kinc_g1_begin(void) {
	kinc_g4_begin(0);
	kinc_internal_g1_image = (uint32_t *)kinc_g4_texture_lock(&texture);
}

static inline kinc_g4_texture_filter_t map_texture_filter(kinc_g1_texture_filter_t filter) {
	switch (filter) {
	case KINC_G1_TEXTURE_FILTER_POINT:
		return KINC_G4_TEXTURE_FILTER_POINT;
	case KINC_G1_TEXTURE_FILTER_LINEAR:
		return KINC_G4_TEXTURE_FILTER_LINEAR;
	case KINC_G1_TEXTURE_FILTER_ANISOTROPIC:
		return KINC_G4_TEXTURE_FILTER_ANISOTROPIC;
	}

	kinc_log(KINC_LOG_LEVEL_WARNING, "unhandled kinc_g1_texture_filter_t (%i)", filter);
	return KINC_G1_TEXTURE_FILTER_LINEAR;
}

static inline kinc_g4_texture_filter_t map_mipmap_filter(kinc_g1_texture_filter_t filter) {
	switch (filter) {
	case KINC_G1_MIPMAP_FILTER_NONE:
		return KINC_G4_MIPMAP_FILTER_NONE;
	case KINC_G1_MIPMAP_FILTER_POINT:
		return KINC_G4_MIPMAP_FILTER_POINT;
	case KINC_G1_MIPMAP_FILTER_LINEAR:
		return KINC_G4_MIPMAP_FILTER_LINEAR;
	}

	kinc_log(KINC_LOG_LEVEL_WARNING, "unhandled kinc_g1_mipmap_filter_t (%i)", filter);
	return KINC_G4_MIPMAP_FILTER_NONE;
}

void kinc_g1_end(void) {
	kinc_internal_g1_image = NULL;
	kinc_g4_texture_unlock(&texture);

	kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0xff000000, 0.0f, 0);

#ifdef KINC_KONG
	kinc_g4_set_pipeline(&kinc_g1_pipeline);
#else
	kinc_g4_set_pipeline(&pipeline);
#endif

#ifndef KINC_KONG
	kinc_g4_set_texture(tex, &texture);
	kinc_g4_set_texture_minification_filter(tex, map_texture_filter(kinc_internal_g1_texture_filter_min));
	kinc_g4_set_texture_magnification_filter(tex, map_texture_filter(kinc_internal_g1_texture_filter_mag));
	kinc_g4_set_texture_mipmap_filter(tex, map_mipmap_filter(kinc_internal_g1_mipmap_filter));
#endif
	kinc_g4_set_vertex_buffer(&vb);
	kinc_g4_set_index_buffer(&ib);
	kinc_g4_draw_indexed_vertices();

	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

void kinc_g1_init(int width, int height) {
	kinc_internal_g1_w = width;
	kinc_internal_g1_h = height;

#ifndef KINC_KONG
	{
		kinc_file_reader_t file;
		kinc_file_reader_open(&file, "g1.vert", KINC_FILE_TYPE_ASSET);
		void *data = malloc(kinc_file_reader_size(&file));
		kinc_file_reader_read(&file, data, kinc_file_reader_size(&file));
		kinc_file_reader_close(&file);
		kinc_g4_shader_init(&vertexShader, data, kinc_file_reader_size(&file), KINC_G4_SHADER_TYPE_VERTEX);
		free(data);
	}

	{
		kinc_file_reader_t file;
		kinc_file_reader_open(&file, "g1.frag", KINC_FILE_TYPE_ASSET);
		void *data = malloc(kinc_file_reader_size(&file));
		kinc_file_reader_read(&file, data, kinc_file_reader_size(&file));
		kinc_file_reader_close(&file);
		kinc_g4_shader_init(&fragmentShader, data, kinc_file_reader_size(&file), KINC_G4_SHADER_TYPE_FRAGMENT);
		free(data);
	}

	kinc_g4_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_F32_3X);
	kinc_g4_vertex_structure_add(&structure, "tex", KINC_G4_VERTEX_DATA_F32_2X);
	kinc_g4_pipeline_init(&pipeline);
	pipeline.input_layout[0] = &structure;
	pipeline.input_layout[1] = NULL;
	pipeline.vertex_shader = &vertexShader;
	pipeline.fragment_shader = &fragmentShader;
	kinc_g4_pipeline_compile(&pipeline);

	tex = kinc_g4_pipeline_get_texture_unit(&pipeline, "texy");
#endif

	kinc_g4_texture_init(&texture, width, height, KINC_IMAGE_FORMAT_RGBA32);
	kinc_internal_g1_tex_width = texture.tex_width;

	kinc_internal_g1_image = (uint32_t *)kinc_g4_texture_lock(&texture);
	int stride = kinc_g4_texture_stride(&texture) / 4;
	for (int y = 0; y < texture.tex_height; ++y) {
		for (int x = 0; x < texture.tex_width; ++x) {
			kinc_internal_g1_image[y * stride + x] = 0;
		}
	}
	kinc_g4_texture_unlock(&texture);

	// Correct for the difference between the texture's desired size and the actual power of 2 size
	float xAspect = (float)width / texture.tex_width;
	float yAspect = (float)height / texture.tex_height;

#ifdef KINC_KONG
	kinc_g4_vertex_buffer_init(&vb, 4, &kinc_g1_vertex_in_structure, KINC_G4_USAGE_STATIC, 0);
#else
	kinc_g4_vertex_buffer_init(&vb, 4, &structure, KINC_G4_USAGE_STATIC, 0);
#endif
	float *v = kinc_g4_vertex_buffer_lock_all(&vb);
	{
		int i = 0;
		v[i++] = -1;
		v[i++] = 1;
		v[i++] = 0.5;
		v[i++] = 0;
		v[i++] = 0;
		v[i++] = 1;
		v[i++] = 1;
		v[i++] = 0.5;
		v[i++] = xAspect;
		v[i++] = 0;
		v[i++] = 1;
		v[i++] = -1;
		v[i++] = 0.5;
		v[i++] = xAspect;
		v[i++] = yAspect;
		v[i++] = -1;
		v[i++] = -1;
		v[i++] = 0.5;
		v[i++] = 0;
		v[i++] = yAspect;
	}
	kinc_g4_vertex_buffer_unlock_all(&vb);

	kinc_g4_index_buffer_init(&ib, 6, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
	uint32_t *ii = (uint32_t *)kinc_g4_index_buffer_lock_all(&ib);
	{
		int i = 0;
		ii[i++] = 0;
		ii[i++] = 1;
		ii[i++] = 3;
		ii[i++] = 1;
		ii[i++] = 2;
		ii[i++] = 3;
	}
	kinc_g4_index_buffer_unlock_all(&ib);
}

#if defined(KINC_DYNAMIC_COMPILE) || defined(KINC_DYNAMIC)

void kinc_g1_set_pixel(int x, int y, float red, float green, float blue) {
	if (x < 0 || x >= kinc_internal_g1_w || y < 0 || y >= kinc_internal_g1_h)
		return;
	int r = (int)(red * 255);
	int g = (int)(green * 255);
	int b = (int)(blue * 255);
	kinc_internal_g1_image[y * kinc_internal_g1_tex_width + x] = 0xff << 24 | b << 16 | g << 8 | r;
}

int kinc_g1_width() {
	return kinc_internal_g1_w;
}

int kinc_g1_height() {
	return kinc_internal_g1_h;
}

void kinc_g1_set_texture_magnification_filter(kinc_g1_texture_filter_t filter) {
	kinc_internal_g1_texture_filter_mag = filter;
}

void kinc_g1_set_texture_minification_filter(kinc_g1_texture_filter_t filter) {
	kinc_internal_g1_texture_filter_min = filter;
}

void kinc_g1_set_texture_mipmap_filter(kinc_g1_mipmap_filter_t filter) {
	kinc_internal_g1_mipmap_filter = filter;
}

#endif

#endif
