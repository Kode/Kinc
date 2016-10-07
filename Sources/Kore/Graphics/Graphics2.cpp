#include "pch.h"

#include "Graphics2.h"
#include "Kore/Simd/float32x4.h"
#include <Kore/IO/FileReader.h>

using namespace Kore;

//==========
// ImageShaderPainter
//==========
ImageShaderPainter::ImageShaderPainter() : bufferSize(1500), bufferIndex(0), vertexSize(9), bilinear(false), bilinearMipmaps(false) {
    initShaders();
    initBuffers();
}

PipelineState* ImageShaderPainter::get_pipeline() const {
    return myPipeline;
}

void ImageShaderPainter::set_pipeline(PipelineState* pipe) {
    if(pipe == nullptr) {
        projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
        textureLocation = shaderPipeline->getTextureUnit("tex");
    } else {
        projectionLocation = pipe->getConstantLocation("projectionMatrix");
        textureLocation = pipe->getTextureUnit("tex");
    }
    myPipeline = pipe;
}

void ImageShaderPainter::setProjection(mat4 projectionMatrix) {
    this->projectionMatrix = projectionMatrix;
}

void ImageShaderPainter::initShaders() {
    if (shaderPipeline != nullptr) return;
    
    structure.add("vertexPosition", Float3VertexData);
    structure.add("texPosition", Float2VertexData);
    structure.add("vertexColor", Float4VertexData);
    
    FileReader fs("painter-image.frag");
    FileReader vs("painter-image.vert");
    Shader* fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
    Shader* vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
    
    shaderPipeline = new PipelineState;
    shaderPipeline->setFragmentShader(fragmentShader);
    shaderPipeline->setVertexShader(vertexShader);
    
    shaderPipeline->blendSource = BlendOne;
    shaderPipeline->blendDestination = InverseSourceAlpha;
    shaderPipeline->alphaBlendSource = SourceAlpha;
    shaderPipeline->alphaBlendDestination = InverseSourceAlpha;
    
//    shaderPipeline->inputLayout[0] = { &structure };
//    shaderPipeline->compile();
    shaderPipeline->link(structure);
    
    projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
    textureLocation = shaderPipeline->getTextureUnit("tex");
}

void ImageShaderPainter::initBuffers() {
    rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure);
    rectVertices = rectVertexBuffer->lock();
    
    indexBuffer = new IndexBuffer(bufferSize * 3 * 2);
    int* indices = indexBuffer->lock();
    for (int i = 0; i < bufferSize; ++i) {
        indices[i * 3 * 2 + 0] = i * 4 + 0;
        indices[i * 3 * 2 + 1] = i * 4 + 1;
        indices[i * 3 * 2 + 2] = i * 4 + 2;
        indices[i * 3 * 2 + 3] = i * 4 + 0;
        indices[i * 3 * 2 + 4] = i * 4 + 2;
        indices[i * 3 * 2 + 5] = i * 4 + 3;
    }
    indexBuffer->unlock();
}

void ImageShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  0] = bottomleftx;
    rectVertices[baseIndex +  1] = bottomlefty;
    rectVertices[baseIndex +  2] = -5.f; // TODO: should be 0?
    
    rectVertices[baseIndex +  9] = topleftx;
    rectVertices[baseIndex + 10] = toplefty;
    rectVertices[baseIndex + 11] = -5.f;
    
    rectVertices[baseIndex + 18] = toprightx;
    rectVertices[baseIndex + 19] = toprighty;
    rectVertices[baseIndex + 20] = -5.f;
    
    rectVertices[baseIndex + 27] = bottomrightx;
    rectVertices[baseIndex + 28] = bottomrighty;
    rectVertices[baseIndex + 29] = -5.f;
}

void ImageShaderPainter::setRectTexCoords(float left, float top, float right, float bottom) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  3] = left;
    rectVertices[baseIndex +  4] = bottom;
    
    rectVertices[baseIndex + 12] = left;
    rectVertices[baseIndex + 13] = top;
    
    rectVertices[baseIndex + 21] = right;
    rectVertices[baseIndex + 22] = top;
    
    rectVertices[baseIndex + 30] = right;
    rectVertices[baseIndex + 31] = bottom;
}

