#include <kinc/global.h>
#include <kinc/io/filereader.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <assert.h>
#include <stdlib.h>

#define BUFFER_COUNT 2
static int current_buffer = -1;
static kinc_g5_render_target_t framebuffers[BUFFER_COUNT];
static kinc_g5_command_list_t command_list;
static kinc_g5_shader_t vertex_shader;
static kinc_g5_shader_t fragment_shader;
static kinc_g5_pipeline_t pipeline;
static kinc_g5_vertex_buffer_t vertices;
static kinc_g5_index_buffer_t indices;

#define HEAP_SIZE 1024 * 1024
static uint8_t *heap = NULL;
static size_t heap_top = 0;

static void *allocate(size_t size) {
	size_t old_top = heap_top;
	heap_top += size;
	assert(heap_top <= HEAP_SIZE);
	return &heap[old_top];
}

static void update() {
	current_buffer = (current_buffer + 1) % BUFFER_COUNT;

	kinc_g5_begin(&framebuffers[current_buffer], 0);

	kinc_g5_command_list_begin(&command_list);
	kinc_g5_command_list_framebuffer_to_render_target_barrier(&command_list, &framebuffers[current_buffer]);
	kinc_g5_render_target_t *renderTargets[1] = {&framebuffers[current_buffer]};
	kinc_g5_command_list_set_render_targets(&command_list, renderTargets, 1);

	kinc_g5_command_list_clear(&command_list, &framebuffers[current_buffer], KINC_G5_CLEAR_COLOR, 0, 0.0f, 0);
	kinc_g5_command_list_set_pipeline(&command_list, &pipeline);
	kinc_g5_command_list_set_pipeline_layout(&command_list);

	int offsets[1] = { 0 };
	kinc_g5_vertex_buffer_t *vertex_buffers[1] = { &vertices };
	kinc_g5_command_list_set_vertex_buffers(&command_list, vertex_buffers, offsets, 1);
	kinc_g5_command_list_set_index_buffer(&command_list, &indices);
	kinc_g5_command_list_draw_indexed_vertices(&command_list);

	kinc_g5_command_list_render_target_to_framebuffer_barrier(&command_list, &framebuffers[current_buffer]);
	kinc_g5_command_list_end(&command_list);
	kinc_g5_command_list_execute_and_wait(&command_list);

	kinc_g5_end(0);
	kinc_g5_swap_buffers();
}

static void load_shader(const char *filename, kinc_g5_shader_t *shader, kinc_g5_shader_type_t shader_type) {
	kinc_file_reader_t file;
	kinc_file_reader_open(&file, filename, KINC_FILE_TYPE_ASSET);
	size_t data_size = kinc_file_reader_size(&file);
	uint8_t *data = allocate(data_size);
	kinc_file_reader_read(&file, data, data_size);
	kinc_file_reader_close(&file);
	kinc_g5_shader_init(shader, data, data_size, shader_type);
}

int kickstart(int argc, char** argv) {
	kinc_init("Shader", 1024, 768, NULL, NULL);
	kinc_set_update_callback(update);

	heap = (uint8_t*)malloc(HEAP_SIZE);
	assert(heap != NULL);

	load_shader("shader.vert", &vertex_shader, KINC_G5_SHADER_TYPE_VERTEX);
	load_shader("shader.frag", &fragment_shader, KINC_G5_SHADER_TYPE_FRAGMENT);

	kinc_g5_vertex_structure_t structure;
	kinc_g4_vertex_structure_init(&structure);
	kinc_g4_vertex_structure_add(&structure, "pos", KINC_G4_VERTEX_DATA_FLOAT3);
	kinc_g5_pipeline_init(&pipeline);
	pipeline.vertexShader = &vertex_shader;
	pipeline.fragmentShader = &fragment_shader;
	pipeline.inputLayout[0] = &structure;
	pipeline.inputLayout[1] = NULL;
	kinc_g5_pipeline_compile(&pipeline);

	kinc_g5_command_list_init(&command_list);
	for (int i = 0; i < BUFFER_COUNT; ++i) {
		kinc_g5_render_target_init(&framebuffers[i], kinc_window_width(0), kinc_window_height(0), 16, false, KINC_G5_RENDER_TARGET_FORMAT_32BIT, -1,
			-i - 1 /* hack in an index for backbuffer render targets */);
	}

	kinc_g5_vertex_buffer_init(&vertices, 3, &structure, true, 0);
	float *v = kinc_g5_vertex_buffer_lock_all(&vertices);
	v[0] = -1; v[1] = -1; v[2] = 0.5;
	v[3] = 1;  v[4] = -1; v[5] = 0.5;
	v[6] = -1; v[7] = 1;  v[8] = 0.5;
	kinc_g5_vertex_buffer_unlock_all(&vertices);
	kinc_g5_command_list_upload_vertex_buffer(&command_list, &vertices);

	kinc_g5_index_buffer_init(&indices, 3, KINC_G5_INDEX_BUFFER_FORMAT_32BIT, true);
	int *i = kinc_g5_index_buffer_lock(&indices);
	i[0] = 0; i[1] = 1; i[2] = 2;
	kinc_g5_index_buffer_unlock(&indices);
	kinc_g5_command_list_upload_index_buffer(&command_list, &indices);

	kinc_start();

	return 0;
}
