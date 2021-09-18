#pragma once

typedef struct {
	int location;
	unsigned int type;
} kinc_compute_constant_location_impl_t;

typedef struct {
	int unit;
} kinc_compute_texture_unit_impl_t;

typedef struct {
	// int findTexture(const char* name);
	char **textures;
	int *textureValues;
	int textureCount;
	unsigned _id;
	unsigned _programid;
	char *_source;
	int _length;
} kinc_compute_shader_impl_t;
