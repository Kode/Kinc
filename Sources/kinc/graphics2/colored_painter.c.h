#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/math/matrix.h>

#include <kong.h>

static kinc_matrix4x4_t colored_projection_matrix;

// static var standardColorPipeline : PipelineCache = null;
// static VertexStructure structure;

static int colored_rect_buffer_size = 1000;
static int colored_rect_buffer_index;
static kinc_g4_vertex_buffer_t colored_rect_vertex_buffer;
static kinc_g2_colored_vertex_in *colored_rect_vertices;
static kinc_g4_index_buffer_t colored_rect_index_buffer;

static int colored_triangle_buffer_size = 1000;
static int colored_triangle_buffer_index;
static kinc_g4_vertex_buffer_t colored_triangle_vertex_buffer;
static kinc_g2_colored_vertex_in *colored_triangle_vertices;
static kinc_g4_index_buffer_t colored_triangle_index_buffer;

static kinc_g2_constants_type_buffer colored_constants;

// var myPipeline : PipelineCache = null;

static void colored_init_shaders(void);
static void colored_init_buffers(void);
static void colored_end_tris(bool rectsDone);
static void colored_end_rects(bool trisDone);

static void colored_init(void) {
	colored_rect_buffer_index = 0;
	colored_triangle_buffer_index = 0;
	colored_init_shaders();
	// myPipeline = standardColorPipeline;
	colored_init_buffers();
}

// function get_pipeline() : PipelineCache {
//	return myPipeline;
// }

// function set_pipeline(pipe : PipelineCache) : PipelineCache {
//	myPipeline = pipe != null ? pipe : standardColorPipeline;
//	return myPipeline;
// }

static void colored_set_projection(kinc_matrix4x4_t matrix) {
	colored_projection_matrix = matrix;
}

static void colored_init_shaders(void) {
	// if (structure == NULL) {
	//	structure = Graphics2.createColoredVertexStructure();
	// }
	//  if (standardColorPipeline == null) {
	//	var pipeline = Graphics2.createColoredPipeline(structure);
	//	standardColorPipeline = new PerFramebufferPipelineCache(pipeline, false);
	//  }
}

static bool colored_buffers_initialized = false;

