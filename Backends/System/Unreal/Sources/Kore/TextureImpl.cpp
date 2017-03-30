#include "pch.h"

#include "TextureImpl.h"

#include <Kore/IO/BufferReader.h>
#include <Kore/Math/Random.h>

#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "PixelShaderDeclaration.h"

using namespace Kore;

void Texture::init(const char* format, bool readable) {
	Kore::Random::init(0);

	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	FRHIResourceCreateInfo createInfo;
	_tex = commandList.CreateTexture2D(width, height, PF_B8G8R8A8, 1, 1, 0, createInfo);

	uint32 stride;
	u8* to = (u8*)commandList.LockTexture2D(_tex, 0, RLM_WriteOnly, stride, false);
	
	pitch = stride;
	u8* from = (u8*)this->data;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			to[pitch * y + x * 4 + 0 /* blue*/] = (from[y * width * 4 + x * 4 + 2]); /// 255.0f;
			to[pitch * y + x * 4 + 1 /*green*/] = (from[y * width * 4 + x * 4 + 1]); /// 255.0f;
			to[pitch * y + x * 4 + 2 /*  red*/] = (from[y * width * 4 + x * 4 + 0]); /// 255.0f;
			to[pitch * y + x * 4 + 3 /*alpha*/] = (from[y * width * 4 + x * 4 + 3]); /// 255.0f;
		}
	}

	commandList.UnlockTexture2D(_tex, 0, false);

	if (!readable) {
//		delete[] data;
//		data = nullptr;
	}
}

Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {

}

Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

TextureImpl::~TextureImpl() {

}

void Texture::_set(TextureUnit unit) {
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	TShaderMapRef<FPixelShaderDeclaration> pixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	SetTextureParameter(commandList, pixelShader->GetPixelShader(), unit.parameter, _tex);

	FSamplerStateRHIParamRef SamplerStateLinear = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	SetTextureParameter(commandList, pixelShader->GetPixelShader(), unit.parameter, unit.sampler, SamplerStateLinear, _tex);
}

void TextureImpl::unset() {

}

u8* Texture::lock() {
	return nullptr;
}

void Texture::unlock() {

}

void Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

int Texture::stride() {
	return pitch;
}

void Texture::generateMipmaps(int levels) {}

void Texture::setMipmap(Texture* mipmap, int level) {}