void ImageShaderPainter::setRectColor(float r, float g, float b, float a) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  5] = r;
    rectVertices[baseIndex +  6] = g;
    rectVertices[baseIndex +  7] = b;
    rectVertices[baseIndex +  8] = a;
    
    rectVertices[baseIndex + 14] = r;
    rectVertices[baseIndex + 15] = g;
    rectVertices[baseIndex + 16] = b;
    rectVertices[baseIndex + 17] = a;
    
    rectVertices[baseIndex + 23] = r;
    rectVertices[baseIndex + 24] = g;
    rectVertices[baseIndex + 25] = b;
    rectVertices[baseIndex + 26] = a;
    
    rectVertices[baseIndex + 32] = r;
    rectVertices[baseIndex + 33] = g;
    rectVertices[baseIndex + 34] = b;
    rectVertices[baseIndex + 35] = a;
}

void ImageShaderPainter::drawBuffer() {
    rectVertexBuffer->unlock();
    Graphics::setVertexBuffer(*rectVertexBuffer);
    Graphics::setIndexBuffer(*indexBuffer);
    Graphics::setTexture(textureLocation, lastTexture);
    Graphics::setTextureAddressing(textureLocation, U, Clamp);
    Graphics::setTextureAddressing(textureLocation, V, Clamp);
    Graphics::setTextureMinificationFilter(textureLocation, bilinear ? LinearFilter : PointFilter);
    Graphics::setTextureMagnificationFilter(textureLocation, bilinear ? LinearFilter : PointFilter);
    Graphics::setTextureMipmapFilter(textureLocation, NoMipFilter);
    Graphics::setMatrix(projectionLocation, projectionMatrix);
    
    Graphics::drawIndexedVertices(0, bufferIndex * 2 * 3);
    
    shaderPipeline->set();
    
    //Graphics::setTexture(textureLocation, nullptr);
    bufferIndex = 0;
    rectVertices = rectVertexBuffer->lock();
}

void ImageShaderPainter::setBilinearFilter(bool bilinear) {
    end();
    this->bilinear = bilinear;
}

void ImageShaderPainter::setBilinearMipmapFilter(bool bilinear) {
    end();
    this->bilinearMipmaps = bilinear;
}

inline void ImageShaderPainter::drawImage(Kore::Texture* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color) {
    Texture* tex = img;
    if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture)) drawBuffer();
    
    Color c = Color(color);
    setRectColor(c.R, c.G, c.B, c.A * opacity);
    setRectTexCoords(0, 0, tex->width / (float)tex->texWidth, tex->height / (float)tex->texHeight);
    setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
    
    ++bufferIndex;
    lastTexture = tex;
}

inline void ImageShaderPainter::drawImage2(Kore::Texture* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color) {
    Texture* tex = img;
    if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture)) drawBuffer();
    
    Color c = Color(color);
    setRectColor(c.R, c.G, c.B, c.A * opacity);
    setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
    setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
    
    ++bufferIndex;
    lastTexture = tex;
}

inline void ImageShaderPainter::drawImageScale(Kore::Texture* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom, float opacity, uint color) {
    Texture* tex = img;
    if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture)) drawBuffer();
    
    Color c = Color(color);
    setRectColor(c.R, c.G, c.B, opacity);
    setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
    setRectVertices(left, bottom, left, top, right, top, right, bottom);
    
    ++bufferIndex;
    lastTexture = tex;
}

void ImageShaderPainter::end() {
    if (bufferIndex > 0) drawBuffer();
    lastTexture = nullptr;
}





//==========
// ColoredShaderPainter
//==========

ColoredShaderPainter::ColoredShaderPainter() : bufferSize(100), bufferIndex(0), vertexSize(7), triangleBufferSize(100), triangleBufferIndex(0) {
    initShaders();
    initBuffers();
}

PipelineState* ColoredShaderPainter::get_pipeline() const {
    return myPipeline;
}

void ColoredShaderPainter::set_pipeline(PipelineState* pipe) {
    if(pipe == nullptr) {
        projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
    } else {
        projectionLocation = pipe->getConstantLocation("projectionMatrix");
    }
    myPipeline = pipe;
}

void ColoredShaderPainter::setProjection(mat4 projectionMatrix) {
    this->projectionMatrix = projectionMatrix;
}

