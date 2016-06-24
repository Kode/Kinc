#pragma once

#include "Image.h"
#include <Kore/Math/Matrix.h>
#include <Kore/Math/Vector.h>
#include <Kore/GraphicsImpl.h>
#include "Shader.h"
#include "VertexStructure.h"
#include "Texture.h"

namespace Kore {
	class VertexBuffer : public VertexBufferImpl {
	public:
		VertexBuffer(int count, const VertexStructure& structure, int instanceDataStepRate = 0);
		virtual ~VertexBuffer();
		float* lock();
		float* lock(int start, int count);
		void unlock();
		int count();
		int stride();
		int _set(int offset = 0); // Do not call this directly, use Graphics::setVertexBuffers
	};

	class IndexBuffer : public IndexBufferImpl {
	public:
		IndexBuffer(int count);
		virtual ~IndexBuffer();
		int* lock();
		void unlock();
		int count();
		void _set();
	};

	enum TextureAddressing {
		Repeat,
		Mirror,
		Clamp,
		Border
	};

	enum TextureFilter {
		PointFilter,
		LinearFilter,
		AnisotropicFilter
	};

	enum MipmapFilter {
		NoMipFilter,
		PointMipFilter,
		LinearMipFilter //linear texture filter + linear mip filter -> trilinear filter
	};

	enum RenderState {
		BlendingState, DepthTest, DepthTestCompare, /*Lighting,*/ DepthWrite, Normalize, BackfaceCulling, /*FogState, FogStartState, FogEndState, FogTypeState, FogColorState,*/ ScissorTestState,
		AlphaTestState, AlphaReferenceState
	};

	enum BlendingOperation {
		BlendOne,
		BlendZero,
		SourceAlpha,
		DestinationAlpha,
		InverseSourceAlpha,
		InverseDestinationAlpha
	};

	enum ZCompareMode {
		ZCompareAlways,
		ZCompareNever,
		ZCompareEqual,
		ZCompareNotEqual,
		ZCompareLess,
		ZCompareLessEqual,
		ZCompareGreater,
		ZCompareGreaterEqual
	};

	enum CullMode {
		Clockwise,
		CounterClockwise,
		NoCulling
	};

	enum TexDir {
		U, V
	};

	enum FogType {
		LinearFog
	};

	enum RenderTargetFormat {
		Target32Bit,
		Target64BitFloat,
		Target32BitRedFloat,
		Target128BitFloat,
		Target16BitDepth
	};

	enum StencilAction {
		Keep,
		Zero,
		Replace,
		Increment,
		IncrementWrap,
		Decrement,
		DecrementWrap,
		Invert
	};

	enum TextureOperation {
		ModulateOperation,
		SelectFirstOperation,
		SelectSecondOperation
	};

	enum TextureArgument {
		CurrentColorArgument,
		TextureColorArgument
	};

	class RenderTarget : public RenderTargetImpl {
	public:
		RenderTarget(int width, int height, int depthBufferBits, bool antialiasing = false, RenderTargetFormat format = Target32Bit, int stencilBufferBits = -1, int contextId = 0);
		int width;
		int height;
		int texWidth;
		int texHeight;
		int contextId;
		void useColorAsTexture(TextureUnit unit);
		void useDepthAsTexture(TextureUnit unit);
		void setDepthStencilFrom(RenderTarget* source);
	};

	namespace Graphics {
		void setBool(ConstantLocation location, bool value);
		void setInt(ConstantLocation location, int value);
		void setFloat(ConstantLocation location, float value);
		void setFloat2(ConstantLocation location, float value1, float value2);
		void setFloat2(ConstantLocation location, vec2 value);
		void setFloat3(ConstantLocation location, float value1, float value2, float value3);
		void setFloat3(ConstantLocation location, vec3 value);
		void setFloat4(ConstantLocation location, float value1, float value2, float value3, float value4);
		void setFloat4(ConstantLocation location, vec4 value);
		void setFloats(ConstantLocation location, float* values, int count);
		void setMatrix(ConstantLocation location, const mat3& value);
		void setMatrix(ConstantLocation location, const mat4& value);

		void setVertexBuffer(VertexBuffer& vertexBuffer);
		void setVertexBuffers(VertexBuffer** vertexBuffers, int count);
		void setIndexBuffer(IndexBuffer& indexBuffer);
		void setTexture(TextureUnit unit, Texture* texture);
	
		void drawIndexedVertices();
		void drawIndexedVertices(int start, int count);
		void drawIndexedVerticesInstanced(int instanceCount);
		void drawIndexedVerticesInstanced(int instanceCount, int start, int count);

		void changeResolution(int width, int height);
		bool hasWindow();
		void setWindow(bool);
		int antialiasingSamples();
		void setAntialiasingSamples(int samples);

		bool renderTargetsInvertedY();
		void setRenderTarget(RenderTarget* texture, int num = 0, int additionalTargets = 0);
		void restoreRenderTarget();

		// TODO (DK) windowId should be renamed contextId?
		void setup();
		void swapBuffers(int windowId = 0);
		void begin(int windowId = 0);
		void end(int windowId = 0);
		void makeCurrent(int windowId);
		void clearCurrent();

		void viewport(int x, int y, int width, int height);
		void scissor(int x, int y, int width, int height);
		void disableScissor();
		void setStencilParameters(ZCompareMode compareMode, StencilAction bothPass, StencilAction depthFail, StencilAction stencilFail, int referenceValue, int readMask = 0, int writeMask = 0);

		void setRenderState(RenderState state, bool on);
		void setRenderState(RenderState state, int v);
		void setRenderState(RenderState state, float value);
		void setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing);
		void setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter);
		void setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter);
		void setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter);
		void setBlendingMode(BlendingOperation source, BlendingOperation destination);
		void setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2);
		void setColorMask(bool red, bool green, bool blue, bool alpha);

		bool vsynced();
		unsigned refreshRate();
		bool nonPow2TexturesSupported();

		const uint ClearColorFlag   = 1;
		const uint ClearDepthFlag   = 2;
		const uint ClearStencilFlag = 4;

		void clear(uint flags, uint color = 0, float depth = 1.0f, int stencil = 0);

		void init(int windowId, int depthBufferBits, int stencilBufferBits);
		void destroy(int windowId);

		extern bool fullscreen;

		void flush();
	};
}