static void colored_init_buffers(void) {
	if (!colored_buffers_initialized) {
		kinc_g4_vertex_buffer_init(&colored_rect_vertex_buffer, colored_rect_buffer_size * 4, &kinc_g2_colored_vertex_in_structure, KINC_G4_USAGE_DYNAMIC, 0);
		colored_rect_vertices = (kinc_g2_colored_vertex_in *)kinc_g4_vertex_buffer_lock_all(&colored_rect_vertex_buffer);

		kinc_g4_index_buffer_init(&colored_rect_index_buffer, colored_rect_buffer_size * 3 * 2, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
		int *indices = (int *)kinc_g4_index_buffer_lock_all(&colored_rect_index_buffer);
		for (int i = 0; i < colored_rect_buffer_size; ++i) {
			indices[i * 3 * 2 + 0] = i * 4 + 0;
			indices[i * 3 * 2 + 1] = i * 4 + 1;
			indices[i * 3 * 2 + 2] = i * 4 + 2;
			indices[i * 3 * 2 + 3] = i * 4 + 0;
			indices[i * 3 * 2 + 4] = i * 4 + 2;
			indices[i * 3 * 2 + 5] = i * 4 + 3;
		}
		kinc_g4_index_buffer_unlock_all(&colored_rect_index_buffer);

		kinc_g4_vertex_buffer_init(&colored_triangle_vertex_buffer, colored_triangle_buffer_size * 3, &kinc_g2_colored_vertex_in_structure,
		                           KINC_G4_USAGE_DYNAMIC, 0);
		colored_triangle_vertices = (kinc_g2_colored_vertex_in *)kinc_g4_vertex_buffer_lock_all(&colored_triangle_vertex_buffer);

		kinc_g4_index_buffer_init(&colored_triangle_index_buffer, colored_triangle_buffer_size * 3, KINC_G4_INDEX_BUFFER_FORMAT_32BIT, KINC_G4_USAGE_STATIC);
		int *tri_indices = (int *)kinc_g4_index_buffer_lock_all(&colored_rect_index_buffer);
		for (int i = 0; i < colored_triangle_buffer_size; ++i) {
			tri_indices[i * 3 + 0] = i * 3 + 0;
			tri_indices[i * 3 + 1] = i * 3 + 1;
			tri_indices[i * 3 + 2] = i * 3 + 2;
		}
		kinc_g4_index_buffer_unlock_all(&colored_triangle_index_buffer);

		kinc_g2_constants_type_buffer_init(&colored_constants);

		colored_buffers_initialized = true;
	}
}

static void colored_set_rect_vertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
                                      float bottomrightx, float bottomrighty) {
	int base_index = colored_rect_buffer_index * 4;

	colored_rect_vertices[base_index + 0].pos.x = bottomleftx;
	colored_rect_vertices[base_index + 0].pos.y = bottomlefty;
	colored_rect_vertices[base_index + 0].pos.z = -5.0f;

	colored_rect_vertices[base_index + 1].pos.x = topleftx;
	colored_rect_vertices[base_index + 1].pos.y = toplefty;
	colored_rect_vertices[base_index + 1].pos.z = -5.0f;

	colored_rect_vertices[base_index + 2].pos.x = toprightx;
	colored_rect_vertices[base_index + 2].pos.y = toprighty;
	colored_rect_vertices[base_index + 2].pos.z = -5.0f;

	colored_rect_vertices[base_index + 3].pos.x = bottomrightx;
	colored_rect_vertices[base_index + 3].pos.y = bottomrighty;
	colored_rect_vertices[base_index + 3].pos.z = -5.0f;
}

static void colored_set_rect_colors(float opacity, uint32_t color) {
	int base_index = colored_rect_buffer_index * 4;

	float color_components[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};

	float a = opacity * color_components[0];
	float r = a * color_components[1];
	float g = a * color_components[2];
	float b = a * color_components[3];

	for (int i = 0; i < 4; ++i) {
		colored_rect_vertices[base_index + i].col.x = r;
		colored_rect_vertices[base_index + i].col.y = g;
		colored_rect_vertices[base_index + i].col.z = b;
		colored_rect_vertices[base_index + i].col.w = a;
	}
}

static void colored_set_tri_vertices(float x1, float y1, float x2, float y2, float x3, float y3) {
	int base_index = colored_triangle_buffer_index * 3;

	colored_triangle_vertices[base_index + 0].pos.x = x1;
	colored_triangle_vertices[base_index + 0].pos.y = y1;
	colored_triangle_vertices[base_index + 0].pos.z = -5.0f;

	colored_triangle_vertices[base_index + 1].pos.x = x2;
	colored_triangle_vertices[base_index + 1].pos.y = y2;
	colored_triangle_vertices[base_index + 1].pos.z = -5.0f;

	colored_triangle_vertices[base_index + 2].pos.x = x3;
	colored_triangle_vertices[base_index + 2].pos.y = y3;
	colored_triangle_vertices[base_index + 2].pos.z = -5.0f;
}

static void colored_set_tri_colors(float opacity, uint32_t color) {
	int base_index = colored_triangle_buffer_index * 3;

	float color_components[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
	                            ((color & 0xff000000) >> 24) / 255.0f};

	float a = opacity * color_components[0];
	float r = a * color_components[1];
	float g = a * color_components[2];
	float b = a * color_components[3];

	for (int i = 0; i < 4; ++i) {
		colored_triangle_vertices[base_index + i].col.x = r;
		colored_triangle_vertices[base_index + i].col.y = g;
		colored_triangle_vertices[base_index + i].col.z = b;
		colored_triangle_vertices[base_index + i].col.w = a;
	}
}