void ColoredShaderPainter::initShaders() {
    if (shaderPipeline != nullptr) return;
    
    structure.add("vertexPosition", Float3VertexData);
    structure.add("vertexColor", Float4VertexData);
    
    FileReader fs("painter-colored.frag");
    FileReader vs("painter-colored.vert");
    Shader* fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
    Shader* vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
    
    shaderPipeline = new PipelineState();
    shaderPipeline->setFragmentShader(fragmentShader);
    shaderPipeline->setVertexShader(vertexShader);
    
    shaderPipeline->blendSource = BlendOne;
    shaderPipeline->blendDestination = InverseSourceAlpha;
    shaderPipeline->alphaBlendSource = SourceAlpha;
    shaderPipeline->alphaBlendDestination = InverseSourceAlpha;
    
//  shaderPipeline->inputLayout[0] = { &structure };
//  shaderPipeline->compile();
    shaderPipeline->link(structure);
    
    projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
}

void ColoredShaderPainter::initBuffers() {
    rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure);
    rectVertices = rectVertexBuffer->lock();
    
    indexBuffer = new IndexBuffer(bufferSize * 3 * 2);
    int* indices = indexBuffer->lock();
    for (int i = 0; i < bufferSize; ++i) {
        indices[i * 3 * 2 + 0] = i * 4 + 0;
        indices[i * 3 * 2 + 1] = i * 4 + 1;
        indices[i * 3 * 2 + 2] = i * 4 + 2;
        indices[i * 3 * 2 + 3] = i * 4 + 0;
        indices[i * 3 * 2 + 4] = i * 4 + 2;
        indices[i * 3 * 2 + 5] = i * 4 + 3;
    }
    indexBuffer->unlock();
    
    triangleVertexBuffer = new VertexBuffer(triangleBufferSize * 3, structure);
    triangleVertices = triangleVertexBuffer->lock();
    
    triangleIndexBuffer = new IndexBuffer(triangleBufferSize * 3);
    int* triIndices = triangleIndexBuffer->lock();
    for (int i = 0; i < bufferSize; ++i) {
        triIndices[i * 3 + 0] = i * 3 + 0;
        triIndices[i * 3 + 1] = i * 3 + 1;
        triIndices[i * 3 + 2] = i * 3 + 2;
    }
    triangleIndexBuffer->unlock();
}

void ColoredShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  0] = bottomleftx;
    rectVertices[baseIndex +  1] = bottomlefty;
    rectVertices[baseIndex +  2] = -5.0; // TODO: should be 0?
    
    rectVertices[baseIndex +  7] = topleftx;
    rectVertices[baseIndex +  8] = toplefty;
    rectVertices[baseIndex +  9] = -5.0;
    
    rectVertices[baseIndex + 14] = toprightx;
    rectVertices[baseIndex + 15] = toprighty;
    rectVertices[baseIndex + 16] = -5.0;
    
    rectVertices[baseIndex + 21] = bottomrightx;
    rectVertices[baseIndex + 22] = bottomrighty;
    rectVertices[baseIndex + 23] = -5.0;
}

void ColoredShaderPainter::setRectColors(float opacity, uint color) {
    // TODO: i am not sure if this is correct
    Color c = Color(color);
    float r = c.R;
    float g = c.G;
    float b = c.B;
    float a = c.A * opacity;
    
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  3] = r;
    rectVertices[baseIndex +  4] = g;
    rectVertices[baseIndex +  5] = b;
    rectVertices[baseIndex +  6] = a;
    
    rectVertices[baseIndex + 10] = r;
    rectVertices[baseIndex + 11] = g;
    rectVertices[baseIndex + 12] = b;
    rectVertices[baseIndex + 13] = a;
    
    rectVertices[baseIndex + 17] = r;
    rectVertices[baseIndex + 18] = g;
    rectVertices[baseIndex + 19] = b;
    rectVertices[baseIndex + 20] = a;
    
    rectVertices[baseIndex + 24] = r;
    rectVertices[baseIndex + 25] = g;
    rectVertices[baseIndex + 26] = b;
    rectVertices[baseIndex + 27] = a;
}

void ColoredShaderPainter::setTriVertices(float x1, float y1, float x2, float y2, float x3, float y3) {
    int baseIndex = triangleBufferIndex * 7 * 3;
    triangleVertices[baseIndex +  0] = x1;
    triangleVertices[baseIndex +  1] = y1;
    triangleVertices[baseIndex +  2] = -5.0; // TODO: should be 0?
    
    triangleVertices[baseIndex +  7] = x2;
    triangleVertices[baseIndex +  8] = y2;
    triangleVertices[baseIndex +  9] = -5.0;
    
    triangleVertices[baseIndex + 14] = x3;
    triangleVertices[baseIndex + 15] = y3;
    triangleVertices[baseIndex + 16] = -5.0;
}

