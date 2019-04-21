
#ifdef KORE_G4

#include "pch.h"

#include "Graphics.h"
#include "PipelineState.h"

#include <Kinc/Graphics4/Graphics.h>

#include <limits>

using namespace Kore;

namespace {
	int samples = 1;
}

int Graphics4::antialiasingSamples() {
	return ::samples;
}

void Graphics4::setAntialiasingSamples(int samples) {
	::samples = samples;
}

bool Kore::Graphics4::fullscreen = false;

void Graphics4::setFloat2(ConstantLocation position, vec2 value) {
	setFloat2(position, value.x(), value.y());
}

void Graphics4::setFloat3(ConstantLocation position, vec3 value) {
	setFloat3(position, value.x(), value.y(), value.z());
}

void Graphics4::setFloat4(ConstantLocation position, vec4 value) {
	setFloat4(position, value.x(), value.y(), value.z(), value.w());
}

void Graphics4::setVertexBuffer(VertexBuffer& vertexBuffer) {
	VertexBuffer* vertexBuffers[1] = {&vertexBuffer};
	setVertexBuffers(vertexBuffers, 1);
}

void Graphics4::setRenderTarget(RenderTarget* target) {
	setRenderTargets(&target, 1);
}

void Graphics4::setIndexBuffer(Kore::Graphics4::IndexBuffer& indexBuffer) {
	Kinc_G4_SetIndexBuffer(&indexBuffer.kincBuffer);
}

void Graphics4::setPipeline(Kore::Graphics4::PipelineState* pipeline) {
	Kinc_G4_SetPipeline(&pipeline->kincPipeline);
}

void Graphics4::drawIndexedVertices() {
	Kinc_G4_DrawIndexedVertices();
}

void Graphics4::begin(int window) {
	Kinc_G4_Begin(window);
}

void Graphics4::end(int window) {
	Kinc_G4_End(window);
}

bool Graphics4::swapBuffers() {
	return Kinc_G4_SwapBuffers();
}

void Graphics4::clear(unsigned flags, unsigned color, float depth, int stencil) {
	Kinc_G4_Clear(flags, color, depth, stencil);
}

#endif
