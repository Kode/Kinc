#include "pch.h"

#include <kinc/display.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/math/core.h>
#undef CreateWindow
#include <kinc/system.h>
#include <kinc/window.h>

#include <Kore/SystemMicrosoft.h>
#include <Kore/Windows.h>

#include <Kore/Log.h>

#include <vector>

#include "Direct3D9.h"

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 device;

namespace {
	HWND hWnd;

	int _width;
	int _height;

	unsigned hz;
	bool vsync;

	bool resizable;

	D3DVIEWPORT9 vp;

	bool swapBuffers() {
		HRESULT result;
		if (resizable) {
			RECT vRect;
			GetClientRect(hWnd, &vRect);
			result = device->Present(&vRect, &vRect, 0, 0);
		}
		else {
			result = device->Present(0, 0, 0, 0);
		}
		return result != D3DERR_DEVICELOST;
	}

	kinc_g4_shader_t *pixelShader = nullptr;
	kinc_g4_shader_t *vertexShader = nullptr;
	IDirect3DSurface9 *backBuffer = nullptr;
	IDirect3DSurface9 *depthBuffer = nullptr;

	void initDeviceStates() {
		D3DCAPS9 caps;
		device->GetDeviceCaps(&caps);

		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
//	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
#ifndef USE_SHADER
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
#endif
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

#ifndef USE_SHADER
		device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
		kinc_microsoft_affirm(device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
		kinc_microsoft_affirm(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
#endif
		// if (d3dpp.Windowed != TRUE) Cursor->Hide();

		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		for (int i = 0; i < 16; ++i) {
			device->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
			device->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			device->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
			device->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, caps.MaxAnisotropy);
		}

		device->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(D3DVERTEXTEXTURESAMPLER0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);

		device->SetRenderState(D3DRS_ZENABLE, FALSE);

		device->Clear(0, 0, D3DCLEAR_TARGET, 0, 0, 0);
	}
}

void kinc_g4_destroy(int window) {}

extern "C" void kinc_internal_resize(int width, int height) {
	if (!resizable) {
		return;
	}

	_width = width;
	_height = height;
	kinc_g4_viewport(0, 0, width, height);
	/*D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = (!fullscreen) ? TRUE : FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferCount = 2;
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; //D3DPRESENT_INTERVAL_IMMEDIATE;
	if (antialiasing()) {
	    if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, D3DMULTISAMPLE_4_SAMPLES, nullptr)))
	        d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	    if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, D3DMULTISAMPLE_8_SAMPLES, nullptr)))
	        d3dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
	}
	else {
	    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	}

	device->Reset(&d3dpp);

	initDeviceStates();*/
}

extern "C" void kinc_internal_change_framebuffer(int window, struct kinc_framebuffer_options *frame) {}

void kinc_g4_init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync) {
	bool fullscreen = kinc_window_get_mode(windowId) == KINC_WINDOW_MODE_FULLSCREEN || kinc_window_get_mode(windowId) == KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN;

	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	hWnd = kinc_windows_window_handle(windowId);
	long style = GetWindowLong(hWnd, GWL_STYLE);

	resizable = false;

	if ((style & WS_SIZEBOX) != 0) {
		resizable = true;
	}

	if ((style & WS_MAXIMIZEBOX) != 0) {
		resizable = true;
	}

	// TODO (DK) just setup the primary window for now and ignore secondaries
	//	-this should probably be implemented via swap chain for real at a later time
	//  -http://www.mvps.org/directx/articles/rendering_to_multiple_windows.htm
	if (windowId > 0) {
		return;
	}

#ifdef KORE_WINDOWS
	// TODO (DK) convert depthBufferBits + stencilBufferBits to: d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = (!fullscreen) ? TRUE : FALSE;

	if (resizable) {
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferCount = 1;
		kinc_display_mode_t mode = kinc_display_current_mode(kinc_primary_display());
		d3dpp.BackBufferWidth = mode.width;
		d3dpp.BackBufferHeight = mode.height;
	}
	else {
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferCount = 2;
		d3dpp.BackBufferWidth = kinc_window_width(windowId);
		d3dpp.BackBufferHeight = kinc_window_height(windowId);
	}

	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	// d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	if (kinc_g4_antialiasing_samples() > 1) {
		for (int samples = min(kinc_g4_antialiasing_samples(), 16); samples > 1; --samples) {
			if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, (D3DMULTISAMPLE_TYPE)samples, nullptr))) {
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)samples;
				break;
			}
		}
	}