void ColoredShaderPainter::setTriColors(float opacity, uint color) {
    Color c = Color(color);
    float r = c.R;
    float g = c.G;
    float b = c.B;
    float a = c.A * opacity;
    
    int baseIndex = triangleBufferIndex * 7 * 3;
    triangleVertices[baseIndex +  3] = r;
    triangleVertices[baseIndex +  4] = g;
    triangleVertices[baseIndex +  5] = b;
    triangleVertices[baseIndex +  6] = a;
    
    triangleVertices[baseIndex + 10] = r;
    triangleVertices[baseIndex + 11] = g;
    triangleVertices[baseIndex + 12] = b;
    triangleVertices[baseIndex + 13] = a;
    
    triangleVertices[baseIndex + 17] = r;
    triangleVertices[baseIndex + 18] = g;
    triangleVertices[baseIndex + 19] = b;
    triangleVertices[baseIndex + 20] = a;
}

void ColoredShaderPainter::drawBuffer(bool trisDone) {
    if (!trisDone) endTris(true);
    
    rectVertexBuffer->unlock();
    Graphics::setVertexBuffer(*rectVertexBuffer);
    Graphics::setIndexBuffer(*indexBuffer);
    Graphics::setMatrix(projectionLocation, projectionMatrix);
    
    Graphics::drawIndexedVertices(0, bufferIndex * 2 * 3);
    
    shaderPipeline->set();
    
    bufferIndex = 0;
    rectVertices = rectVertexBuffer->lock();
}

void ColoredShaderPainter::drawTriBuffer(bool rectsDone) {
    if (!rectsDone) endRects(true);
    
    triangleVertexBuffer->unlock();
    Graphics::setVertexBuffer(*triangleVertexBuffer);
    Graphics::setIndexBuffer(*triangleIndexBuffer);
    Graphics::setMatrix(projectionLocation, projectionMatrix);
    
    Graphics::drawIndexedVertices(0, triangleBufferIndex * 3);
    
    shaderPipeline->set();
    
    triangleBufferIndex = 0;
    triangleVertices = triangleVertexBuffer->lock();
}

void ColoredShaderPainter::fillRect(float opacity, uint color, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty) {
    if (triangleBufferIndex > 0) drawTriBuffer(true); // Flush other buffer for right render order
    
    if (bufferIndex + 1 >= bufferSize) drawBuffer(false);
    
    setRectColors(opacity, color);
    setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
    ++bufferIndex;
}

void ColoredShaderPainter::fillTriangle(float opacity, uint color, float x1, float y1, float x2, float y2, float x3, float y3) {
    if (bufferIndex > 0) drawBuffer(true); // Flush other buffer for right render order
    
    if (triangleBufferIndex + 1 >= triangleBufferSize) drawTriBuffer(false);
    
    setTriColors(opacity, color);
    setTriVertices(x1, y1, x2, y2, x3, y3);
    ++triangleBufferIndex;
}

inline void ColoredShaderPainter::endTris(bool rectsDone) {
    if (triangleBufferIndex > 0) drawTriBuffer(rectsDone);
}

void ColoredShaderPainter::endRects(bool trisDone) {
    if (bufferIndex > 0) drawBuffer(trisDone);
}

void ColoredShaderPainter::end() {
    endTris(false);
    endRects(false);
}



//==========
// TextShaderPainter
//==========
TextShaderPainter::TextShaderPainter() : bufferSize(100), bufferIndex(0), vertexSize(9), bilinear(false), lastTexture(nullptr) {
    initShaders();
    initBuffers();
}

PipelineState* TextShaderPainter::get_pipeline() const {
    return myPipeline;
}

void TextShaderPainter::set_pipeline(PipelineState* pipe) {
    if(pipe == nullptr) {
        projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
        textureLocation = shaderPipeline->getTextureUnit("tex");
    } else {
        projectionLocation = pipe->getConstantLocation("projectionMatrix");
        textureLocation = pipe->getTextureUnit("tex");
    }
    myPipeline = pipe;
}

void TextShaderPainter::setProjection(mat4 projectionMatrix) {
    this->projectionMatrix = projectionMatrix;
}

