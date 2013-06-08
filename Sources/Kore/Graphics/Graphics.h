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
		VertexBuffer(int count, const VertexStructure& structure);
		virtual ~VertexBuffer();
		float* lock();
		float* lock(int start, int count);
		void unlock();
		int count();
		int stride();
		void set();
	};

	class IndexBuffer : public IndexBufferImpl {
	public:
		IndexBuffer(int count);
		virtual ~IndexBuffer();
		int* lock();
		void unlock();
		int count();
		void set();
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

	enum ZCompare {
		ZCmp_Always      ,
		ZCmp_Never       ,
		ZCmp_Equal       ,
		ZCmp_NotEqual    ,
		ZCmp_Less        ,
		ZCmp_LessEqual   ,
		ZCmp_Greater     ,
		ZCmp_GreaterEqual
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
		Target32BitRedFloat
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
		RenderTarget(int width, int height, bool depthBuffer, bool antialiasing = false, RenderTargetFormat format = Target32Bit);
		int width();
		int height();
		void useColorAsTexture(int texunit);
		//void useDepthAsTexture(int texunit);
	};

	namespace Graphics {
		void setInt(ConstantLocation location, int value);
		void setFloat(ConstantLocation location, float value);
		void setFloat2(ConstantLocation location, float value1, float value2);
		void setFloat3(ConstantLocation location, float value1, float value2, float value3);
		void setMatrix(ConstantLocation location, const mat4& value);
	
		void drawIndexedVertices();
		void drawIndexedVertices(int start, int count);

		void changeResolution(int width, int height);
		bool hasWindow();
		void setWindow(bool);
		bool antialiasing();
		void setAntialiasing(bool);
		
		void setRenderTarget(RenderTarget* texture, int num = 0);
		void restoreRenderTarget();

		void swapBuffers();
		void* getControl();
		void begin();
		void end();

		void setRenderState(RenderState state, bool on);
		void setRenderState(RenderState state, int v);
		void setRenderState(RenderState state, float value);
		void setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing);
		void setTextureMagnificationFilter(int texunit, TextureFilter filter);
		void setTextureMinificationFilter(int texunit, TextureFilter filter);
		void setTextureMipmapFilter(int texunit, MipmapFilter filter);
		void setBlendingMode(BlendingOperation source, BlendingOperation destination);
		void setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2);

		bool isVSynced();
		unsigned getHz();

		const uint ClearColorFlag   = 1;
		const uint ClearDepthFlag   = 2;
		const uint ClearStencilFlag = 4;

		void clear(uint flags, uint color = 0, float depth = 1.0f, int stencil = 0);

		void init();
		void destroy();

		extern bool fullscreen;
	};
}
