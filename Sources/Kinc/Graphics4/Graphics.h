#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void Kinc_G4_Begin(int window);

void Kinc_G4_End(int window);

bool Kinc_G4_SwapBuffers();

#define KINC_G4_CLEAR_COLOR   1
#define KINC_G4_CLEAR_DEPTH   2
#define KINC_G4_CLEAR_STENCIL 4

void Kinc_G4_Clear(unsigned flags, unsigned color, float depth, int stencil);

void Kinc_G4_Viewport(int x, int y, int width, int height);

void Kinc_G4_Scissor(int x, int y, int width, int height);

void Kinc_G4_DisableScissor();

void Kinc_G4_DrawIndexedVertices();

void Kinc_G4_DrawIndexedVerticesFromTo(int start, int count);

void Kinc_G4_DrawIndexedVerticesInstanced(int instanceCount);

void Kinc_G4_DrawIndexedVerticesInstancedFromTo(int instanceCount, int start, int count);

#ifdef __cplusplus
}
#endif