void TextShaderPainter::initShaders() {
    if (shaderPipeline != nullptr) return;
    
    structure.add("vertexPosition", Float3VertexData);
    structure.add("texPosition", Float2VertexData);
    structure.add("vertexColor", Float4VertexData);
    
    FileReader fs("painter-text.frag");
    FileReader vs("painter-text.vert");
    Shader* fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
    Shader* vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
    
    shaderPipeline = new PipelineState();
    shaderPipeline->setFragmentShader(fragmentShader);
    shaderPipeline->setVertexShader(vertexShader);
    
    shaderPipeline->blendSource = BlendOne;
    shaderPipeline->blendDestination = InverseSourceAlpha;
    shaderPipeline->alphaBlendSource = SourceAlpha;
    shaderPipeline->alphaBlendDestination = InverseSourceAlpha;
    
    //shaderPipeline->inputLayout[0] = { &structure };
    //shaderPipeline->compile();
    shaderPipeline->link(structure);
    
    projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
    textureLocation = shaderPipeline->getTextureUnit("tex");
}

void TextShaderPainter::initBuffers() {
    rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure);
    rectVertices = rectVertexBuffer->lock();
    
    indexBuffer = new IndexBuffer(bufferSize * 3 * 2);
    int* indices = indexBuffer->lock();
    for (int i = 0; i < bufferSize; ++i) {
        indices[i * 3 * 2 + 0] = i * 4 + 0;
        indices[i * 3 * 2 + 1] = i * 4 + 1;
        indices[i * 3 * 2 + 2] = i * 4 + 2;
        indices[i * 3 * 2 + 3] = i * 4 + 0;
        indices[i * 3 * 2 + 4] = i * 4 + 2;
        indices[i * 3 * 2 + 5] = i * 4 + 3;
    }
    indexBuffer->unlock();

}

void TextShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  0] = bottomleftx;
    rectVertices[baseIndex +  1] = bottomlefty;
    rectVertices[baseIndex +  2] = -5.0f; // TODO: should be 0?
    
    rectVertices[baseIndex +  9] = topleftx;
    rectVertices[baseIndex + 10] = toplefty;
    rectVertices[baseIndex + 11] = -5.0;
    
    rectVertices[baseIndex + 18] = toprightx;
    rectVertices[baseIndex + 19] = toprighty;
    rectVertices[baseIndex + 20] = -5.0;
    
    rectVertices[baseIndex + 27] = bottomrightx;
    rectVertices[baseIndex + 28] = bottomrighty;
    rectVertices[baseIndex + 29] = -5.0;
}

void TextShaderPainter::setRectTexCoords(float left, float top, float right, float bottom) {
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  3] = left;
    rectVertices[baseIndex +  4] = bottom;
    
    rectVertices[baseIndex + 12] = left;
    rectVertices[baseIndex + 13] = top;
    
    rectVertices[baseIndex + 21] = right;
    rectVertices[baseIndex + 22] = top;
    
    rectVertices[baseIndex + 30] = right;
    rectVertices[baseIndex + 31] = bottom;
}

void TextShaderPainter::setRectColors(float opacity, uint color) {
    Color c = Color(color);
    float r = c.R;
    float g = c.G;
    float b = c.B;
    float a = c.A * opacity;
    
    int baseIndex = bufferIndex * vertexSize * 4;
    rectVertices[baseIndex +  5] = r;
    rectVertices[baseIndex +  6] = g;
    rectVertices[baseIndex +  7] = b;
    rectVertices[baseIndex +  8] = a;
    
    rectVertices[baseIndex + 14] = r;
    rectVertices[baseIndex + 15] = g;
    rectVertices[baseIndex + 16] = b;
    rectVertices[baseIndex + 17] = a;
    
    rectVertices[baseIndex + 23] = r;
    rectVertices[baseIndex + 24] = g;
    rectVertices[baseIndex + 25] = b;
    rectVertices[baseIndex + 26] = a;
    
    rectVertices[baseIndex + 32] = r;
    rectVertices[baseIndex + 33] = g;
    rectVertices[baseIndex + 34] = b;
    rectVertices[baseIndex + 35] = a;
}

