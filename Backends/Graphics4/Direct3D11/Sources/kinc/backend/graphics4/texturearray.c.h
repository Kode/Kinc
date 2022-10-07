#include <kinc/graphics4/texturearray.h>
#include <kinc/graphics4/textureunit.h>

void kinc_g4_texture_array_init(kinc_g4_texture_array_t *array, kinc_image_t *textures, int count) {
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = textures[0].width;
	desc.Height = textures[0].height;
	desc.MipLevels = 1;
	desc.ArraySize = 2;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA *resdata = (D3D11_SUBRESOURCE_DATA *)alloca(sizeof(D3D11_SUBRESOURCE_DATA) * count);
	for (int i = 0; i < count; ++i) {
		resdata[i].pSysMem = textures[i].data;
		resdata[i].SysMemPitch = textures[0].width * 4;
		resdata[i].SysMemSlicePitch = 0;
	}

	array->impl.texture = NULL;
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateTexture2D(dx_ctx.device, &desc, resdata, &array->impl.texture));
	kinc_microsoft_affirm(dx_ctx.device->lpVtbl->CreateShaderResourceView(dx_ctx.device, (ID3D11Resource *)array->impl.texture, NULL, &array->impl.view));
}

void kinc_g4_texture_array_destroy(kinc_g4_texture_array_t *array) {}

void kinc_internal_texture_array_set(kinc_g4_texture_array_t *array, kinc_g4_texture_unit_t unit) {
	if (unit.impl.unit < 0)
		return;
	dx_ctx.context->lpVtbl->PSSetShaderResources(dx_ctx.context, unit.impl.unit, 1, &array->impl.view);
	// this->stage = unit.unit;
	// setTextures[stage] = this;
}
