#include "pch.h"
#include <Kore/Math/Core.h>
#include "Direct3D9.h"
#include <Kore/Graphics/Shader.h>
#undef CreateWindow
#include <Kore/System.h>
#include <Kore/WinError.h>

#include <Kore/Log.h>

using namespace Kore;

#ifdef SYS_XBOX360
#define USE_SHADER
#endif

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

	void swapBuffers() {
		if( resizable ){
			RECT vRect;
			GetClientRect(hWnd, &vRect);
			device->Present(&vRect, &vRect, 0, 0);
		} else {
			device->Present(0, 0, 0, 0);
		}
	}

	Shader* pixelShader = nullptr;
	Shader* vertexShader = nullptr;
	IDirect3DSurface9* backBuffer = nullptr;
	IDirect3DSurface9* depthBuffer = nullptr;

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
		affirm(device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE));
		affirm(device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE));
	#endif
		//if (d3dpp.Windowed != TRUE) Cursor->Hide();
	
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

void Graphics::destroy(int windowId) {

}

void Graphics::changeResolution(int width, int height) {
	if(!resizable){
		return;
	}
	
	_width = width;
	_height = height;
	viewport(0, 0, width, height);
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

void Graphics::setup() {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	//if (!d3d) throw Exception("Could not initialize Direct3D9");
}

void Graphics::init(int windowId, int depthBufferBits, int stencilBufferBits) {
	if (!hasWindow()) return;

	hWnd = (HWND)System::windowHandle(windowId);
	long style = GetWindowLong(hWnd, GWL_STYLE);
	
	resizable = false;
	
	if((style & WS_SIZEBOX) != 0){
		resizable = true;
	}
	
	if((style & WS_MAXIMIZEBOX) != 0){
		resizable = true;
	}

	// TODO (DK) just setup the primary window for now and ignore secondaries
	//	-this should probably be implemented via swap chain for real at a later time
	//  -http://www.mvps.org/directx/articles/rendering_to_multiple_windows.htm
	if (windowId > 0) {
		return;
	}

#ifdef SYS_WINDOWS
	// TODO (DK) convert depthBufferBits + stencilBufferBits to: d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = (!fullscreen) ? TRUE : FALSE;
	
	if(resizable) {
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferWidth = Kore::System::desktopWidth();
		d3dpp.BackBufferHeight = Kore::System::desktopHeight();
	}else{
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferCount = 2;
		d3dpp.BackBufferWidth = System::windowWidth(windowId);
		d3dpp.BackBufferHeight = System::windowHeight(windowId);
	}
	
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	if (antialiasingSamples() > 1) {
		for (int samples = min(antialiasingSamples(), 16); samples > 1; --samples) {
			if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_A8R8G8B8, FALSE, (D3DMULTISAMPLE_TYPE)samples, nullptr))) {
				d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)samples;
				break;
			}
		}
	}
#endif

#ifdef SYS_XBOX360
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	XVIDEO_MODE VideoMode;
	XGetVideoMode( &VideoMode );
	//g_bWidescreen = VideoMode.fIsWideScreen;
	d3dpp.BackBufferWidth        = min(VideoMode.dwDisplayWidth, 1280);
	d3dpp.BackBufferHeight       = min(VideoMode.dwDisplayHeight, 720);
	d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount        = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
#endif
	
#ifdef SYS_XBOX360
	d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device);
#else
	if (!SUCCEEDED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device)))
		d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);
	//d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);
#endif

#ifdef SYS_WINDOWS
	if (System::hasShowWindowFlag(/*windowId*/)) {
		ShowWindow(hWnd, SW_SHOWDEFAULT);
		UpdateWindow(hWnd);
	}
#endif

	initDeviceStates();