#endif

	if (!SUCCEEDED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device)))
		d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);
		// d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);

#ifdef KORE_WINDOWS
	// if (System::hasShowWindowFlag(/*windowId*/)) {
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	//}
#endif

	initDeviceStates();

#ifdef KORE_WINDOWS
	if (fullscreen) {
		// hz = d3dpp.FullScreen_RefreshRateInHz;
		D3DDISPLAYMODE mode;
		device->GetDisplayMode(0, &mode);
		hz = mode.RefreshRate;
	}
	if (!fullscreen || hz == 0) {
		DEVMODE devMode;
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
		hz = devMode.dmDisplayFrequency;
	}
#endif

	// vsync = d3dpp.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE;

	kinc_ticks_t test1 = kinc_timestamp();
	for (int i = 0; i < 3; ++i) {
		kinc_g4_swap_buffers();
	}
	kinc_ticks_t test2 = kinc_timestamp();
	if (test2 - test1 < (1.0 / hz) * kinc_frequency()) {
		vsync = false;
	}
	else {
		vsync = true;
	}

	_width = kinc_window_width(windowId);
	_height = kinc_window_height(windowId);
}

void kinc_g4_flush() {}

namespace {
	DWORD convertFilter(kinc_g4_texture_filter_t filter) {
		switch (filter) {
		case KINC_G4_TEXTURE_FILTER_POINT:
			return D3DTEXF_POINT;
		case KINC_G4_TEXTURE_FILTER_LINEAR:
			return D3DTEXF_LINEAR;
		case KINC_G4_TEXTURE_FILTER_ANISOTROPIC:
			return D3DTEXF_ANISOTROPIC;
		default:
			return D3DTEXF_POINT;
		}
	}

	DWORD convertMipFilter(kinc_g4_mipmap_filter_t filter) {
		switch (filter) {
		case KINC_G4_MIPMAP_FILTER_NONE:
			return D3DTEXF_NONE;
		case KINC_G4_MIPMAP_FILTER_POINT:
			return D3DTEXF_POINT;
		case KINC_G4_MIPMAP_FILTER_LINEAR:
			return D3DTEXF_LINEAR;
		default:
			return D3DTEXF_NONE;
		}
	}

	_D3DTEXTUREOP convert(kinc_g4_texture_operation_t operation) {
		switch (operation) {
		case KINC_G4_TEXTURE_OPERATION_MODULATE:
			return D3DTOP_MODULATE;
		case KINC_G4_TEXTURE_OPERATION_SELECT_FIRST:
			return D3DTOP_SELECTARG1;
		case KINC_G4_TEXTURE_OPERATION_SELECT_SECOND:
			return D3DTOP_SELECTARG2;
		default:
			//	throw Exception("Unknown texture operation.");
			return D3DTOP_MODULATE;
		}
	}

	int convert(kinc_g4_texture_argument_t arg) {
		switch (arg) {
		case KINC_G4_TEXTURE_ARGUMENT_CURRENT_COLOR:
			return D3DTA_CURRENT;
		case KINC_G4_TEXTURE_ARGUMENT_TEXTURE_COLOR:
			return D3DTA_TEXTURE;
		default:
			//	throw Exception("Unknown texture argument.");
			return D3DTA_CURRENT;
		}
	}
}

void kinc_g4_set_texture_operation(kinc_g4_texture_operation_t operation, kinc_g4_texture_argument_t arg1, kinc_g4_texture_argument_t arg2) {
	device->SetTextureStageState(0, D3DTSS_COLOROP, convert(operation));
	device->SetTextureStageState(0, D3DTSS_COLORARG1, convert(arg1));
	device->SetTextureStageState(0, D3DTSS_COLORARG2, convert(arg2));
}

void kinc_g4_set_texture_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	device->SetSamplerState(texunit.impl.unit, D3DSAMP_MAGFILTER, convertFilter(filter));
}

void kinc_g4_set_texture3d_magnification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_magnification_filter(texunit, filter);
}

void kinc_g4_set_texture_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	device->SetSamplerState(texunit.impl.unit, D3DSAMP_MINFILTER, convertFilter(filter));
}

void kinc_g4_set_texture3d_minification_filter(kinc_g4_texture_unit_t texunit, kinc_g4_texture_filter_t filter) {
	kinc_g4_set_texture_minification_filter(texunit, filter);
}

