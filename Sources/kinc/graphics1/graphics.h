#pragma once

#include <kinc/color.h>

#ifdef __cplusplus
extern "C" {
#endif

void Kinc_G1_Init(int width, int height);
void Kinc_G1_Begin();
void Kinc_G1_End();
void Kinc_G1_SetPixel(int x, int y, float red, float green, float blue);
int Kinc_G1_Width();
int Kinc_G1_Height();

#ifdef __cplusplus
}
#endif
