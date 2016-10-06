#pragma once

#include "PipelineState.h"
#include "Color.h"
#include <Kore/Math/Matrix.h>

namespace Kore {
    class Graphics2;
    
    class ImageShaderPainter {
    private:
        mat4 projectionMatrix;
        
        PipelineState* shaderPipeline;
        VertexStructure structure;
        ConstantLocation projectionLocation;
        TextureUnit textureLocation;
        
        int bufferSize;
        int vertexSize;
        int bufferIndex;
        VertexBuffer* rectVertexBuffer;
        float* rectVertices;
        IndexBuffer* indexBuffer;
        
        Texture* lastTexture;
        
        bool bilinear;
        bool bilinearMipmaps;

        PipelineState* myPipeline;
        
        BlendingOperation sourceBlend;      // = Undefined;
        BlendingOperation destinationBlend; // = Undefined;
        
        void initShaders();
        void initBuffers();
        
        void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty);
        void setRectTexCoords(float left, float top, float right, float bottom);
        void setRectColor(float r, float g, float b, float a);
        void drawBuffer();
        
    public:
        ImageShaderPainter();
        
        PipelineState* get_pipeline() const;
        void set_pipeline(PipelineState* pipe);
        
        void setProjection(mat4 projectionMatrix);
        
        void setBilinearFilter(bool bilinear);
        void setBilinearMipmapFilter(bool bilinear);
        
        inline void drawImage(Texture* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, Color color);
        inline void drawImage2(Texture* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, Color color);
        inline void drawImageScale(Texture* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom, float opacity, Color color);
        
        void end();
    };
    
    
    
    class ColoredShaderPainter {
    private:
        mat4 projectionMatrix;
        PipelineState* shaderPipeline;
        VertexStructure structure;
        ConstantLocation projectionLocation;
        
        int bufferSize;
        int bufferIndex;
        VertexBuffer* rectVertexBuffer;
        float* rectVertices;
        IndexBuffer* indexBuffer;
        
        int triangleBufferSize;
        int triangleBufferIndex;
        VertexBuffer* triangleVertexBuffer;
        float* triangleVertices;
        IndexBuffer* triangleIndexBuffer;
        
        PipelineState* myPipeline;
        
        BlendingOperation sourceBlend;      // = Undefined;
        BlendingOperation destinationBlend; // = Undefined;
        
        void initShaders();
        void initBuffers();
        
        void setTriVertices(float x1, float y1, float x2, float y2, float x3, float y3);
        void setTriColors(float opacity, Color color);
        void drawBuffer(bool trisDone);
        void drawTriBuffer(bool rectsDone);

    public:
        ColoredShaderPainter();
        
        PipelineState* get_pipeline() const;
        void set_pipeline(PipelineState* pipe);
        
        void setProjection(mat4 projectionMatrix);
        
        void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty);
        void setRectColors(float opacity, Color color);
        
        void fillRect(float opacity, Color color, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty);
        void fillTriangle(float opacity, Color color, float x1, float y1, float x2, float y2, float x3, float y3);
        
        inline void endTris(bool rectsDone);
        inline void endRects(bool trisDone);
        
        inline void end();
    
    };
    
    
    
    class TextShaderPainter {
    private:
        mat4 projectionMatrix;
        PipelineState* shaderPipeline;
        VertexStructure structure;
        ConstantLocation projectionLocation;
        TextureUnit textureLocation;
        
        int bufferSize;
        int bufferIndex;
        VertexBuffer* rectVertexBuffer;
        float* rectVertices;
        IndexBuffer* indexBuffer;
        
        //Kravur font;
        
        Texture* lastTexture;
        
        PipelineState* myPipeline;
        
        bool bilinear;
        
        BlendingOperation sourceBlend;      // = Undefined;
        BlendingOperation destinationBlend; // = Undefined;
        
        void initShaders();
        void initBuffers();
        
        void setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty);
        void setRectTexCoords(float left, float top, float right, float bottom);
        void setRectColors(float opacity, Color color);
        void drawBuffer();
        
        char* text;
        void startString(const char* text);
        int charCodeAt(int position);
        int stringLength();
        void endString();
        
        int findIndex(int charcode, int* fontGlyphs);
        
    public:
        TextShaderPainter();
        
        int fontSize;
        
        PipelineState* get_pipeline() const;
        void set_pipeline(PipelineState* pipe);
        
        void setProjection(mat4 projectionMatrix);
        
        void setBilinearFilter(bool bilinear);
        
        //void setFont(Font font);
        
        void drawString(char* text, float opacity, Color color, float x, float y, mat3 transformation, int* fontGlyphs);
        
        void end();
    };
    
    
    enum ImageScaleQuality {
        Low,
        High
    };
    
    class Graphics2 {
    private:
        int screenWidth;
        int screenHeight;
        
        Color color;

        float opacity;
        
        //Font myFont;
        //Font get_font() const;
        //void set_font(Font font);
        //int set_fontSize(int value);
        
        mat4 projectionMatrix;
        
        mat3 transformation;

        int* fontGlyphs;

        ImageShaderPainter* imagePainter;
        ColoredShaderPainter* coloredPainter;
        TextShaderPainter* textPainter;
        
        PipelineState* videoPipeline;
        
        ImageScaleQuality myImageScaleQuality = High;
        ImageScaleQuality get_imageScaleQuality() const;
        void set_imageScaleQuality(ImageScaleQuality value);
        
        ImageScaleQuality myMipmapScaleQuality = High;
        ImageScaleQuality get_mipmapScaleQuality() const;
        void set_mipmapScaleQuality(ImageScaleQuality value);
        
        int upperPowerOfTwo(int v);
        void setProjection();
        
        void initShaders();

        
    public:
        Graphics2(int width, int height);
        
        void drawImage(Texture* img, float x, float y);
        void drawScaledSubImage(Texture* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);
        
        void drawRect(float x, float y, float width, float height, float strength = 1.0);
        void fillRect(float x, float y, float width, float height);
        
        void drawString(char* text, float x, float y);
        
        void drawLine(float x1, float y1, float x2, float y2, float strength = 1.0);
        
        void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
        
        void setPipeline(PipelineState* pipeline);
        
        void scissor(int x, int y, int width, int height);
        void disableScissor();
        
        void begin(bool clear = true, uint clearColor = 0);
        void clear(uint color = 0);
        void flush();
        void end();
        
        void drawVideoInternal(/*Video video,*/ float x, float y, float width, float height);
        void drawVideo(/*Video video,*/ float x, float y, float width, float height);
        
    };

    
}