void TextShaderPainter::drawBuffer() {
    rectVertexBuffer->unlock();
    Graphics::setVertexBuffer(*rectVertexBuffer);
    Graphics::setIndexBuffer(*indexBuffer);
    Graphics::setTexture(textureLocation, lastTexture);
    Graphics::setMatrix(projectionLocation, projectionMatrix);
    Graphics::setTextureAddressing(textureLocation, U, Clamp);
    Graphics::setTextureAddressing(textureLocation, V, Clamp);
    Graphics::setTextureMinificationFilter(textureLocation, bilinear ? LinearFilter : PointFilter);
    Graphics::setTextureMagnificationFilter(textureLocation, bilinear ? LinearFilter : PointFilter);
    Graphics::setTextureMipmapFilter(textureLocation, NoMipFilter);
    
    Graphics::drawIndexedVertices(0, bufferIndex * 2 * 3);
    
    shaderPipeline->set();
    
    bufferIndex = 0;
    rectVertices = rectVertexBuffer->lock();
}

void TextShaderPainter::setBilinearFilter(bool bilinear) {
    end();
    this->bilinear = bilinear;
}

void TextShaderPainter::setFont(Kravur* font) {
    this->font = font;
}

//int TextShaderPainter::charCodeAt(int position) {}


//TODO: Make this fast
int TextShaderPainter::findIndex(int charcode, int* fontGlyphs, int glyphCount) {
    for (int index = 0; index < glyphCount; ++index) {
        if (fontGlyphs[index] == charcode) return index;
    }
    return -1;
}

void TextShaderPainter::drawString(const char* text, float opacity, uint color, float x, float y, const mat3& transformation, int* fontGlyphs) {
    Texture* tex = font->getTexture();
    if (lastTexture != nullptr && tex != lastTexture) drawBuffer();
    lastTexture = tex;
    
    float xpos = x;
    float ypos = y;
    unsigned length = strlen(text);
    for (unsigned i = 0; i < length; ++i) {
        AlignedQuad q = font->getBakedQuad(text[i] - 32, xpos, ypos);
        if (q.x0 >= 0) {
            if (bufferIndex + 1 >= bufferSize) drawBuffer();
            setRectColors(1.0f, color);
            setRectTexCoords(q.s0 * tex->width / tex->texWidth, q.t0 * tex->height / tex->texHeight, q.s1 * tex->width / tex->texWidth, q.t1 * tex->height / tex->texHeight);
            vec3 p0 = transformation * vec3(q.x0, q.y1, 1.0f); //bottom-left
            vec3 p1 = transformation * vec3(q.x0, q.y0, 1.0f); //top-left
            vec3 p2 = transformation * vec3(q.x1, q.y0, 1.0f); //top-right
            vec3 p3 = transformation * vec3(q.x1, q.y1, 1.0f); //bottom-right
            setRectVertices(p0.x(), p0.y(), p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
            xpos += q.xadvance;
            ++bufferIndex;
        }
    }
}

void TextShaderPainter::end() {
    if (bufferIndex > 0) drawBuffer();
    lastTexture = nullptr;
}







//==========
// Graphics2
//==========

Graphics2::Graphics2(int width, int height) : color(Color::White), fontColor(Color::Black), screenWidth(width), screenHeight(height), fontSize(14) {
    transformation = mat3::Identity(); // TODO
    opacity = 1.f;
    
    imagePainter = new ImageShaderPainter();
    coloredPainter = new ColoredShaderPainter();
    textPainter = new TextShaderPainter();
    textPainter->fontSize = fontSize;
    
    setProjection();
    
    initShaders();
}

void Graphics2::initShaders() {
    if (videoPipeline != nullptr) return;
    
    VertexStructure structure;
    structure.add("vertexPosition", Float3VertexData);
    structure.add("texPosition", Float2VertexData);
    structure.add("vertexColor", Float4VertexData);
    
    FileReader fs("painter-video.frag");
    FileReader vs("painter-video.vert");
    Shader* fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
    Shader* vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
    
    videoPipeline = new PipelineState();
    videoPipeline->setFragmentShader(fragmentShader);
    videoPipeline->setVertexShader(vertexShader);
    
    //videoPipeline->inputLayout[0] = { &structure };
    
    //videoPipeline->compile();
    videoPipeline->link(structure);
}

int Graphics2::upperPowerOfTwo(int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    
    return v;
};

void Graphics2::setProjection() {
    if (!Graphics::nonPow2TexturesSupported()) {
        screenWidth = upperPowerOfTwo(screenWidth);
        screenHeight = upperPowerOfTwo(screenHeight);
    }
    
    if (!Graphics::renderTargetsInvertedY()) {
        projectionMatrix = mat4::orthogonalProjection(0, screenWidth, 0, screenHeight, 0.1f, 1000);
    } else {
        projectionMatrix = mat4::orthogonalProjection(0, screenWidth, screenHeight, 0, 0.1f, 1000);
    }
    projectionMatrix.Set(2, 3, 0);
    
    imagePainter->setProjection(projectionMatrix);
    coloredPainter->setProjection(projectionMatrix);
    textPainter->setProjection(projectionMatrix);
}

void Graphics2::drawImage(Kore::Texture* img, float x, float y) {
    coloredPainter->end();
    textPainter->end();
    
    float xw = x + img->width;
    float yh = y + img->height;
    
    float32x4 xx = load(x, x, xw, xw);
    float32x4 yy = load(yh, y, y, yh);
    
    float32x4 _00 = loadAll(transformation.get(0, 0));
    float32x4 _01 = loadAll(transformation.get(0, 1));
    float32x4 _02 = loadAll(transformation.get(0, 2));
    float32x4 _10 = loadAll(transformation.get(1, 0));
    float32x4 _11 = loadAll(transformation.get(1, 1));
    float32x4 _12 = loadAll(transformation.get(1, 2));
    float32x4 _20 = loadAll(transformation.get(2, 0));
    float32x4 _21 = loadAll(transformation.get(2, 1));
    float32x4 _22 = loadAll(transformation.get(2, 2));
    
    // matrix multiply
    float32x4 w  = add(add(mul(_02, xx), mul(_12, yy)), _22);
    float32x4 px = div(add(add(mul(_00, xx), mul(_10, yy)), _20), w);
    float32x4 py = div(add(add(mul(_01, xx), mul(_11, yy)), _21), w);
    
    imagePainter->drawImage(img, get(px, 0), get(py, 0), get(px, 1), get(py, 1), get(px, 2), get(py, 2), get(px, 3), get(py, 3), opacity, color);
}

void Graphics2::drawScaledSubImage(Kore::Texture* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
    coloredPainter->end();
    textPainter->end();
    vec2 p1 = transformation * vec3(dx, dy + dh, 1.0f);
    vec2 p2 = transformation * vec3(dx, dy, 3);
    vec2 p3 = transformation * vec3(dx + dw, dy, 1.0f);
    vec2 p4 = transformation * vec3(dx + dw, dy + dh, 1.0f);
    
    imagePainter->drawImage2(img, sx, sy, sw, sh, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), opacity, color);
}