void kinc_g4_set_texture_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	device->SetSamplerState(texunit.impl.unit, D3DSAMP_MIPFILTER, convertMipFilter(filter));
}

void kinc_g4_set_texture3d_mipmap_filter(kinc_g4_texture_unit_t texunit, kinc_g4_mipmap_filter_t filter) {
	kinc_g4_set_texture_mipmap_filter(texunit, filter);
}

void kinc_g4_set_texture_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {}

void kinc_g4_set_cubemap_compare_mode(kinc_g4_texture_unit_t unit, bool enabled) {}

void kinc_g4_set_render_targets(struct kinc_g4_render_target **targets, int count) {
	// if (backBuffer != nullptr) backBuffer->Release();

	if (backBuffer == nullptr) {
		device->GetRenderTarget(0, &backBuffer);
		device->GetDepthStencilSurface(&depthBuffer);
	}
	kinc_microsoft_affirm(device->SetDepthStencilSurface(targets[0]->impl.depthSurface));
	for (int i = 0; i < count; ++i) {
		kinc_microsoft_affirm(device->SetRenderTarget(i, targets[i]->impl.colorSurface));
	}
}

void kinc_g4_set_render_target_face(struct kinc_g4_render_target *texture, int face) {}

// void Graphics::setDepthStencilTarget(Texture* texture) {
//	//if (depthBuffer != nullptr) depthBuffer->Release();
//	device->GetDepthStencilSurface(&depthBuffer);
//	Microsoft::affirm(device->SetDepthStencilSurface(dcast<D3D9Texture*>(texture)->getSurface()));
//}

void kinc_g4_restore_render_target() {
	if (backBuffer != nullptr) {
		device->SetRenderTarget(0, backBuffer);
		device->SetRenderTarget(1, nullptr);
		backBuffer->Release();
		backBuffer = nullptr;
		device->SetDepthStencilSurface(depthBuffer);
		depthBuffer->Release();
		depthBuffer = nullptr;
		kinc_g4_viewport(0, 0, _width, _height);
	}
}

void kinc_g4_draw_indexed_vertices() {
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, kinc_g4_vertex_buffer_count(kinc_internal_current_vertex_buffer), 0,
	                             kinc_g4_index_buffer_count(kinc_internal_current_index_buffer) / 3);
}

void kinc_g4_draw_indexed_vertices_from_to(int start, int count) {
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, kinc_g4_vertex_buffer_count(kinc_internal_current_vertex_buffer), start, count / 3);
}

void kinc_g4_draw_indexed_vertices_instanced(int instanceCount) {
	kinc_microsoft_affirm(device->SetStreamSourceFreq(kinc_internal_current_vertex_buffer->impl._offset, (D3DSTREAMSOURCE_INDEXEDDATA | instanceCount)));
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, kinc_g4_vertex_buffer_count(kinc_internal_current_vertex_buffer), 0,
	                             kinc_g4_index_buffer_count(kinc_internal_current_index_buffer) / 3);
}

void kinc_g4_draw_indexed_vertices_instanced_from_to(int instanceCount, int start, int count) {
	kinc_microsoft_affirm(device->SetStreamSourceFreq(kinc_internal_current_vertex_buffer->impl._offset, (D3DSTREAMSOURCE_INDEXEDDATA | instanceCount)));
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, kinc_g4_vertex_buffer_count(kinc_internal_current_vertex_buffer), start, count / 3);
}

void kinc_g4_set_texture_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	DWORD value = 0;
	switch (addressing) {
	case KINC_G4_TEXTURE_ADDRESSING_REPEAT:
		value = D3DTADDRESS_WRAP;
		break;
	case KINC_G4_TEXTURE_ADDRESSING_MIRROR:
		value = D3DTADDRESS_MIRROR;
		break;
	case KINC_G4_TEXTURE_ADDRESSING_CLAMP:
		value = D3DTADDRESS_CLAMP;
		break;
	case KINC_G4_TEXTURE_ADDRESSING_BORDER:
		value = D3DTADDRESS_BORDER;
		break;
	}
	device->SetSamplerState(unit.impl.unit, dir == KINC_G4_TEXTURE_DIRECTION_U ? D3DSAMP_ADDRESSU : D3DSAMP_ADDRESSV, value);
}

void kinc_g4_set_texture3d_addressing(kinc_g4_texture_unit_t unit, kinc_g4_texture_direction_t dir, kinc_g4_texture_addressing_t addressing) {
	kinc_g4_set_texture_addressing(unit, dir, addressing);
}

