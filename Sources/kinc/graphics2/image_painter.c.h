#include <Kinc/math/matrix.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/vertexbuffer.h>

#include <kong.h>

static kinc_matrix4x4_t image_projection_matrix;

// static var standardImagePipeline : PipelineCache = null;
static int image_buffer_size = 1500;
static int image_vertex_size = 6;
static int image_buffer_start;
static int image_buffer_index;
static kinc_g4_vertex_buffer_t image_vertex_buffer;
static kinc_g2_image_vertex_in *image_vertices;
static kinc_g4_index_buffer_t image_index_buffer;
static kinc_g4_texture_t *image_last_texture = NULL;

static kinc_g2_constants_type_buffer image_constants;

static bool image_bilinear = false;
static bool image_bilinear_mipmaps = false;
// var myPipeline : PipelineCache = null;

static void image_init_shaders(void);
static void image_init_buffers(void);

static void image_init(void) {
	image_buffer_start = 0;
	image_buffer_index = 0;
	image_init_shaders();
	// myPipeline = standardImagePipeline;
	image_init_buffers();
}

// function get_pipeline() : PipelineCache {
//	return myPipeline;
// }

// function set_pipeline(pipe : PipelineCache) : PipelineCache {
//	myPipeline = pipe != null ? pipe : standardImagePipeline;
//	return myPipeline;
// }

static void image_set_projection(kinc_matrix4x4_t matrix) {
	image_projection_matrix = matrix;
}

static void image_init_shaders(void) {
	// if (structure == NULL) {
	//	structure = Graphics2.createImageVertexStructure();
	// }
	//  if (standardImagePipeline == null) {
	//	var pipeline = Graphics2.createImagePipeline(structure);
	//	standardImagePipeline = new PerFramebufferPipelineCache(pipeline, true);
	//  }
}

static bool image_buffers_initialized = false;