#ifdef SYS_WINDOWS
	if (fullscreen) {
		//hz = d3dpp.FullScreen_RefreshRateInHz;
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
#ifdef SYS_XBOX360
	hz = 60;
#endif
	//vsync = d3dpp.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE;

	System::ticks test1 = Kore::System::timestamp();
	for (int i = 0; i < 3; ++i) swapBuffers(windowId);
	System::ticks test2 = Kore::System::timestamp();
	if (test2 - test1 < (1.0 / hz) * System::frequency()) {
		vsync = false;
	}
	else vsync = true;
	
#ifdef USE_SHADER
	vertexShader = new Shader("standard", SHADER_VERTEX);
	pixelShader = new Shader("standard", SHADER_FRAGMENT);

	vertexShader->set();
	pixelShader->set();

	setFragmentBool(L"lighting", false);
#endif
	
	_width = System::windowWidth(windowId);
	_height =  System::windowHeight(windowId);
	
	System::makeCurrent(windowId);
}

void Graphics::flush() {

}

namespace {
	DWORD convertFilter(TextureFilter filter) {
		switch (filter) {
		case PointFilter:
			return D3DTEXF_POINT;
		case LinearFilter:
			return D3DTEXF_LINEAR;
		case AnisotropicFilter:
			return D3DTEXF_ANISOTROPIC;
		default:
			return D3DTEXF_POINT;
		}
	}

	DWORD convertMipFilter(MipmapFilter filter) {
		switch (filter) {
		case NoMipFilter:
			return D3DTEXF_NONE;
		case PointMipFilter:
			return D3DTEXF_POINT;
		case LinearMipFilter:
			return D3DTEXF_LINEAR;
		default:
			return D3DTEXF_NONE;
		}
	}

	_D3DTEXTUREOP convert(TextureOperation operation) {
		switch (operation) {
		case ModulateOperation:
			return D3DTOP_MODULATE;
		case SelectFirstOperation:
			return D3DTOP_SELECTARG1;
		case SelectSecondOperation:
			return D3DTOP_SELECTARG2;
		default:
		//	throw Exception("Unknown texture operation.");
			return D3DTOP_MODULATE;
		}
	}

	int convert(TextureArgument arg) {
		switch (arg) {
		case CurrentColorArgument:
			return D3DTA_CURRENT;
		case TextureColorArgument:
			return D3DTA_TEXTURE;
		default:
		//	throw Exception("Unknown texture argument.");
			return D3DTA_CURRENT;
		}
	}
}

void Graphics::setColorMask(bool red, bool green, bool blue, bool alpha) {
	DWORD flags = 0;
	if (red) flags |= D3DCOLORWRITEENABLE_RED;
	if (green) flags |= D3DCOLORWRITEENABLE_GREEN;
	if (blue) flags |= D3DCOLORWRITEENABLE_BLUE;
	if (alpha) flags |= D3DCOLORWRITEENABLE_ALPHA;

	device->SetRenderState(D3DRS_COLORWRITEENABLE, flags);
}

void Graphics::setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2) {
	device->SetTextureStageState(0, D3DTSS_COLOROP, convert(operation));
	device->SetTextureStageState(0, D3DTSS_COLORARG1, convert(arg1));
	device->SetTextureStageState(0, D3DTSS_COLORARG2, convert(arg2));
}

void Graphics::setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter) {
	device->SetSamplerState(texunit.unit, D3DSAMP_MAGFILTER, convertFilter(filter));
}

void Graphics::setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter) {
	device->SetSamplerState(texunit.unit, D3DSAMP_MINFILTER, convertFilter(filter));
}

void Graphics::setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter) {
	device->SetSamplerState(texunit.unit, D3DSAMP_MIPFILTER, convertMipFilter(filter));
}

void Graphics::makeCurrent( int contextId ) {
	// TODO (DK) implement me
}

void Graphics::clearCurrent() {
	// TODO (DK) implement me
}

void Graphics::setRenderTarget(RenderTarget* target, int num, int additionalTargets) {
	//if (backBuffer != nullptr) backBuffer->Release();
	
	System::makeCurrent(target->contextId);

	if (num == 0) {
		if (backBuffer == nullptr) {
			device->GetRenderTarget(0, &backBuffer);
			device->GetDepthStencilSurface(&depthBuffer);
		}
		affirm(device->SetDepthStencilSurface(target->depthSurface));
	}
	affirm(device->SetRenderTarget(num, target->colorSurface));
}