namespace {
	void tod3dmatrix(kinc_matrix4x4_t *matrix, D3DMATRIX &d3dm) {
		d3dm._11 = kinc_matrix4x4_get(matrix, 0, 0);
		d3dm._12 = kinc_matrix4x4_get(matrix, 0, 1);
		d3dm._13 = kinc_matrix4x4_get(matrix, 0, 2);
		d3dm._14 = kinc_matrix4x4_get(matrix, 0, 3);

		d3dm._21 = kinc_matrix4x4_get(matrix, 1, 0);
		d3dm._22 = kinc_matrix4x4_get(matrix, 1, 1);
		d3dm._23 = kinc_matrix4x4_get(matrix, 1, 2);
		d3dm._24 = kinc_matrix4x4_get(matrix, 1, 3);

		d3dm._31 = kinc_matrix4x4_get(matrix, 2, 0);
		d3dm._32 = kinc_matrix4x4_get(matrix, 2, 1);
		d3dm._33 = kinc_matrix4x4_get(matrix, 2, 2);
		d3dm._34 = kinc_matrix4x4_get(matrix, 2, 3);

		d3dm._41 = kinc_matrix4x4_get(matrix, 3, 0);
		d3dm._42 = kinc_matrix4x4_get(matrix, 3, 1);
		d3dm._43 = kinc_matrix4x4_get(matrix, 3, 2);
		d3dm._44 = kinc_matrix4x4_get(matrix, 3, 3);
	}
}

void kinc_g4_clear(unsigned flags, unsigned color, float depth, int stencil) {
	device->Clear(0, nullptr, flags, color, depth, stencil);
}

void kinc_g4_begin(int window) {
	// TODO (DK) ignore secondary windows for now
	if (window > 0) {
		return;
	}

	kinc_g4_viewport(0, 0, _width, _height);
	device->BeginScene();
}

void kinc_g4_viewport(int x, int y, int width, int height) {
	vp.X = x;
	vp.Y = y;
	vp.Width = width;
	vp.Height = height;
	device->SetViewport(&vp);
}

void kinc_g4_scissor(int x, int y, int width, int height) {
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);

	RECT rc;
	rc.left = x;
	rc.top = y;
	rc.right = x + width;
	rc.bottom = y + height;
	device->SetScissorRect(&rc);
}

void kinc_g4_disable_scissor() {
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
}

void kinc_g4_end(int window) {
	// TODO (DK) ignore secondary windows for now
	if (window > 0) {
		return;
	}

	/*if (backBuffer != nullptr) {
	    backBuffer->Release();
	    backBuffer = nullptr;
	}*/
	device->EndScene();
}

bool kinc_window_vsynced(int window) {
	return vsync;
}

// unsigned Graphics4::refreshRate() {
//	return hz;
//}

bool kinc_g4_swap_buffers() {
	return ::swapBuffers();
}

void kinc_g4_set_stencil_reference_value(int value) {}

void kinc_g4_set_bool(kinc_g4_constant_location_t position, bool value) {
	if (position.impl.shaderType == -1) return;
	BOOL bools[4];
	bools[0] = value ? 1 : 0;
	bools[1] = bools[0];
	bools[2] = bools[0];
	bools[3] = bools[0];
	if (position.impl.shaderType == 0)
		device->SetVertexShaderConstantB(position.impl.reg.regindex, &bools[0], 1);
	else
		device->SetPixelShaderConstantB(position.impl.reg.regindex, &bools[0], 1);
}

void kinc_g4_set_int(kinc_g4_constant_location_t position, int value) {
	if (position.impl.shaderType == -1) return;
	if (position.impl.reg.regtype == 'f') {
		kinc_g4_set_float(position, (float)value);
	}
	else {
		int ints[4];
		ints[0] = value;
		ints[1] = value;
		ints[2] = value;
		ints[3] = value;
		if (position.impl.shaderType == 0)
			device->SetVertexShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
		else
			device->SetPixelShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
	}
}

void kinc_g4_set_int2(kinc_g4_constant_location_t position, int value1, int value2) {
	if (position.impl.shaderType == -1) return;
	if (position.impl.reg.regtype == 'f') {
		kinc_g4_set_float2(position, (float)value1, (float)value2);
	}
	else {
		int ints[4];
		ints[0] = value1;
		ints[1] = value2;
		ints[2] = value1;
		ints[3] = value2;
		if (position.impl.shaderType == 0)
			device->SetVertexShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
		else
			device->SetPixelShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
	}
}