static void colored_draw_rect_buffer(bool trisDone) {
	if (colored_rect_buffer_index == 0) {
		return;
	}

	if (!trisDone) {
		colored_end_tris(true);
	}

	kinc_g2_constants_type *constants_data = kinc_g2_constants_type_buffer_lock(&colored_constants);
	constants_data->projection = colored_projection_matrix;
	kinc_g2_constants_type_buffer_unlock(&colored_constants);

	kinc_g4_vertex_buffer_unlock(&colored_rect_vertex_buffer, colored_rect_buffer_index * 4);
	// PipelineState pipeline = myPipeline.get(null, Depth24Stencil8);
	kinc_g4_set_pipeline(&kinc_g2_colored_pipeline);
	kinc_g4_set_vertex_buffer(&colored_rect_vertex_buffer);
	kinc_g4_set_index_buffer(&colored_rect_index_buffer);
	kinc_g2_constants_type_buffer_set(&colored_constants);

	kinc_g4_draw_indexed_vertices_from_to(0, colored_rect_buffer_index * 2 * 3);

	colored_rect_buffer_index = 0;
	colored_rect_vertices = (kinc_g2_colored_vertex_in *)kinc_g4_vertex_buffer_lock_all(&colored_rect_vertex_buffer);
}

static void colored_draw_tri_buffer(bool rectsDone) {
	if (!rectsDone) {
		colored_end_rects(true);
	}

	kinc_g2_constants_type *constants_data = kinc_g2_constants_type_buffer_lock(&colored_constants);
	constants_data->projection = colored_projection_matrix;
	kinc_g2_constants_type_buffer_unlock(&colored_constants);

	kinc_g4_vertex_buffer_unlock(&colored_triangle_vertex_buffer, colored_triangle_buffer_index * 3);
	// PipelineState pipeline = myPipeline.get(null, Depth24Stencil8);
	kinc_g4_set_pipeline(&kinc_g2_colored_pipeline);
	kinc_g4_set_vertex_buffer(&colored_triangle_vertex_buffer);
	kinc_g4_set_index_buffer(&colored_triangle_index_buffer);
	kinc_g2_constants_type_buffer_set(&colored_constants);

	kinc_g4_draw_indexed_vertices_from_to(0, colored_triangle_buffer_index * 3);

	colored_triangle_buffer_index = 0;
	colored_triangle_vertices = (kinc_g2_colored_vertex_in *)kinc_g4_vertex_buffer_lock_all(&colored_triangle_vertex_buffer);
}

static void colored_fill_rect(float opacity, uint32_t color, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx,
                              float toprighty, float bottomrightx, float bottomrighty) {
	if (colored_triangle_buffer_index > 0) {
		colored_draw_tri_buffer(true); // Flush other buffer for right render order
	}

	if (colored_rect_buffer_index + 1 >= colored_rect_buffer_size) {
		colored_draw_rect_buffer(false);
	}

	colored_set_rect_colors(opacity, color);
	colored_set_rect_vertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
	++colored_rect_buffer_index;
}

static void colored_fill_triangle(float opacity, uint32_t color, float x1, float y1, float x2, float y2, float x3, float y3) {
	if (colored_rect_buffer_index > 0) {
		colored_draw_rect_buffer(true); // Flush other buffer for right render order
	}

	if (colored_triangle_buffer_index + 1 >= colored_triangle_buffer_size) {
		colored_draw_tri_buffer(false);
	}

	colored_set_tri_colors(opacity, color);
	colored_set_tri_vertices(x1, y1, x2, y2, x3, y3);
	++colored_triangle_buffer_index;
}

static void colored_end_tris(bool rectsDone) {
	if (colored_triangle_buffer_index > 0) {
		colored_draw_tri_buffer(rectsDone);
	}
}

static void colored_end_rects(bool trisDone) {
	if (colored_rect_buffer_index > 0) {
		colored_draw_rect_buffer(trisDone);
	}
}

static void colored_end(void) {
	colored_end_tris(false);
	colored_end_rects(false);
}