//void Graphics::setDepthStencilTarget(Texture* texture) {
//	//if (depthBuffer != nullptr) depthBuffer->Release();
//	device->GetDepthStencilSurface(&depthBuffer);
//	affirm(device->SetDepthStencilSurface(dcast<D3D9Texture*>(texture)->getSurface()));
//}

void Graphics::restoreRenderTarget() {
	if (backBuffer != nullptr) {
		device->SetRenderTarget(0, backBuffer);
		device->SetRenderTarget(1, nullptr);
		backBuffer->Release();
		backBuffer = nullptr;
		device->SetDepthStencilSurface(depthBuffer);
		depthBuffer->Release();
		depthBuffer = nullptr;
		viewport(0, 0, _width, _height);
	}
}

void Graphics::drawIndexedVertices() {
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexBuffer::_current->count(), 0, IndexBuffer::_current->count() / 3);
}

void Graphics::drawIndexedVertices(int start, int count) {
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexBuffer::_current->count(), start, count / 3);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount) {
	affirm(device->SetStreamSourceFreq(VertexBuffer::_current->_offset, (D3DSTREAMSOURCE_INDEXEDDATA | instanceCount)));
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexBuffer::_current->count(), 0, IndexBuffer::_current->count() / 3 * instanceCount);
}

void Graphics::drawIndexedVerticesInstanced(int instanceCount, int start, int count) {
	affirm(device->SetStreamSourceFreq(VertexBuffer::_current->_offset, (D3DSTREAMSOURCE_INDEXEDDATA | instanceCount)));
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VertexBuffer::_current->count(), start, count / 3 * instanceCount);
}

void Graphics::setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing) {
	DWORD value = 0;
	switch (addressing) {
	case Repeat:
		value = D3DTADDRESS_WRAP;
		break;
	case Mirror:
		value = D3DTADDRESS_MIRROR;
		break;
	case Clamp:
		value = D3DTADDRESS_CLAMP;
		break;
	case Border:
		value = D3DTADDRESS_BORDER;
		break;
	}
	device->SetSamplerState(unit.unit, dir == U ? D3DSAMP_ADDRESSU : D3DSAMP_ADDRESSV, value);
}

namespace {
	void tod3dmatrix(mat4& matrix, D3DMATRIX& d3dm) {
		d3dm._11 = matrix.get(0, 0);
		d3dm._12 = matrix.get(0, 1);
		d3dm._13 = matrix.get(0, 2);
		d3dm._14 = matrix.get(0, 3);

		d3dm._21 = matrix.get(1, 0);
		d3dm._22 = matrix.get(1, 1);
		d3dm._23 = matrix.get(1, 2);
		d3dm._24 = matrix.get(1, 3);

		d3dm._31 = matrix.get(2, 0);
		d3dm._32 = matrix.get(2, 1);
		d3dm._33 = matrix.get(2, 2);
		d3dm._34 = matrix.get(2, 3);

		d3dm._41 = matrix.get(3, 0);
		d3dm._42 = matrix.get(3, 1);
		d3dm._43 = matrix.get(3, 2);
		d3dm._44 = matrix.get(3, 3);
	}
}

void Graphics::clear(uint flags, uint color, float z, int stencil) {
	device->Clear(0, nullptr, flags, color, z, stencil);
}

void Graphics::begin(int windowId) {
	// TODO (DK) ignore secondary windows for now
	if (windowId > 0) {
		return;
	}
	
	viewport(0, 0, _width, _height);
	device->BeginScene();
}

void Graphics::viewport(int x, int y, int width, int height) {
	vp.X = x;
	vp.Y = y;
	vp.Width = width;
	vp.Height = height;
	device->SetViewport(&vp);
}

void Graphics::scissor(int x, int y, int width, int height) {
	// TODO
}

void Graphics::disableScissor() {
	// TODO
}

void Graphics::setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask, int writeMask) {
	// TODO
}

void Graphics::end(int windowId) {
	// TODO (DK) ignore secondary windows for now
	if (windowId > 0) {
		return;
	}

	/*if (backBuffer != nullptr) {
		backBuffer->Release();
		backBuffer = nullptr;
	}*/
	device->EndScene();
}

