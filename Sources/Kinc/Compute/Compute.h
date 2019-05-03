#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <Kore/ComputeImpl.h>
#ifdef KORE_OPENGL
#include <Kore/ShaderStorageBufferImpl.h>
#endif
#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/Math/Matrix.h>

struct kinc_g4_texture;
struct kinc_g4_render_target;

typedef struct kinc_compute_constant_location {
	ComputeConstantLocationImpl impl;
} kinc_compute_constant_location_t;

typedef struct kinc_compute_texture_unit {
	ComputeTextureUnitImpl impl;
} kinc_compute_texture_unit_t;

typedef struct kinc_compute_shader {
public:
	ComputeShaderImpl impl;
} kinc_compute_shader_t;

void kinc_compute_shader_init(kinc_compute_shader_t *shader, void *source, int length);
void kinc_compute_shader_destroy(kinc_compute_shader_t *shader);
kinc_compute_constant_location_t kinc_compute_shader_get_constant_location(kinc_compute_shader_t *shader, const char *name);
kinc_compute_texture_unit_t kinc_compute_shader_get_texture_unit(kinc_compute_shader_t *shader, const char *name);


#ifdef KORE_OPENGL
class ShaderStorageBuffer : public ShaderStorageBufferImpl {
public:
	ShaderStorageBuffer(int count, Graphics4::VertexData type);
	virtual ~ShaderStorageBuffer();
	int *lock();
	void unlock();
	int count();
	void _set();
};
#endif

typedef enum kinc_compute_access { KINC_COMPUTE_ACCESS_READ, KINC_COMPUTE_ACCESS_WRITE, KINC_COMPUTE_ACCESS_READ_WRITE } kinc_compute_access_t;

void kinc_compute_set_bool(kinc_compute_constant_location_t location, bool value);
void kinc_compute_set_int(kinc_compute_constant_location_t location, int value);
void kinc_compute_set_float(kinc_compute_constant_location_t location, float value);
void kinc_compute_set_float2(kinc_compute_constant_location_t location, float value1, float value2);
void kinc_compute_set_float3(kinc_compute_constant_location_t location, float value1, float value2, float value3);
void kinc_compute_set_float4(kinc_compute_constant_location_t location, float value1, float value2, float value3, float value4);
void kinc_compute_set_floats(kinc_compute_constant_location_t location, float *values, int count);
void kinc_compute_set_matrix4(kinc_compute_constant_location_t location, Kinc_Matrix4x4 *value);
void kinc_compute_set_matrix3(kinc_compute_constant_location_t location, Kinc_Matrix3x3 *value);
#ifdef KORE_OPENGL
void kinc_compute_set_buffer(ShaderStorageBuffer *buffer, int index);
#endif
void kinc_compute_set_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture *texture, kinc_compute_access_t access);
void kinc_compute_set_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *texture, kinc_compute_access_t access);
void kinc_compute_set_sampled_texture(kinc_compute_texture_unit_t unit, kinc_g4_texture *texture);
void kinc_compute_set_sampled_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *target);
void kinc_compute_set_sampled_depth_from_render_target(kinc_compute_texture_unit_t unit, kinc_g4_render_target *target);
void kinc_compute_set_texture_addressing(kinc_compute_texture_unit_t unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);
void kinc_compute_set_texture_magnification_filter(kinc_compute_texture_unit_t unit, Kinc_G4_TextureFilter filter);
void kinc_compute_set_set_texture_minification_filter(kinc_compute_texture_unit_t unit, Kinc_G4_TextureFilter filter);
void kinc_compute_set_texture_mipmap_filter(kinc_compute_texture_unit_t unit, Kinc_G4_MipmapFilter filter);
void kinc_compute_set_texture3d_addressing(kinc_compute_texture_unit_t unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);
void kinc_compute_set_texture3d_magnification_filter(kinc_compute_texture_unit_t unit, Kinc_G4_TextureFilter filter);
void kinc_compute_set_texture3d_minification_filter(kinc_compute_texture_unit_t unit, Kinc_G4_TextureFilter filter);
void kinc_compute_set_texture3d_mipmap_filter(kinc_compute_texture_unit_t unit, Kinc_G4_MipmapFilter filter);
void kinc_compute_set_shader(kinc_compute_shader_t *shader);
void kinc_compute(int x, int y, int z);

#ifdef __cplusplus
}
#endif
