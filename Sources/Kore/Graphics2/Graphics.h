#pragma once

#include <Kore/Graphics1/Color.h>
#include "Kravur.h"
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Math/Matrix.h>

namespace Kore {
	namespace Graphics2 {
		class Graphics2;

		typedef Kore::Graphics1::Color Color;

		class ImageShaderPainter {
		private:
			mat4 projectionMatrix;

			Graphics4::PipelineState* shaderPipeline;
			Graphics4::VertexStructure structure;
			Graphics4::ConstantLocation projectionLocation;
			Graphics4::TextureUnit textureLocation;

			int bufferSize;
			int vertexSize;
			int bufferIndex;
			Graphics4::VertexBuffer* rectVertexBuffer;
			float* rectVertices;
			Graphics4::IndexBuffer* indexBuffer;

			Graphics4::Texture* lastTexture;
			Graphics4::RenderTarget* lastRenderTarget;

			bool bilinear;
			bool bilinearMipmaps;

			Graphics4::PipelineState* myPipeline;

			Graphics4::BlendingOperation sourceBlend;      // = Undefined;
			Graphics4::BlendingOperation destinationBlend; // = Undefined;

			void initShaders();
			void initBuffers();

			void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx,
				float bottomrighty);
			void setRectTexCoords(float left, float top, float right, float bottom);
			void setRectColor(float r, float g, float b, float a);
			void drawBuffer();

		public:
			ImageShaderPainter();
			~ImageShaderPainter();

			Graphics4::PipelineState* get_pipeline() const;
			void set_pipeline(Graphics4::PipelineState* pipe);

			void setProjection(mat4 projectionMatrix);

			void setBilinearFilter(bool bilinear);
			void setBilinearMipmapFilter(bool bilinear);

			inline void drawImage(Graphics4::Texture* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
				float bottomrightx, float bottomrighty, float opacity, uint color);
			inline void drawImage2(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx, float toplefty,
				float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color);
			inline void drawImageScale(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom, float opacity,
				uint color);

			inline void drawImage(Graphics4::RenderTarget* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
				float bottomrightx, float bottomrighty, float opacity, uint color);
			inline void drawImage2(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx, float toplefty,
				float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color);
			inline void drawImageScale(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom, float opacity,
				uint color);

			void end();
		};

		class ColoredShaderPainter {
		private:
			mat4 projectionMatrix;
			Graphics4::PipelineState* shaderPipeline;
			Graphics4::VertexStructure structure;
			Graphics4::ConstantLocation projectionLocation;

			int bufferSize;
			int vertexSize;
			int bufferIndex;
			Graphics4::VertexBuffer* rectVertexBuffer;
			float* rectVertices;
			Graphics4::IndexBuffer* indexBuffer;

			int triangleBufferSize;
			int triangleBufferIndex;
			Graphics4::VertexBuffer* triangleVertexBuffer;
			float* triangleVertices;
			Graphics4::IndexBuffer* triangleIndexBuffer;

			Graphics4::PipelineState* myPipeline;

			Graphics4::BlendingOperation sourceBlend;      // = Undefined;
			Graphics4::BlendingOperation destinationBlend; // = Undefined;

			void initShaders();
			void initBuffers();

			void setTriVertices(float x1, float y1, float x2, float y2, float x3, float y3);
			void setTriColors(float opacity, uint color);
			void drawBuffer(bool trisDone);
			void drawTriBuffer(bool rectsDone);

		public:
			ColoredShaderPainter();
			~ColoredShaderPainter();

			Graphics4::PipelineState* get_pipeline() const;
			void set_pipeline(Graphics4::PipelineState* pipe);

			void setProjection(mat4 projectionMatrix);

			void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx,
				float bottomrighty);
			void setRectColors(float opacity, uint color);

			void fillRect(float opacity, uint color, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
				float bottomrightx, float bottomrighty);
			void fillTriangle(float opacity, uint color, float x1, float y1, float x2, float y2, float x3, float y3);

			inline void endTris(bool rectsDone);
			inline void endRects(bool trisDone);