bool Graphics::vsynced() {
	return vsync;
}

unsigned Graphics::refreshRate() {
	return hz;
}

void Graphics::swapBuffers(int windowId) {
	// TODO (DK) ignore secondary windows for now
	if (windowId > 0) {
		return;
	}

	::swapBuffers();
}

namespace {
	_D3DBLEND convert(BlendingOperation operation) {
		switch (operation) {
		case BlendOne:
			return D3DBLEND_ONE;
		case BlendZero:
			return D3DBLEND_ZERO;
		case SourceAlpha:
			return D3DBLEND_SRCALPHA;
		case DestinationAlpha:
			return D3DBLEND_DESTALPHA;
		case InverseSourceAlpha:
			return D3DBLEND_INVSRCALPHA;
		case InverseDestinationAlpha:
			return D3DBLEND_INVDESTALPHA;
		default:
		//	throw Exception("Unknown blending operation.");
			return D3DBLEND_SRCALPHA;
		}
	}
}

void Graphics::setBlendingMode(BlendingOperation source, BlendingOperation destination) {
	device->SetRenderState(D3DRS_SRCBLEND, convert(source));
	device->SetRenderState(D3DRS_DESTBLEND, convert(destination));
}

void Graphics::setRenderState(RenderState state, bool on) {
	switch (state) {
	case BlendingState:
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, on ? TRUE : FALSE);
		break;
	case DepthWrite:
		device->SetRenderState(D3DRS_ZWRITEENABLE, on ? TRUE : FALSE);
		break;
	case DepthTest:
		device->SetRenderState(D3DRS_ZENABLE, on ? TRUE : FALSE);
		break;
	case Normalize:
		device->SetRenderState(D3DRS_NORMALIZENORMALS, on ? TRUE : FALSE);
		break;
	case ScissorTestState:
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, on ? TRUE : FALSE);
		break;
	case AlphaTestState:
		device->SetRenderState(D3DRS_ALPHATESTENABLE, on ? TRUE : FALSE);
		device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		break;
	//default:
	//	throw Exception();
	}
}

void Graphics::setRenderState(RenderState state, int v) {
	switch (state) {
	case DepthTestCompare:
		switch (v) {
		// TODO: Cmp-Konstanten systemabh�ngig abgleichen
		default:
		case ZCompareAlways      : v = D3DCMP_ALWAYS; break;
		case ZCompareNever       : v = D3DCMP_NEVER; break;
		case ZCompareEqual       : v = D3DCMP_EQUAL; break;
		case ZCompareNotEqual    : v = D3DCMP_NOTEQUAL; break;
		case ZCompareLess        : v = D3DCMP_LESS; break;
		case ZCompareLessEqual   : v = D3DCMP_LESSEQUAL; break;
		case ZCompareGreater     : v = D3DCMP_GREATER; break;
		case ZCompareGreaterEqual: v = D3DCMP_GREATEREQUAL; break;
		}
		device->SetRenderState(D3DRS_ZFUNC, v);
		break;
	case BackfaceCulling:
		switch (v) {
		case Clockwise:
			device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			break;
		case CounterClockwise:
			device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			break;
		case NoCulling:
			device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			break;
		default:
			break;
		}
	case AlphaReferenceState:
		device->SetRenderState(D3DRS_ALPHAREF, (DWORD)v);
		break;
	}
}

void Graphics::setRenderState(RenderState state, float value) {
	
}

void Graphics::setBool(ConstantLocation position, bool value) {
	if (position.shaderType == -1) return;
	BOOL bools[4];
	bools[0] = value ? 1 : 0;
	bools[1] = bools[0];
	bools[2] = bools[0];
	bools[3] = bools[0];
	if (position.shaderType == 0) device->SetVertexShaderConstantB(position.reg.regindex, &bools[0], 1);
	else device->SetPixelShaderConstantB(position.reg.regindex, &bools[0], 1);
}

