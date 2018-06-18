#include "pch.h"

#include "IndexBufferImpl.h"

#include <Kore/Graphics/Graphics.h>

#include <Runtime/RHI/Public/DynamicRHI.h>
#include <Runtime/RHI/Public/RHI.h>
#include <Runtime/RHI/Public/RHIResources.h>
#include <Runtime/RHI/Public/RHIStaticStates.h>

using namespace Kore;

IndexBuffer* IndexBufferImpl::_current = nullptr;

IndexBufferImpl::IndexBufferImpl(int count) : myCount(count) {}

IndexBuffer::IndexBuffer(int count) : IndexBufferImpl(count) {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	FRHIResourceCreateInfo createInfo;
	indexBuffer = GDynamicRHI->CreateIndexBuffer_RenderThread(commandList, 4, count * 4, BUF_UnorderedAccess, createInfo);
}

IndexBuffer::~IndexBuffer() {}

int* IndexBuffer::lock() {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	return (int*)GDynamicRHI->LockIndexBuffer_RenderThread(commandList, indexBuffer, 0, count() * 4, RLM_WriteOnly);
}

void IndexBuffer::unlock() {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	GDynamicRHI->UnlockIndexBuffer_RenderThread(commandList, indexBuffer);
}

void IndexBuffer::_set() {
	_current = this;
}

int IndexBuffer::count() {
	return myCount;
}
