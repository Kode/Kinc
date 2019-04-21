#include "pch.h"

#include "Graphics.h"

using namespace Kore;
using namespace Kore::Graphics4;

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, Usage usage, int instanceDataStepRate) {
	Kinc_G4_VertexStructure kincStructure;
	Kinc_G4_VertexStructure_Create(&kincStructure);
	for (int i = 0; i < structure.size; ++i) {
		Kinc_G4_VertexStructure_Add(&kincStructure, structure.elements[i].name, (Kinc_G4_VertexData)structure.elements[i].data);
	}
	kincStructure.instanced = structure.instanced;
	Kinc_G4_VertexBuffer_Create(&kincBuffer, count, &kincStructure, (Kinc_G4_Usage)usage, instanceDataStepRate);
}

VertexBuffer::~VertexBuffer() {
	Kinc_G4_VertexBuffer_Destroy(&kincBuffer);
}

float* VertexBuffer::lock() {
	return Kinc_G4_VertexBuffer_LockAll(&kincBuffer);
}

float* VertexBuffer::lock(int start, int count) {
	return Kinc_G4_VertexBuffer_Lock(&kincBuffer, start, count);
}

void VertexBuffer::unlock() {
	Kinc_G4_VertexBuffer_UnlockAll(&kincBuffer);
}

void VertexBuffer::unlock(int count) {
	Kinc_G4_VertexBuffer_Unlock(&kincBuffer, count);
}

int VertexBuffer::count() {
	return Kinc_G4_VertexBuffer_Count(&kincBuffer);
}

int VertexBuffer::stride() {
	return Kinc_G4_VertexBuffer_Stride(&kincBuffer);
}

int VertexBuffer::_set(int offset) {
	return Kinc_Internal_G4_VertexBuffer_Set(&kincBuffer, offset);
}