void Graphics::setInt(ConstantLocation position, int value) {
	if (position.shaderType == -1) return;
	int ints[4];
	ints[0] = value;
	ints[1] = value;
	ints[2] = value;
	ints[3] = value;
	if (position.shaderType == 0) device->SetVertexShaderConstantI(position.reg.regindex, &ints[0], 1);
	else device->SetPixelShaderConstantI(position.reg.regindex, &ints[0], 1);
}

void Graphics::setFloat(ConstantLocation position, float value) {
	if (position.shaderType == -1) return;
	float floats[4];
	floats[0] = value;
	floats[1] = value;
	floats[2] = value;
	floats[3] = value;
	if (position.shaderType == 0) device->SetVertexShaderConstantF(position.reg.regindex, floats, 1);
	else device->SetPixelShaderConstantF(position.reg.regindex, floats, 1);
}

void Graphics::setFloat2(ConstantLocation position, float value1, float value2) {
	if (position.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value1;
	floats[3] = value2;
	if (position.shaderType == 0) device->SetVertexShaderConstantF(position.reg.regindex, floats, 1);
	else device->SetPixelShaderConstantF(position.reg.regindex, floats, 1);
}

void Graphics::setFloat3(ConstantLocation position, float value1, float value2, float value3) {
	if (position.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value1;
	if (position.shaderType == 0) device->SetVertexShaderConstantF(position.reg.regindex, floats, 1);
	else device->SetPixelShaderConstantF(position.reg.regindex, floats, 1);
}

void Graphics::setFloat4(ConstantLocation position, float value1, float value2, float value3, float value4) {
	if (position.shaderType == -1) return;
	float floats[4];
	floats[0] = value1;
	floats[1] = value2;
	floats[2] = value3;
	floats[3] = value4;
	if (position.shaderType == 0) device->SetVertexShaderConstantF(position.reg.regindex, floats, 1);
	else device->SetPixelShaderConstantF(position.reg.regindex, floats, 1);
}

void Graphics::setFloats(ConstantLocation location, float* values, int count) {
	if (location.shaderType == -1) return;
	int registerCount = (count + 3) / 4; // round up
	if (registerCount == count / 4) { // round down
		if (location.shaderType == 0) device->SetVertexShaderConstantF(location.reg.regindex, values, registerCount);
		else device->SetPixelShaderConstantF(location.reg.regindex, values, registerCount);
	}
	else {
		float* data = (float*)alloca(registerCount * 4 * sizeof(float));
		memcpy(data, values, count * sizeof(float));
		if (location.shaderType == 0) device->SetVertexShaderConstantF(location.reg.regindex, data, registerCount);
		else device->SetPixelShaderConstantF(location.reg.regindex, data, registerCount);
	}
}

void Graphics::setMatrix(ConstantLocation location, const mat4& value) {
	if (location.shaderType == -1) return;
	float floats[16];
	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			floats[y * 4 + x] = value.get(y, x);
		}
	}
	if (location.shaderType == 0) device->SetVertexShaderConstantF(location.reg.regindex, floats, 4);
	else device->SetPixelShaderConstantF(location.reg.regindex, floats, 4);
}

void Graphics::setMatrix(ConstantLocation location, const mat3& value) {
	if (location.shaderType == -1) return;
	float floats[12];
	for (int y = 0; y < 3; ++y) {
		for (int x = 0; x < 3; ++x) {
			floats[y * 4 + x] = value.get(y, x);
		}
	}
	if (location.shaderType == 0) device->SetVertexShaderConstantF(location.reg.regindex, floats, 3);
	else device->SetPixelShaderConstantF(location.reg.regindex, floats, 3);
}

bool Graphics::renderTargetsInvertedY() {
	return false;
}

bool Graphics::nonPow2TexturesSupported() {
	return true;
}

void Graphics::setVertexBuffers(VertexBuffer** buffers, int count) {
	for (int i = 0; i < count; ++i) {
		buffers[i]->_set(i);
	}
}

void Graphics::setIndexBuffer(IndexBuffer& buffer) {
	buffer._set();
}

void Graphics::setTexture(TextureUnit unit, Texture* texture) {
	texture->_set(unit);
}
