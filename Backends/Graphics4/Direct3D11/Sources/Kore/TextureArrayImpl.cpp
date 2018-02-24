#include "pch.h"

#include <Kore/Graphics4/TextureArray.h>
#include <Kore/WinError.h>

#include "Direct3D11.h"

#include <malloc.h>

using namespace Kore;
using namespace Kore::Graphics4;

TextureArray::TextureArray(Image** textures, int count) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = textures[0]->width;
	desc.Height = textures[0]->height;
	desc.MipLevels = 1;
	desc.ArraySize = 2;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	u8* data = new u8[textures[0]->width * textures[0]->height * 4 * count];

	D3D11_SUBRESOURCE_DATA* resdata = (D3D11_SUBRESOURCE_DATA*)alloca(sizeof(D3D11_SUBRESOURCE_DATA) * count);
	for (int i = 0; i < count; ++i) {
		resdata[i].pSysMem = textures[i]->data;
		resdata[i].SysMemPitch = textures[0]->width * 4;
		resdata[i].SysMemSlicePitch = 0;
	}
	
	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, resdata, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));
}

void TextureArrayImpl::set(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &view);
	//this->stage = unit.unit;
	//setTextures[stage] = this;
}