void Graphics2::drawRect(float x, float y, float width, float height, float strength) {
    imagePainter->end();
    textPainter->end();
    
    vec2 p1 = transformation * vec3(x - strength / 2, y + strength / 2, 1.0f); //bottom-left
    vec2 p2 = transformation * vec3(x - strength / 2, y - strength / 2, 1.0f); //top-left
    vec2 p3 = transformation * vec3(x + width + strength / 2, y - strength / 2, 1.0f); //top-right
    vec2 p4 = transformation * vec3(x + width + strength / 2, y + strength / 2, 1.0f); //bottom-right
    coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()); // top
    
    p1 = transformation * vec3(x - strength / 2, y + height + strength / 2, 1.0f);
    p3 = transformation * vec3(x + strength / 2, y - strength / 2, 1.0f);
    p4 = transformation * vec3(x + strength / 2, y + height + strength / 2, 1.0f);
    coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()); // left
    
    p2 = transformation * vec3(x - strength / 2, y + height - strength / 2, 1.0f);
    p3 = transformation * vec3(x + width + strength / 2, y + height - strength / 2, 1.0f);
    p4 = transformation * vec3(x + width + strength / 2, y + height + strength / 2, 1.0f);
    coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()); // bottom
    
    p1 = transformation * vec3(x + width - strength / 2, y + height + strength / 2, 1.0f);
    p2 = transformation * vec3(x + width - strength / 2, y - strength / 2, 1.0f);
    p3 = transformation * vec3(x + width + strength / 2, y - strength / 2, 1.0f);
    p4 = transformation * vec3(x + width + strength / 2, y + height + strength / 2, 1.0f);
    coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y()); // right
    
}

void Graphics2::fillRect(float x, float y, float width, float height) {
    imagePainter->end();
    textPainter->end();
    
    vec2 p1 = transformation * vec3(x, y + height, 1.0f);
    vec2 p2 = transformation * vec3(x, y, 1.0f);
    vec2 p3 = transformation * vec3(x + width, y, 1.0f);
    vec2 p4 = transformation * vec3(x + width, y + height, 1.0f);
    coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y());
}