void kinc_g4_set_int3(kinc_g4_constant_location_t position, int value1, int value2, int value3) {
	if (position.impl.shaderType == -1) return;
	if (position.impl.reg.regtype == 'f') {
		kinc_g4_set_float3(position, (float)value1, (float)value2, (float)value3);
	}
	else {
		int ints[4];
		ints[0] = value1;
		ints[1] = value2;
		ints[2] = value3;
		ints[3] = value1;
		if (position.impl.shaderType == 0)
			device->SetVertexShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
		else
			device->SetPixelShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
	}
}

void kinc_g4_set_int4(kinc_g4_constant_location_t position, int value1, int value2, int value3, int value4) {
	if (position.impl.shaderType == -1) return;
	if (position.impl.reg.regtype == 'f') {
		kinc_g4_set_float4(position, (float)value1, (float)value2, (float)value3, (float)value4);
	}
	else {
		int ints[4];
		ints[0] = value1;
		ints[1] = value2;
		ints[2] = value3;
		ints[3] = value4;
		if (position.impl.shaderType == 0)
			device->SetVertexShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
		else
			device->SetPixelShaderConstantI(position.impl.reg.regindex, &ints[0], 1);
	}
}

void kinc_g4_set_ints(kinc_g4_constant_location_t location, int *values, int count) {
	if (location.impl.shaderType == -1) return;
	int registerCount = (count + 3) / 4; // round up
	if (registerCount == count / 4) {    // round down
		if (location.impl.shaderType == 0)
			device->SetVertexShaderConstantI(location.impl.reg.regindex, values, registerCount);
		else
			device->SetPixelShaderConstantI(location.impl.reg.regindex, values, registerCount);
	}
	else {
		int *data = (int *)alloca(registerCount * 4 * sizeof(int));
		memcpy(data, values, count * sizeof(int));
		if (location.impl.shaderType == 0)
			device->SetVertexShaderConstantI(location.impl.reg.regindex, data, registerCount);
		else
			device->SetPixelShaderConstantI(location.impl.reg.regindex, data, registerCount);
	}
}

void kinc_g4_set_float(kinc_g4_constant_location_t position, float value) {
	if (position.impl.shaderType == -1) return;
	float floats[4];
	floats[0] = value;
	floats[1] = value;
	floats[2] = value;
	floats[3] = value;
	if (position.impl.shaderType == 0)
		device->SetVertexShaderConstantF(position.impl.reg.regindex, floats, 1);
	else
		device->SetPixelShaderConstantF(position.impl.reg.regindex, floats, 1);
}

void kinc_g4_set_float2(kinc_g4_constant_location_t position, float value1, float value2) {
	if (position.impl.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value1;
	floats[3] = value2;
	if (position.impl.shaderType == 0)
		device->SetVertexShaderConstantF(position.impl.reg.regindex, floats, 1);
	else
		device->SetPixelShaderConstantF(position.impl.reg.regindex, floats, 1);
}

void kinc_g4_set_float3(kinc_g4_constant_location_t position, float value1, float value2, float value3) {
	if (position.impl.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value1;
	if (position.impl.shaderType == 0)
		device->SetVertexShaderConstantF(position.impl.reg.regindex, floats, 1);
	else
		device->SetPixelShaderConstantF(position.impl.reg.regindex, floats, 1);
}

void kinc_g4_set_float4(kinc_g4_constant_location_t position, float value1, float value2, float value3, float value4) {
	if (position.impl.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
	if (position.impl.shaderType == 0)
		device->SetVertexShaderConstantF(position.impl.reg.regindex, floats, 1);
	else
		device->SetPixelShaderConstantF(position.impl.reg.regindex, floats, 1);
}

void kinc_g4_set_floats(kinc_g4_constant_location_t location, float *values, int count) {
	if (location.impl.shaderType == -1) return;
	int registerCount = (count + 3) / 4; // round up
	if (registerCount == count / 4) {    // round down
		if (location.impl.shaderType == 0)
			device->SetVertexShaderConstantF(location.impl.reg.regindex, values, registerCount);
		else
			device->SetPixelShaderConstantF(location.impl.reg.regindex, values, registerCount);
	}
	else {
		float *data = (float *)alloca(registerCount * 4 * sizeof(float));
		memcpy(data, values, count * sizeof(float));
		if (location.impl.shaderType == 0)
			device->SetVertexShaderConstantF(location.impl.reg.regindex, data, registerCount);
		else
			device->SetPixelShaderConstantF(location.impl.reg.regindex, data, registerCount);
	}
}

void kinc_g4_set_matrix4(kinc_g4_constant_location_t location, kinc_matrix4x4_t *value) {
	if (location.impl.shaderType == -1) return;
	float floats[16];
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[y * 4 + x] = kinc_matrix4x4_get(value, y, x);
		}
	}
	if (location.impl.shaderType == 0)
		device->SetVertexShaderConstantF(location.impl.reg.regindex, floats, 4);
	else
		device->SetPixelShaderConstantF(location.impl.reg.regindex, floats, 4);
}

void kinc_g4_set_matrix3(kinc_g4_constant_location_t location, kinc_matrix3x3_t *value) {
	if (location.impl.shaderType == -1) return;
	float floats[12];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[y * 4 + x] = kinc_matrix3x3_get(value, y, x);
		}
	}
	if (location.impl.shaderType == 0)
		device->SetVertexShaderConstantF(location.impl.reg.regindex, floats, 3);
	else
		device->SetPixelShaderConstantF(location.impl.reg.regindex, floats, 3);
}