static void image_init_buffers(void) {
	if (!image_buffers_initialized) {
		kinc_g4_vertex_buffer_init(&image_vertex_buffer, image_buffer_size * 4, &kinc_g2_image_vertex_in_structure, KINC_G4_USAGE_DYNAMIC, 0);
		image_vertices = (kinc_g2_image_vertex_in *)kinc_g4_vertex_buffer_lock_all(&image_vertex_buffer);

		kinc_g4_index_buffer_init(&image_index_buffer, image_buffer_size * 3 * 2, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
		int *indices = (int *)kinc_g4_index_buffer_lock_all(&image_index_buffer);
		for (int i = 0; i < image_buffer_size; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		kinc_g4_index_buffer_unlock_all(&image_index_buffer);

		kinc_g2_constants_type_buffer_init(&image_constants);

		image_buffers_initialized = true;
	}
}

static void image_set_vertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx,
                               float bottomrighty) {
	int base_index = (image_buffer_index - image_buffer_start) * 4;

	image_vertices[base_index + 0].pos.x = bottomleftx;
	image_vertices[base_index + 0].pos.y = bottomlefty;
	image_vertices[base_index + 0].pos.z = -5.0f;

	image_vertices[base_index + 1].pos.x = topleftx;
	image_vertices[base_index + 1].pos.y = toplefty;
	image_vertices[base_index + 1].pos.z = -5.0f;

	image_vertices[base_index + 2].pos.x = toprightx;
	image_vertices[base_index + 2].pos.y = toprighty;
	image_vertices[base_index + 2].pos.z = -5.0f;

	image_vertices[base_index + 3].pos.x = bottomrightx;
	image_vertices[base_index + 3].pos.y = bottomrighty;
	image_vertices[base_index + 3].pos.z = -5.0f;
}

static void image_set_tex_coords(float left, float top, float right, float bottom) {
	int base_index = (image_buffer_index - image_buffer_start) * 4;

	image_vertices[base_index + 0].tex.x = left;
	image_vertices[base_index + 0].tex.y = bottom;

	image_vertices[base_index + 1].tex.x = left;
	image_vertices[base_index + 1].tex.y = top;

	image_vertices[base_index + 2].tex.x = right;
	image_vertices[base_index + 2].tex.y = top;

	image_vertices[base_index + 3].tex.x = right;
	image_vertices[base_index + 3].tex.y = bottom;
}

static void image_set_color(float r, float g, float b, float a) {
	int base_index = (image_buffer_index - image_buffer_start) * 4;

	for (int i = 0; i < 4; ++i) {
		image_vertices[base_index + i].col.x = r;
		image_vertices[base_index + i].col.y = g;
		image_vertices[base_index + i].col.z = b;
		image_vertices[base_index + i].col.w = a;
	}
}

static void image_draw_buffer(bool end) {
	if (image_buffer_index - image_buffer_start == 0) {
		return;
	}

	kinc_g2_constants_type *constants_data = kinc_g2_constants_type_buffer_lock(&image_constants);
	constants_data->projection = image_projection_matrix;
	kinc_g2_constants_type_buffer_unlock(&image_constants);

	kinc_g4_vertex_buffer_unlock(&image_vertex_buffer, (image_buffer_index - image_buffer_start) * 4);
	// PipelineState *pipeline = myPipeline.get(NULL, Depth24Stencil8);
	kinc_g4_set_pipeline(&kinc_g2_image_pipeline);
	kinc_g4_set_vertex_buffer(&image_vertex_buffer);
	kinc_g4_set_index_buffer(&image_index_buffer);
	kinc_g2_constants_type_buffer_set(&image_constants);
	kinc_g4_set_texture(kinc_g2_texture, image_last_texture);
	// kinc_g4_set_texture_minification_filter(kinc_g2_texture, image_bilinear ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	// kinc_g4_set_texture_magnification_filter(kinc_g2_texture, image_bilinear ? KINC_G4_TEXTURE_FILTER_LINEAR : KINC_G4_TEXTURE_FILTER_POINT);
	// kinc_g4_set_texture_mipmap_filter(kinc_g2_texture, image_bilinear_mipmaps ? KINC_G4_MIPMAP_FILTER_LINEAR : KINC_G4_MIPMAP_FILTER_POINT);

	kinc_g4_draw_indexed_vertices_from_to(image_buffer_start * 2 * 3, (image_buffer_index - image_buffer_start) * 2 * 3);

	// kinc_g4_set_texture(kinc_g2_texture, NULL);

	if (end || (image_buffer_start + image_buffer_index + 1) * 4 >= image_buffer_size) {
		image_buffer_start = 0;
		image_buffer_index = 0;
		image_vertices = (kinc_g2_image_vertex_in *)kinc_g4_vertex_buffer_lock_all(&image_vertex_buffer);
	}
	else {
		image_buffer_start = image_buffer_index;
		image_vertices = (kinc_g2_image_vertex_in *)kinc_g4_vertex_buffer_lock(&image_vertex_buffer, image_buffer_start * 4,
		                                                                       kinc_g4_vertex_buffer_count(&image_vertex_buffer) - image_buffer_start * 4);
	}
}

static void image_set_bilinear_filter(bool bilinear) {
	image_draw_buffer(false);
	image_last_texture = NULL;
	image_bilinear = bilinear;
}

static void image_set_bilinear_mipmap_filter(bool bilinear) {
	image_draw_buffer(false);
	image_last_texture = NULL;
	image_bilinear_mipmaps = bilinear;
}

static void image_draw_image(kinc_g4_texture_t *tex, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
                             float bottomrightx, float bottomrighty, float opacity, uint32_t color) {
	if (image_buffer_start + image_buffer_index + 1 >= image_buffer_size || (image_last_texture != NULL && tex != image_last_texture)) {
		image_draw_buffer(false);
	}

	float color_components[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};

	float a = opacity * color_components[0];
	float r = a * color_components[1];
	float g = a * color_components[2];
	float b = a * color_components[3];

	image_set_tex_coords(0, 0, 1.0f, 1.0f);
	image_set_color(r, g, b, a * opacity);
	image_set_vertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++image_buffer_index;
	image_last_texture = tex;
}

static void image_draw_image2(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx,
                              float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, uint32_t color) {
	if (image_buffer_start + image_buffer_index + 1 >= image_buffer_size || (image_last_texture != NULL && tex != image_last_texture)) {
		image_draw_buffer(false);
	}

	float color_components[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};

	float a = opacity * color_components[0];
	float r = a * color_components[1];
	float g = a * color_components[2];
	float b = a * color_components[3];

	image_set_tex_coords(sx / tex->tex_width, sy / tex->tex_height, (sx + sw) / tex->tex_width, (sy + sh) / tex->tex_height);
	image_set_color(r, g, b, a * opacity);
	image_set_vertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++image_buffer_index;
	image_last_texture = tex;
}

static void image_draw_image_scale(kinc_g4_texture_t *tex, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom,
                                   float opacity, uint32_t color) {
	if (image_buffer_start + image_buffer_index + 1 >= image_buffer_size || (image_last_texture != NULL && tex != image_last_texture)) {
		image_draw_buffer(false);
	}

	float color_components[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};

	float a = opacity * color_components[0];
	float r = a * color_components[1];
	float g = a * color_components[2];
	float b = a * color_components[3];

	image_set_tex_coords(sx / tex->tex_width, sy / tex->tex_height, (sx + sw) / tex->tex_width, (sy + sh) / tex->tex_height);
	image_set_color(r, g, b, a * opacity);
	image_set_vertices(left, bottom, left, top, right, top, right, bottom);

	++image_buffer_index;
	image_last_texture = tex;
}

static void image_end(void) {
	if (image_buffer_index > 0) {
		image_draw_buffer(true);
	}
	image_last_texture = NULL;
}
