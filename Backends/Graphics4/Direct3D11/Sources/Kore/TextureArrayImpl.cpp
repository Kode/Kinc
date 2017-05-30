#include "pch.h"

#include <Kore/Graphics4/TextureArray.h>
#include <Kore/WinError.h>

#include "Direct3D11.h"

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

	D3D11_SUBRESOURCE_DATA resdata;
	resdata.pSysMem = data;
	resdata.SysMemPitch = textures[0]->width * 4;
	resdata.SysMemSlicePitch = textures[0]->width * textures[0]->height * 4;

	for (int layer = 0; layer < count; ++layer) {
		for (int y = 0; y < textures[0]->height; ++y) {
			for (int x = 0; x < textures[0]->width; ++x) {
				data[resdata.SysMemSlicePitch * layer + resdata.SysMemPitch * y + x * 4 + 0] = textures[layer]->data[textures[0]->width * 4 * y + x * 4 + 0];
				data[resdata.SysMemSlicePitch * layer + resdata.SysMemPitch * y + x * 4 + 1] = textures[layer]->data[textures[0]->width * 4 * y + x * 4 + 1];
				data[resdata.SysMemSlicePitch * layer + resdata.SysMemPitch * y + x * 4 + 2] = textures[layer]->data[textures[0]->width * 4 * y + x * 4 + 2];
				data[resdata.SysMemSlicePitch * layer + resdata.SysMemPitch * y + x * 4 + 3] = textures[layer]->data[textures[0]->width * 4 * y + x * 4 + 3];
			}
		}
	}

	texture = nullptr;
	affirm(device->CreateTexture2D(&desc, &resdata, &texture));
	affirm(device->CreateShaderResourceView(texture, nullptr, &view));

	delete[] data;
}

void TextureArrayImpl::set(TextureUnit unit) {
	if (unit.unit < 0) return;
	context->PSSetShaderResources(unit.unit, 1, &view);
	//this->stage = unit.unit;
	//setTextures[stage] = this;
}