void Graphics2::drawString(char *text, float x, float y) {
    imagePainter->end();
    coloredPainter->end();
    
    textPainter->drawString(text, opacity, fontColor, x, y, transformation, fontGlyphs);
}

void Graphics2::drawLine(float x1, float y1, float x2, float y2, float strength) {
    imagePainter->end();
    textPainter->end();
    
    vec3 vec;
    if (y2 == y1) vec = vec3(0.0f, -1.0f, 1.0f);
    else vec = vec3(1.0f, -(x2 - x1) / (y2 - y1), 1.0f);
    vec.setLength(strength);
    vec3 p1 = vec3(x1 + 0.5 * vec.x(), y1 + 0.5 * vec.y(), 1.0f);
    vec3 p2 = vec3(x2 + 0.5 * vec.x(), y2 + 0.5 * vec.y(), 1.0f);
    vec3 p3 = p1 - vec;
    vec3 p4 = p2 - vec;
    
    p1 = transformation * p1;
    p2 = transformation * p2;
    p3 = transformation * p3;
    p4 = transformation * p4;
    
    coloredPainter->fillTriangle(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
    coloredPainter->fillTriangle(opacity, color, p3.x(), p3.y(), p2.x(), p2.y(), p4.x(), p4.y());
}

void Graphics2::fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    imagePainter->end();
    textPainter->end();
    
    vec2 p1 = transformation * vec3(x1, y1, 1.0f);
    vec2 p2 = transformation * vec3(x2, y2, 1.0f);
    vec2 p3 = transformation * vec3(x3, y3, 1.0f);
    coloredPainter->fillTriangle(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
}

ImageScaleQuality Graphics2::get_imageScaleQuality() const {
    return myImageScaleQuality;
}

void Graphics2::set_imageScaleQuality(Kore::ImageScaleQuality value) {
    imagePainter->setBilinearFilter(value == High);
    textPainter->setBilinearFilter(value == High);
    myImageScaleQuality = value;
}

ImageScaleQuality Graphics2::get_mipmapScaleQuality() const {
    return myMipmapScaleQuality;
}

void Graphics2::set_mipmapScaleQuality(Kore::ImageScaleQuality value) {
    imagePainter->setBilinearMipmapFilter(value == High);
    //textPainter->setBilinearMipmapFilter(value == High); // TODO (DK) implement for fonts as well?
    myMipmapScaleQuality = value;
}

void Graphics2::setPipeline(Kore::PipelineState* pipeline) {
    flush();
    imagePainter->set_pipeline(pipeline);
    coloredPainter->set_pipeline(pipeline);
    textPainter->set_pipeline(pipeline);
    //if (pipeline != nullptr) g.setPipeline(pipeline);
}

void Graphics2::scissor(int x, int y, int width, int height) {
    Graphics2::flush();
    Graphics::scissor(x, y, width, height);
}

void Graphics2::disableScissor() {
    Graphics2::flush();
    Graphics::disableScissor();
}

void Graphics2::begin(bool clear, uint clearColor) {
    Graphics::begin();
    if(clear) Graphics2::clear(clearColor);
    setProjection();
}

void Graphics2::clear(uint color) {
    Graphics::clear(Graphics::ClearColorFlag, color);
}

void Graphics2::flush() {
    imagePainter->end();
    textPainter->end();
    coloredPainter->end();
}

void Graphics2::end() {
    flush();
    Graphics::end();
}

void Graphics2::drawVideoInternal(/*Video video,*/ float x, float y, float width, float height) {

}

void Graphics2::drawVideo(/*Video video,*/ float x, float y, float width, float height) {
//    setPipeline(videoPipeline);
    drawVideoInternal(/*video,*/ x, y, width, height);
//    setPipeline(nullptr);
}

uint Graphics2::getColor() const {
    return color;
}

void Graphics2::setColor(uint color) {
    this->color = color;
}

float Graphics2::getOpacity() const {
    return opacity;
}

void Graphics2::setOpacity(float opacity) {
    this->opacity = opacity;
}

Kravur* Graphics2::getFont() const {
    return font;
}

void Graphics2::setFont(Kravur* font) {
    textPainter->setFont(font);
    this->font = font;
}

int Graphics2::getFontSize() const {
    return fontSize;
}

void Graphics2::setFontSize(int value) {
    this->fontSize = value;
}

uint Graphics2::getFontColor() const {
    return fontColor;
}

void Graphics2::setFontColor(uint color) {
    this->fontColor = color;
}