			inline void end();
		};

		class TextShaderPainter {
		private:
			mat4 projectionMatrix;
			Graphics4::PipelineState* shaderPipeline;
			Graphics4::VertexStructure structure;
			Graphics4::ConstantLocation projectionLocation;
			Graphics4::TextureUnit textureLocation;

			int bufferSize;
			int bufferIndex;
			int vertexSize;
			Graphics4::VertexBuffer* rectVertexBuffer;
			float* rectVertices;
			Graphics4::IndexBuffer* indexBuffer;

			Kravur* font;

			Graphics4::Texture* lastTexture;

			Graphics4::PipelineState* myPipeline;

			bool bilinear;

			Graphics4::BlendingOperation sourceBlend;      // = Undefined;
			Graphics4::BlendingOperation destinationBlend; // = Undefined;

			void initShaders();
			void initBuffers();

			void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx,
				float bottomrighty);
			void setRectTexCoords(float left, float top, float right, float bottom);
			void setRectColors(float opacity, uint color);
			void drawBuffer();

			char* text;
			int charCodeAt(int position);

			int findIndex(int charcode, int* fontGlyphs, int glyphCount);

		public:
			TextShaderPainter();
			~TextShaderPainter();

			int fontSize;

			Graphics4::PipelineState* get_pipeline() const;
			void set_pipeline(Graphics4::PipelineState* pipe);

			void setProjection(mat4 projectionMatrix);

			void setBilinearFilter(bool bilinear);

			void setFont(Kravur* font);

			void drawString(const char* text, int start, int length, float opacity, uint color, float x, float y, const mat3& transformation, int* fontGlyphs);

			void end();
		};

		enum ImageScaleQuality { Low, High };

		class Graphics2 {
		private:
			int screenWidth;
			int screenHeight;

			bool renderTargets;

			uint color;

			float opacity;

			Kravur* font;
			int fontSize;
			uint fontColor;

			mat4 projectionMatrix;

			int* fontGlyphs;

			ImageShaderPainter* imagePainter;
			ColoredShaderPainter* coloredPainter;
			TextShaderPainter* textPainter;

			Graphics4::PipelineState* videoPipeline;
			Graphics4::PipelineState* lastPipeline;

			ImageScaleQuality myImageScaleQuality;
			ImageScaleQuality myMipmapScaleQuality;

			int upperPowerOfTwo(int v);
			void setProjection();

			void initShaders();

		public:
			Graphics2(int width, int height, bool rTargets = false);
			~Graphics2();

			mat3 transformation;

			void drawImage(Graphics4::Texture* img, float x, float y);
			void drawScaledSubImage(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

			void drawImage(Graphics4::RenderTarget* img, float x, float y);
			void drawScaledSubImage(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

			void drawRect(float x, float y, float width, float height, float strength = 1.0);
			void fillRect(float x, float y, float width, float height);

			void drawString(const char* text, float x, float y);
			void drawString(const char* text, int length, float x, float y);
			void drawString(const char* text, int start, int length, float x, float y);

			void drawLine(float x1, float y1, float x2, float y2, float strength = 1.0);

			void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3);

			void setPipeline(Graphics4::PipelineState* pipeline);

			void scissor(int x, int y, int width, int height);
			void disableScissor();

			void begin(bool renderTargets = false, int width = -1, int height = -1, bool clear = true, uint clearColor = Color::Black);
			void clear(uint color = Color::Black);
			void flush();
			void end();

			void drawVideoInternal(/*Video video,*/ float x, float y, float width, float height);
			void drawVideo(/*Video video,*/ float x, float y, float width, float height);

			uint getColor() const;
			void setColor(uint color);

			float getOpacity() const;
			void setOpacity(float opacity);

			ImageScaleQuality getImageScaleQuality() const;
			void setImageScaleQuality(ImageScaleQuality value);

			ImageScaleQuality getMipmapScaleQuality() const;
			void setMipmapScaleQuality(ImageScaleQuality value);

			Kravur* getFont() const;
			void setFont(Kravur* font);
			int getFontSize() const;
			void setFontSize(int value);
			uint getFontColor() const;
			void setFontColor(uint color);
		};
	}
}
