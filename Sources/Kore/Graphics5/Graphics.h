#pragma once

#include <Kore/Graphics1/Image.h>

#include "Shader.h"
#include "Texture.h"
#include "VertexStructure.h"

#include <Kore/Graphics5Impl.h>
#include <Kore/Math/Matrix.h>
#include <Kore/Math/Vector.h>

namespace Kore {
	namespace Graphics5 {
		typedef Graphics1::Image Image;

		class PipelineState;

		class VertexBuffer : public VertexBuffer5Impl {
		public:
			VertexBuffer(int count, const VertexStructure& structure, bool gpuMemory, int instanceDataStepRate = 0);
			virtual ~VertexBuffer();
			float* lock();
			float* lock(int start, int count);
			void unlock();
			int count();
			int stride();
			int _set(int offset = 0); // Do not call this directly, use Graphics::setVertexBuffers
		};

		class IndexBuffer : public IndexBuffer5Impl {
		public:
			IndexBuffer(int count, bool gpuMemory);
			virtual ~IndexBuffer();
			int* lock();
			void unlock();
			int count();
			void _set();
		};

		enum TextureAddressing { Repeat, Mirror, Clamp, Border };

		enum TextureFilter { PointFilter, LinearFilter, AnisotropicFilter };

		enum MipmapFilter {
			NoMipFilter,
			PointMipFilter,
			LinearMipFilter // linear texture filter + linear mip filter -> trilinear filter
		};

		enum RenderState {
			BlendingState,
			DepthTest,
			DepthTestCompare,
			/*Lighting,*/ DepthWrite,
			Normalize,
			BackfaceCulling,
			/*FogState, FogStartState, FogEndState, FogTypeState, FogColorState,*/ ScissorTestState,
			AlphaTestState,
			AlphaReferenceState
		};

		enum BlendingOperation {
			BlendOne,
			BlendZero,
			SourceAlpha,
			DestinationAlpha,
			InverseSourceAlpha,
			InverseDestinationAlpha,
			SourceColor,
			DestinationColor,
			InverseSourceColor,
			InverseDestinationColor
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

		enum CullMode { Clockwise, CounterClockwise, NoCulling };

		enum TexDir { U, V, W };

		enum FogType { LinearFog };

		enum RenderTargetFormat { Target32Bit, Target64BitFloat, Target32BitRedFloat, Target128BitFloat, Target16BitDepth, Target8BitRed };

		enum StencilAction { Keep, Zero, Replace, Increment, IncrementWrap, Decrement, DecrementWrap, Invert };

		enum TextureOperation { ModulateOperation, SelectFirstOperation, SelectSecondOperation };

		enum TextureArgument { CurrentColorArgument, TextureColorArgument };

		class RenderTarget : public RenderTarget5Impl {
		public:
			RenderTarget(int width, int height, int depthBufferBits, bool antialiasing = false, RenderTargetFormat format = Target32Bit, int stencilBufferBits = -1,
				int contextId = 0);
			RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing = false, RenderTargetFormat format = Target32Bit, int stencilBufferBits = -1,
				int contextId = 0);
			~RenderTarget();
			int width;
			int height;
			int texWidth;
			int texHeight;
			int contextId;
			bool isCubeMap;
			bool isDepthAttachment;
			void useColorAsTexture(TextureUnit unit);
			void useDepthAsTexture(TextureUnit unit);
			void setDepthStencilFrom(RenderTarget* source);
		};

		void setTexture(TextureUnit unit, Texture* texture);
		void setImageTexture(TextureUnit unit, Texture* texture);

		void drawIndexedVerticesInstanced(int instanceCount);
		void drawIndexedVerticesInstanced(int instanceCount, int start, int count);

		void changeResolution(int width, int height);
		bool hasWindow();
		void setWindow(bool);
		int antialiasingSamples();
		void setAntialiasingSamples(int samples);

		bool renderTargetsInvertedY();
		void setRenderTargetFace(RenderTarget* texture, int face = 0);

		// TODO (DK) windowId should be renamed contextId?
		void setup();
		void begin(RenderTarget* renderTarget, int windowId = 0);
		void end(int windowId = 0);
		bool swapBuffers(int windowId);
		void makeCurrent(int windowId);
		void clearCurrent();

		void setTextureAddressing(TextureUnit unit, TexDir dir, TextureAddressing addressing);
		void setTextureMagnificationFilter(TextureUnit texunit, TextureFilter filter);
		void setTextureMinificationFilter(TextureUnit texunit, TextureFilter filter);
		void setTextureMipmapFilter(TextureUnit texunit, MipmapFilter filter);
		void setTextureOperation(TextureOperation operation, TextureArgument arg1, TextureArgument arg2);

		bool vsynced();
		unsigned refreshRate();
		bool nonPow2TexturesSupported();

		// Occlusion Query
		bool initOcclusionQuery(uint* occlusionQuery);
		void deleteOcclusionQuery(uint occlusionQuery);
		void renderOcclusionQuery(uint occlusionQuery, int triangles);
		bool isQueryResultsAvailable(uint occlusionQuery);
		void getQueryResults(uint occlusionQuery, uint* pixelCount);

		const uint ClearColorFlag = 1;
		const uint ClearDepthFlag = 2;
		const uint ClearStencilFlag = 4;
		
		void init(int windowId, int depthBufferBits, int stencilBufferBits, bool vsync = true);
		void destroy(int windowId);

		extern bool fullscreen;

		void flush();
	}
}
