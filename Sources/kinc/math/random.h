#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void Kinc_Random_Init(int seed);
int Kinc_Random_Get();
int Kinc_Random_GetMax(int max);
int Kinc_Random_GetIn(int min, int max);

#ifdef __cplusplus
}
#endif