bool kinc_g4_render_targets_inverted_y() {
	return false;
}

bool kinc_g4_non_pow2_textures_supported() {
	return true;
}

void kinc_g4_set_vertex_buffers(kinc_g4_vertex_buffer_t **buffers, int count) {
	for (int i = 0; i < count; ++i) {
		kinc_internal_g4_vertex_buffer_set(buffers[i], i);
	}
}

void kinc_g4_set_index_buffer(kinc_g4_index_buffer_t *buffer) {
	kinc_internal_g4_index_buffer_set(buffer);
}

void kinc_internal_texture_set(kinc_g4_texture_t *texture, kinc_g4_texture_unit_t unit);

void kinc_g4_set_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture) {
	kinc_internal_texture_set(texture, unit);
}

void kinc_g4_set_image_texture(kinc_g4_texture_unit_t unit, struct kinc_g4_texture *texture) {}

unsigned queryCount = 0;
std::vector<IDirect3DQuery9 *> queryPool;

bool kinc_g4_init_occlusion_query(unsigned *occlusionQuery) {
	// check if the runtime supports queries
	HRESULT result = device->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL);
	if (FAILED(result)) {
		Kore::log(Kore::LogLevel::Warning, "Internal query creation failed, result: 0x%X.", result);
		return false;
	}

	IDirect3DQuery9 *pQuery = nullptr;
	device->CreateQuery(D3DQUERYTYPE_OCCLUSION, &pQuery);

	queryPool.push_back(pQuery);
	*occlusionQuery = queryCount;
	++queryCount;

	return true;
}

void kinc_g4_delete_occlusion_query(unsigned occlusionQuery) {
	if (occlusionQuery < queryPool.size()) queryPool[occlusionQuery] = nullptr;
}

void kinc_g4_start_occlusion_query(unsigned occlusionQuery) {
	IDirect3DQuery9 *pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		pQuery->Issue(D3DISSUE_BEGIN);
	}
}

void kinc_g4_end_occlusion_query(unsigned occlusionQuery) {
	IDirect3DQuery9 *pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		pQuery->Issue(D3DISSUE_END);
	}
}

bool kinc_g4_are_query_results_available(unsigned occlusionQuery) {
	IDirect3DQuery9 *pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		if (S_OK == pQuery->GetData(0, 0, 0)) {
			return true;
		}
	}
	return false;
}

void kinc_g4_get_query_results(unsigned occlusionQuery, unsigned *pixelCount) {
	IDirect3DQuery9 *pQuery = queryPool[occlusionQuery];
	if (pQuery != nullptr) {
		DWORD numberOfPixelsDrawn;
		HRESULT result = pQuery->GetData(&numberOfPixelsDrawn, sizeof(DWORD), 0);
		if (S_OK == result) {
			*pixelCount = numberOfPixelsDrawn;
		}
		else {
			Kore::log(Kore::LogLevel::Warning, "Check first if results are available");
			*pixelCount = 0;
		}
	}
}

void kinc_g4_set_texture_array(kinc_g4_texture_unit_t unit, struct kinc_g4_texture_array *array) {}

void kinc_g4_set_pipeline(struct kinc_g4_pipeline *pipeline) {
	kinc_g4_internal_set_pipeline(pipeline);
}
