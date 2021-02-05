#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DVertexDeclaration9;

typedef struct {
	struct IDirect3DVertexDeclaration9 *vertexDecleration;
	int halfPixelLocation;
} kinc_g4_pipeline_impl_t;

void kinc_internal_set_constants(void);

#ifdef __cplusplus
}
#endif
