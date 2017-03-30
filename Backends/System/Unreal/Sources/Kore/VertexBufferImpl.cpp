#include "pch.h"

#include "VertexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

#include <Runtime/RHI/Public/RHI.h>
#include <Runtime/RHI/Public/RHIResources.h>
#include <Runtime/RHI/Public/DynamicRHI.h>
#include <Runtime/RHI/Public/RHIStaticStates.h>

using namespace Kore;

VertexBufferImpl::VertexBufferImpl(int count, int instanceDataStepRate) : myCount(count), instanceDataStepRate(instanceDataStepRate) {}

VertexBuffer::VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate) : VertexBufferImpl(count, instanceDataStepRate) {
	myStride = 0;
	for (int i = 0; i < structure.size; ++i) {
		VertexElement element = structure.elements[i];
		switch (element.data) {
		case ColorVertexData:
			myStride += 1 * 4;
			break;
		case Float1VertexData:
			myStride += 1 * 4;
			break;
		case Float2VertexData:
			myStride += 2 * 4;
			break;
		case Float3VertexData:
			myStride += 3 * 4;
			break;
		case Float4VertexData:
			myStride += 4 * 4;
			break;
		case NoVertexData:
			break;
		}
	}

	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	FRHIResourceCreateInfo createInfo;
	vertexBuffer = GDynamicRHI->CreateVertexBuffer_RenderThread(commandList, myCount * myStride, BUF_UnorderedAccess, createInfo);
}

VertexBuffer::~VertexBuffer() {

}

float* VertexBuffer::lock() {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	return (float*)GDynamicRHI->LockVertexBuffer_RenderThread(commandList, vertexBuffer, 0, myCount * myStride, RLM_WriteOnly);
}

float* VertexBuffer::lock(int start, int count) {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	return (float*)GDynamicRHI->LockVertexBuffer_RenderThread(commandList, vertexBuffer, 0, myCount * myStride, RLM_WriteOnly);
}

void VertexBuffer::unlock() {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	GDynamicRHI->UnlockVertexBuffer_RenderThread(commandList, vertexBuffer);
}

int VertexBuffer::_set(int offset) {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	commandList.SetStreamSource(0, vertexBuffer, myStride, 0);
	return 0;
}

void VertexBufferImpl::unset() {

}

int VertexBuffer::count() {
	return myCount;
}

int VertexBuffer::stride() {
	return myStride;
}
