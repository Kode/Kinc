#include "pch.h"

#include "Graphics.h"

#include <Kore/Graphics3/Graphics.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Simd/float32x4.h>

#include <string.h>

using namespace Kore;

#ifdef KORE_G4

//==========
// ImageShaderPainter
//==========
Graphics2::ImageShaderPainter::ImageShaderPainter()
    : bufferSize(1500), bufferIndex(0), vertexSize(9), bilinear(false), bilinearMipmaps(false), shaderPipeline(nullptr), lastTexture(nullptr), lastRenderTarget(nullptr), myPipeline(nullptr) {
	initShaders();
	initBuffers();
}

Graphics4::PipelineState* Graphics2::ImageShaderPainter::get_pipeline() const {
	return myPipeline;
}

void Graphics2::ImageShaderPainter::set_pipeline(Graphics4::PipelineState* pipe) {
	if (pipe == nullptr) {
		projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
		textureLocation = shaderPipeline->getTextureUnit("tex");
		myPipeline = shaderPipeline;
	}
	else {
		projectionLocation = pipe->getConstantLocation("projectionMatrix");
		textureLocation = pipe->getTextureUnit("tex");
		myPipeline = pipe;
	}
}

void Graphics2::ImageShaderPainter::setProjection(mat4 projectionMatrix) {
	this->projectionMatrix = projectionMatrix;
}

void Graphics2::ImageShaderPainter::initShaders() {
	if (shaderPipeline != nullptr) return;

	structure.add("vertexPosition", Graphics4::Float3VertexData);
	structure.add("texPosition", Graphics4::Float2VertexData);
	structure.add("vertexColor", Graphics4::Float4VertexData);

	FileReader fs("painter-image.frag");
	FileReader vs("painter-image.vert");
	Graphics4::Shader* fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	Graphics4::Shader* vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);

	shaderPipeline = new Graphics4::PipelineState;
	shaderPipeline->fragmentShader = fragmentShader;
	shaderPipeline->vertexShader = vertexShader;

	shaderPipeline->blendSource = Graphics4::BlendOne;
	shaderPipeline->blendDestination = Graphics4::InverseSourceAlpha;
	shaderPipeline->alphaBlendSource = Graphics4::SourceAlpha;
	shaderPipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;

	//    shaderPipeline->inputLayout[0] = { &structure };
	//    shaderPipeline->compile();
	shaderPipeline->inputLayout[0] = &structure;
	shaderPipeline->inputLayout[1] = nullptr;
	shaderPipeline->compile();

	projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
	textureLocation = shaderPipeline->getTextureUnit("tex");

	myPipeline = shaderPipeline;
}

void Graphics2::ImageShaderPainter::initBuffers() {
	rectVertexBuffer = new Graphics4::VertexBuffer(bufferSize * 4, structure);
	rectVertices = rectVertexBuffer->lock();

	indexBuffer = new Graphics4::IndexBuffer(bufferSize * 3 * 2);
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

void Graphics2::ImageShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
                                         float bottomrightx, float bottomrighty) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 0] = bottomleftx;
	rectVertices[baseIndex + 1] = bottomlefty;
	rectVertices[baseIndex + 2] = -5.f; // TODO: should be 0?

	rectVertices[baseIndex + 9] = topleftx;
	rectVertices[baseIndex + 10] = toplefty;
	rectVertices[baseIndex + 11] = -5.f;

	rectVertices[baseIndex + 18] = toprightx;
	rectVertices[baseIndex + 19] = toprighty;
	rectVertices[baseIndex + 20] = -5.f;

	rectVertices[baseIndex + 27] = bottomrightx;
	rectVertices[baseIndex + 28] = bottomrighty;
	rectVertices[baseIndex + 29] = -5.f;
}

void Graphics2::ImageShaderPainter::setRectTexCoords(float left, float top, float right, float bottom) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 3] = left;
	rectVertices[baseIndex + 4] = bottom;

	rectVertices[baseIndex + 12] = left;
	rectVertices[baseIndex + 13] = top;

	rectVertices[baseIndex + 21] = right;
	rectVertices[baseIndex + 22] = top;

	rectVertices[baseIndex + 30] = right;
	rectVertices[baseIndex + 31] = bottom;
}

void Graphics2::ImageShaderPainter::setRectColor(float r, float g, float b, float a) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 5] = r;
	rectVertices[baseIndex + 6] = g;
	rectVertices[baseIndex + 7] = b;
	rectVertices[baseIndex + 8] = a;

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

void Graphics2::ImageShaderPainter::drawBuffer() {
	rectVertexBuffer->unlock();
	Graphics4::setPipeline(myPipeline);
	Graphics4::setVertexBuffer(*rectVertexBuffer);
	Graphics4::setIndexBuffer(*indexBuffer);
	if (lastRenderTarget != nullptr) lastRenderTarget->useColorAsTexture(textureLocation);
	else Graphics4::setTexture(textureLocation, lastTexture);
	Graphics4::setTextureAddressing(textureLocation, Graphics4::U, Graphics4::Clamp);
	Graphics4::setTextureAddressing(textureLocation, Graphics4::V, Graphics4::Clamp);
	Graphics4::setTextureMinificationFilter(textureLocation, bilinear ? Graphics4::LinearFilter : Graphics4::PointFilter);
	Graphics4::setTextureMagnificationFilter(textureLocation, bilinear ? Graphics4::LinearFilter : Graphics4::PointFilter);
	Graphics4::setTextureMipmapFilter(textureLocation, Graphics4::NoMipFilter);

    #ifndef KORE_G4
    // Set fixed-function projection matrix
    Graphics3::setProjectionMatrix(projectionMatrix);
    #else
    // Set shader matrix uniform
	Graphics4::setMatrix(projectionLocation, projectionMatrix);
    #endif

	Graphics4::drawIndexedVertices(0, bufferIndex * 2 * 3);

	// Graphics::setTexture(textureLocation, nullptr);
	bufferIndex = 0;
	rectVertices = rectVertexBuffer->lock();
}

void Graphics2::ImageShaderPainter::setBilinearFilter(bool bilinear) {
	end();
	this->bilinear = bilinear;
}

void Graphics2::ImageShaderPainter::setBilinearMipmapFilter(bool bilinear) {
	end();
	this->bilinearMipmaps = bilinear;
}

inline void Graphics2::ImageShaderPainter::drawImage(Graphics4::Texture* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx,
                                          float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color) {
	Graphics4::Texture* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture) || lastRenderTarget != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, c.A * opacity);
	setRectTexCoords(0, 0, tex->width / (float)tex->texWidth, tex->height / (float)tex->texHeight);
	setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++bufferIndex;
	lastTexture = tex;
	lastRenderTarget = nullptr;
}

inline void Graphics2::ImageShaderPainter::drawImage2(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx,
                                           float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity,
                                           uint color) {
	Graphics4::Texture* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture) || lastRenderTarget != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, c.A * opacity);
	setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
	setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++bufferIndex;
	lastTexture = tex;
	lastRenderTarget = nullptr;
}

inline void Graphics2::ImageShaderPainter::drawImageScale(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom,
	float opacity, uint color) {
	Graphics4::Texture* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastTexture != nullptr && tex != lastTexture) || lastRenderTarget != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, opacity);
	setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
	setRectVertices(left, bottom, left, top, right, top, right, bottom);

	++bufferIndex;
	lastTexture = tex;
	lastRenderTarget = nullptr;
}

inline void Graphics2::ImageShaderPainter::drawImage(Graphics4::RenderTarget* img, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx,
	float toprighty, float bottomrightx, float bottomrighty, float opacity, uint color) {
	Graphics4::RenderTarget* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastRenderTarget != nullptr && tex != lastRenderTarget) || lastTexture != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, c.A * opacity);
	setRectTexCoords(0, 0, tex->width / (float)tex->texWidth, tex->height / (float)tex->texHeight);
	setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++bufferIndex;
	lastRenderTarget = tex;
	lastTexture = nullptr;
}

inline void Graphics2::ImageShaderPainter::drawImage2(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float bottomleftx, float bottomlefty, float topleftx,
	float toplefty, float toprightx, float toprighty, float bottomrightx, float bottomrighty, float opacity,
	uint color) {
	Graphics4::RenderTarget* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastRenderTarget != nullptr && tex != lastRenderTarget) || lastTexture != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, c.A * opacity);
	setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
	setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);

	++bufferIndex;
	lastRenderTarget = tex;
	lastTexture = nullptr;
}

inline void Graphics2::ImageShaderPainter::drawImageScale(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float left, float top, float right, float bottom,
                                               float opacity, uint color) {
	Graphics4::RenderTarget* tex = img;
	if (bufferIndex + 1 >= bufferSize || (lastRenderTarget != nullptr && tex != lastRenderTarget) || lastTexture != nullptr) drawBuffer();

	Color c = Color(color);
	setRectColor(c.R, c.G, c.B, opacity);
	setRectTexCoords(sx / (float)tex->texWidth, sy / (float)tex->texHeight, (sx + sw) / (float)tex->texWidth, (sy + sh) / (float)tex->texHeight);
	setRectVertices(left, bottom, left, top, right, top, right, bottom);

	++bufferIndex;
	lastRenderTarget = tex;
	lastTexture = nullptr;
}

void Graphics2::ImageShaderPainter::end() {
	if (bufferIndex > 0) drawBuffer();
	lastTexture = nullptr;
	lastRenderTarget = nullptr;
}

Graphics2::ImageShaderPainter::~ImageShaderPainter() {
	delete shaderPipeline;
	delete rectVertexBuffer;
	delete indexBuffer;
}

//==========
// ColoredShaderPainter
//==========

Graphics2::ColoredShaderPainter::ColoredShaderPainter()
    : bufferSize(100), bufferIndex(0), vertexSize(7), triangleBufferSize(100), triangleBufferIndex(0), shaderPipeline(nullptr) {
	initShaders();
	initBuffers();
}

Graphics4::PipelineState* Graphics2::ColoredShaderPainter::get_pipeline() const {
	return myPipeline;
}

void Graphics2::ColoredShaderPainter::set_pipeline(Graphics4::PipelineState* pipe) {
	if (pipe == nullptr) {
		projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
		myPipeline = shaderPipeline;
	}
	else {
		projectionLocation = pipe->getConstantLocation("projectionMatrix");
		myPipeline = pipe;
	}
}

void Graphics2::ColoredShaderPainter::setProjection(mat4 projectionMatrix) {
	this->projectionMatrix = projectionMatrix;
}

void Graphics2::ColoredShaderPainter::initShaders() {
	if (shaderPipeline != nullptr) return;

	structure.add("vertexPosition", Graphics4::Float3VertexData);
	structure.add("vertexColor", Graphics4::Float4VertexData);

	FileReader fs("painter-colored.frag");
	FileReader vs("painter-colored.vert");
	Graphics4::Shader* fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	Graphics4::Shader* vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);

	shaderPipeline = new Graphics4::PipelineState();
	shaderPipeline->fragmentShader = fragmentShader;
	shaderPipeline->vertexShader = vertexShader;

	shaderPipeline->blendSource = Graphics4::SourceAlpha;
	shaderPipeline->blendDestination = Graphics4::InverseSourceAlpha;
	shaderPipeline->alphaBlendSource = Graphics4::SourceAlpha;
	shaderPipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;

	//  shaderPipeline->inputLayout[0] = { &structure };
	//  shaderPipeline->compile();
	shaderPipeline->inputLayout[0] = &structure;
	shaderPipeline->inputLayout[1] = nullptr;
	shaderPipeline->compile();

	projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
	myPipeline = shaderPipeline;
}

void Graphics2::ColoredShaderPainter::initBuffers() {
	rectVertexBuffer = new Graphics4::VertexBuffer(bufferSize * 4, structure);
	rectVertices = rectVertexBuffer->lock();

	indexBuffer = new Graphics4::IndexBuffer(bufferSize * 3 * 2);
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

	triangleVertexBuffer = new Graphics4::VertexBuffer(triangleBufferSize * 3, structure);
	triangleVertices = triangleVertexBuffer->lock();

	triangleIndexBuffer = new Graphics4::IndexBuffer(triangleBufferSize * 3);
	int* triIndices = triangleIndexBuffer->lock();
	for (int i = 0; i < bufferSize; ++i) {
		triIndices[i * 3 + 0] = i * 3 + 0;
		triIndices[i * 3 + 1] = i * 3 + 1;
		triIndices[i * 3 + 2] = i * 3 + 2;
	}
	triangleIndexBuffer->unlock();
}

void Graphics2::ColoredShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
                                           float bottomrightx, float bottomrighty) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 0] = bottomleftx;
	rectVertices[baseIndex + 1] = bottomlefty;
	rectVertices[baseIndex + 2] = -5.0; // TODO: should be 0?

	rectVertices[baseIndex + 7] = topleftx;
	rectVertices[baseIndex + 8] = toplefty;
	rectVertices[baseIndex + 9] = -5.0;

	rectVertices[baseIndex + 14] = toprightx;
	rectVertices[baseIndex + 15] = toprighty;
	rectVertices[baseIndex + 16] = -5.0;

	rectVertices[baseIndex + 21] = bottomrightx;
	rectVertices[baseIndex + 22] = bottomrighty;
	rectVertices[baseIndex + 23] = -5.0;
}

void Graphics2::ColoredShaderPainter::setRectColors(float opacity, uint color) {
	Color c = Color(color);
	float r = c.R;
	float g = c.G;
	float b = c.B;
	float a = c.A * opacity;

	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 3] = r;
	rectVertices[baseIndex + 4] = g;
	rectVertices[baseIndex + 5] = b;
	rectVertices[baseIndex + 6] = a;

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

void Graphics2::ColoredShaderPainter::setTriVertices(float x1, float y1, float x2, float y2, float x3, float y3) {
	int baseIndex = triangleBufferIndex * 7 * 3;
	triangleVertices[baseIndex + 0] = x1;
	triangleVertices[baseIndex + 1] = y1;
	triangleVertices[baseIndex + 2] = -5.0; // TODO: should be 0?

	triangleVertices[baseIndex + 7] = x2;
	triangleVertices[baseIndex + 8] = y2;
	triangleVertices[baseIndex + 9] = -5.0;

	triangleVertices[baseIndex + 14] = x3;
	triangleVertices[baseIndex + 15] = y3;
	triangleVertices[baseIndex + 16] = -5.0;
}

void Graphics2::ColoredShaderPainter::setTriColors(float opacity, uint color) {
	Color c = Color(color);
	float r = c.R;
	float g = c.G;
	float b = c.B;
	float a = c.A * opacity;

	int baseIndex = triangleBufferIndex * 7 * 3;
	triangleVertices[baseIndex + 3] = r;
	triangleVertices[baseIndex + 4] = g;
	triangleVertices[baseIndex + 5] = b;
	triangleVertices[baseIndex + 6] = a;

	triangleVertices[baseIndex + 10] = r;
	triangleVertices[baseIndex + 11] = g;
	triangleVertices[baseIndex + 12] = b;
	triangleVertices[baseIndex + 13] = a;

	triangleVertices[baseIndex + 17] = r;
	triangleVertices[baseIndex + 18] = g;
	triangleVertices[baseIndex + 19] = b;
	triangleVertices[baseIndex + 20] = a;
}

void Graphics2::ColoredShaderPainter::drawBuffer(bool trisDone) {
	if (!trisDone) endTris(true);

	rectVertexBuffer->unlock();

	Graphics4::setPipeline(myPipeline);
	Graphics4::setVertexBuffer(*rectVertexBuffer);
	Graphics4::setIndexBuffer(*indexBuffer);

    #ifndef KORE_G4
    // Set fixed-function projection matrix
    Graphics3::setProjectionMatrix(projectionMatrix);
    #else
    // Set shader matrix uniform
	Graphics4::setMatrix(projectionLocation, projectionMatrix);
    #endif

	Graphics4::drawIndexedVertices(0, bufferIndex * 2 * 3);

	bufferIndex = 0;
	rectVertices = rectVertexBuffer->lock();
}

void Graphics2::ColoredShaderPainter::drawTriBuffer(bool rectsDone) {
	if (!rectsDone) endRects(true);

	triangleVertexBuffer->unlock();

	Graphics4::setPipeline(myPipeline);
	Graphics4::setVertexBuffer(*triangleVertexBuffer);
	Graphics4::setIndexBuffer(*triangleIndexBuffer);

    #ifndef KORE_G4
    // Set fixed-function projection matrix
    Graphics3::setProjectionMatrix(projectionMatrix);
    #else
    // Set shader matrix uniform
	Graphics4::setMatrix(projectionLocation, projectionMatrix);
    #endif

	Graphics4::drawIndexedVertices(0, triangleBufferIndex * 3);

	triangleBufferIndex = 0;
	triangleVertices = triangleVertexBuffer->lock();
}

void Graphics2::ColoredShaderPainter::fillRect(float opacity, uint color, float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx,
                                    float toprighty, float bottomrightx, float bottomrighty) {
	if (triangleBufferIndex > 0) drawTriBuffer(true); // Flush other buffer for right render order

	if (bufferIndex + 1 >= bufferSize) drawBuffer(false);

	setRectColors(opacity, color);
	setRectVertices(bottomleftx, bottomlefty, topleftx, toplefty, toprightx, toprighty, bottomrightx, bottomrighty);
	++bufferIndex;
}

void Graphics2::ColoredShaderPainter::fillTriangle(float opacity, uint color, float x1, float y1, float x2, float y2, float x3, float y3) {
	if (bufferIndex > 0) drawBuffer(true); // Flush other buffer for right render order

	if (triangleBufferIndex + 1 >= triangleBufferSize) drawTriBuffer(false);

	setTriColors(opacity, color);
	setTriVertices(x1, y1, x2, y2, x3, y3);
	++triangleBufferIndex;
}

inline void Graphics2::ColoredShaderPainter::endTris(bool rectsDone) {
	if (triangleBufferIndex > 0) drawTriBuffer(rectsDone);
}

void Graphics2::ColoredShaderPainter::endRects(bool trisDone) {
	if (bufferIndex > 0) drawBuffer(trisDone);
}

void Graphics2::ColoredShaderPainter::end() {
	endTris(false);
	endRects(false);
}

Graphics2::ColoredShaderPainter::~ColoredShaderPainter() {
	delete shaderPipeline;
	delete rectVertexBuffer;
	delete indexBuffer;
	delete triangleVertexBuffer;
	delete triangleIndexBuffer;
}

//==========
// TextShaderPainter
//==========
Graphics2::TextShaderPainter::TextShaderPainter() : bufferSize(100), bufferIndex(0), vertexSize(9), bilinear(false), lastTexture(nullptr), shaderPipeline(nullptr) {
	initShaders();
	initBuffers();
}

Graphics4::PipelineState* Graphics2::TextShaderPainter::get_pipeline() const {
	return myPipeline;
}

void Graphics2::TextShaderPainter::set_pipeline(Graphics4::PipelineState* pipe) {
	if (pipe == nullptr) {
		projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
		textureLocation = shaderPipeline->getTextureUnit("tex");
	}
	else {
		projectionLocation = pipe->getConstantLocation("projectionMatrix");
		textureLocation = pipe->getTextureUnit("tex");
	}
	myPipeline = pipe;
}

void Graphics2::TextShaderPainter::setProjection(mat4 projectionMatrix) {
	this->projectionMatrix = projectionMatrix;
}

void Graphics2::TextShaderPainter::initShaders() {
	if (shaderPipeline != nullptr) return;

	structure.add("vertexPosition", Graphics4::Float3VertexData);
	structure.add("texPosition", Graphics4::Float2VertexData);
	structure.add("vertexColor", Graphics4::Float4VertexData);

	FileReader fs("painter-text.frag");
	FileReader vs("painter-text.vert");
	Graphics4::Shader* fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	Graphics4::Shader* vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);

	shaderPipeline = new Graphics4::PipelineState();
	shaderPipeline->fragmentShader = fragmentShader;
	shaderPipeline->vertexShader = vertexShader;

	shaderPipeline->blendSource = Graphics4::SourceAlpha;
	shaderPipeline->blendDestination = Graphics4::InverseSourceAlpha;
	shaderPipeline->alphaBlendSource = Graphics4::SourceAlpha;
	shaderPipeline->alphaBlendDestination = Graphics4::InverseSourceAlpha;

	shaderPipeline->inputLayout[0] = &structure;
	shaderPipeline->inputLayout[1] = nullptr;
	shaderPipeline->compile();

	projectionLocation = shaderPipeline->getConstantLocation("projectionMatrix");
	textureLocation = shaderPipeline->getTextureUnit("tex");
}

void Graphics2::TextShaderPainter::initBuffers() {
	rectVertexBuffer = new Graphics4::VertexBuffer(bufferSize * 4, structure);
	rectVertices = rectVertexBuffer->lock();

	indexBuffer = new Graphics4::IndexBuffer(bufferSize * 3 * 2);
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

void Graphics2::TextShaderPainter::setRectVertices(float bottomleftx, float bottomlefty, float topleftx, float toplefty, float toprightx, float toprighty,
                                        float bottomrightx, float bottomrighty) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 0] = bottomleftx;
	rectVertices[baseIndex + 1] = bottomlefty;
	rectVertices[baseIndex + 2] = -5.0f; // TODO: should be 0?

	rectVertices[baseIndex + 9] = topleftx;
	rectVertices[baseIndex + 10] = toplefty;
	rectVertices[baseIndex + 11] = -5.0;

	rectVertices[baseIndex + 18] = toprightx;
	rectVertices[baseIndex + 19] = toprighty;
	rectVertices[baseIndex + 20] = -5.0;

	rectVertices[baseIndex + 27] = bottomrightx;
	rectVertices[baseIndex + 28] = bottomrighty;
	rectVertices[baseIndex + 29] = -5.0;
}

void Graphics2::TextShaderPainter::setRectTexCoords(float left, float top, float right, float bottom) {
	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 3] = left;
	rectVertices[baseIndex + 4] = bottom;

	rectVertices[baseIndex + 12] = left;
	rectVertices[baseIndex + 13] = top;

	rectVertices[baseIndex + 21] = right;
	rectVertices[baseIndex + 22] = top;

	rectVertices[baseIndex + 30] = right;
	rectVertices[baseIndex + 31] = bottom;
}

void Graphics2::TextShaderPainter::setRectColors(float opacity, uint color) {
	Color c = Color(color);
	float r = c.R;
	float g = c.G;
	float b = c.B;
	float a = c.A * opacity;

	int baseIndex = bufferIndex * vertexSize * 4;
	rectVertices[baseIndex + 5] = r;
	rectVertices[baseIndex + 6] = g;
	rectVertices[baseIndex + 7] = b;
	rectVertices[baseIndex + 8] = a;

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

void Graphics2::TextShaderPainter::drawBuffer() {
	rectVertexBuffer->unlock();
	Graphics4::setPipeline(shaderPipeline);
	Graphics4::setVertexBuffer(*rectVertexBuffer);
	Graphics4::setIndexBuffer(*indexBuffer);
	Graphics4::setTexture(textureLocation, lastTexture);

    #ifndef KORE_G4
    // Set fixed-function projection matrix
    Graphics3::setProjectionMatrix(projectionMatrix);
    #else
    // Set shader matrix uniform
	Graphics4::setMatrix(projectionLocation, projectionMatrix);
    #endif

	Graphics4::setTextureAddressing(textureLocation, Graphics4::U, Graphics4::Clamp);
	Graphics4::setTextureAddressing(textureLocation, Graphics4::V, Graphics4::Clamp);
	Graphics4::setTextureMinificationFilter(textureLocation, bilinear ? Graphics4::LinearFilter : Graphics4::PointFilter);
	Graphics4::setTextureMagnificationFilter(textureLocation, bilinear ? Graphics4::LinearFilter : Graphics4::PointFilter);
	Graphics4::setTextureMipmapFilter(textureLocation, Graphics4::NoMipFilter);

	Graphics4::drawIndexedVertices(0, bufferIndex * 2 * 3);

	bufferIndex = 0;
	rectVertices = rectVertexBuffer->lock();
}

void Graphics2::TextShaderPainter::setBilinearFilter(bool bilinear) {
	end();
	this->bilinear = bilinear;
}

void Graphics2::TextShaderPainter::setFont(Kravur* font) {
	this->font = font;
}

// int TextShaderPainter::charCodeAt(int position) {}

// TODO: Make this fast
int Graphics2::TextShaderPainter::findIndex(int charcode, int* fontGlyphs, int glyphCount) {
	for (int index = 0; index < glyphCount; ++index) {
		if (fontGlyphs[index] == charcode) return index;
	}
	return -1;
}

void Graphics2::TextShaderPainter::drawString(const char* text, int start, int length, float opacity, uint color, float x, float y, const mat3& transformation, int* fontGlyphs) {
	Graphics4::Texture* tex = font->getTexture();
	if (lastTexture != nullptr && tex != lastTexture) drawBuffer();
	lastTexture = tex;

	float xpos = x;
	float ypos = y;
	for (int i = start; i < start + length; ++i) {
		AlignedQuad q = font->getBakedQuad(text[i] - 32, xpos, ypos);
		if (q.x0 >= 0) {
			if (bufferIndex + 1 >= bufferSize) drawBuffer();
			setRectColors(1.0f, color);
			setRectTexCoords(q.s0 * tex->width / tex->texWidth, q.t0 * tex->height / tex->texHeight, q.s1 * tex->width / tex->texWidth,
			                 q.t1 * tex->height / tex->texHeight);
			vec3 p0 = transformation * vec3(q.x0, q.y1, 1.0f); // bottom-left
			vec3 p1 = transformation * vec3(q.x0, q.y0, 1.0f); // top-left
			vec3 p2 = transformation * vec3(q.x1, q.y0, 1.0f); // top-right
			vec3 p3 = transformation * vec3(q.x1, q.y1, 1.0f); // bottom-right
			setRectVertices(p0.x(), p0.y(), p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
			xpos += q.xadvance;
			++bufferIndex;
		}
	}
}

void Graphics2::TextShaderPainter::end() {
	if (bufferIndex > 0) drawBuffer();
	lastTexture = nullptr;
}

Graphics2::TextShaderPainter::~TextShaderPainter() {
	delete shaderPipeline;
	delete rectVertexBuffer;
	delete indexBuffer;
}

//==========
// Graphics2
//==========

Graphics2::Graphics2::Graphics2(int width, int height, bool rTargets)
    : screenWidth(width), screenHeight(height), renderTargets(rTargets), color(Color::White), fontColor(Color::Black), fontSize(14), lastPipeline(nullptr) {
	transformation = mat3::Identity();
	opacity = 1.f;

	myImageScaleQuality = High;
	myMipmapScaleQuality = High;

	imagePainter = new ImageShaderPainter();
	coloredPainter = new ColoredShaderPainter();
	textPainter = new TextShaderPainter();
	textPainter->fontSize = fontSize;

	setProjection();

	initShaders();
}

void Graphics2::Graphics2::initShaders() {
	if (videoPipeline != nullptr) return;

	Graphics4::VertexStructure structure;
	structure.add("vertexPosition", Graphics4::Float3VertexData);
	structure.add("texPosition", Graphics4::Float2VertexData);
	structure.add("vertexColor", Graphics4::Float4VertexData);

	FileReader fs("painter-video.frag");
	FileReader vs("painter-video.vert");
	Graphics4::Shader* fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	Graphics4::Shader* vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);

	videoPipeline = new Graphics4::PipelineState();
	videoPipeline->fragmentShader = fragmentShader;
	videoPipeline->vertexShader = vertexShader;

	videoPipeline->inputLayout[0] = &structure;
	videoPipeline->inputLayout[1] = nullptr;
	videoPipeline->compile();
}

int Graphics2::Graphics2::upperPowerOfTwo(int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
};

void Graphics2::Graphics2::setProjection() {
	int width = screenWidth;
	int height = screenHeight;

	if (!renderTargets) {
		projectionMatrix = mat4::orthogonalProjection(0, static_cast<float>(width), static_cast<float>(height), 0, 0.1f, 1000);
	}
	else {
		if (!Graphics4::nonPow2TexturesSupported()) {
			width = upperPowerOfTwo(width);
			height = upperPowerOfTwo(height);
		}
		if (Graphics4::renderTargetsInvertedY()) {
			projectionMatrix = mat4::orthogonalProjection(0, static_cast<float>(width), 0, static_cast<float>(height), 0.1f, 1000);
		}
		else {
			projectionMatrix = mat4::orthogonalProjection(0, static_cast<float>(width), static_cast<float>(height), 0, 0.1f, 1000);
		}
	}

	imagePainter->setProjection(projectionMatrix);
	coloredPainter->setProjection(projectionMatrix);
	textPainter->setProjection(projectionMatrix);
}

void Graphics2::Graphics2::drawImage(Graphics4::Texture* img, float x, float y) {
	coloredPainter->end();
	textPainter->end();

	float xw = x + img->width;
	float yh = y + img->height;

	float32x4 xx = load(x, x, xw, xw);
	float32x4 yy = load(yh, y, y, yh);

	float32x4 _00 = loadAll(transformation.get(0, 0));
	float32x4 _01 = loadAll(transformation.get(1, 0));
	float32x4 _02 = loadAll(transformation.get(2, 0));
	float32x4 _10 = loadAll(transformation.get(0, 1));
	float32x4 _11 = loadAll(transformation.get(1, 1));
	float32x4 _12 = loadAll(transformation.get(2, 1));
	float32x4 _20 = loadAll(transformation.get(0, 2));
	float32x4 _21 = loadAll(transformation.get(1, 2));
	float32x4 _22 = loadAll(transformation.get(2, 2));

	// matrix multiply
	float32x4 w = add(add(mul(_02, xx), mul(_12, yy)), _22);
	float32x4 px = div(add(add(mul(_00, xx), mul(_10, yy)), _20), w);
	float32x4 py = div(add(add(mul(_01, xx), mul(_11, yy)), _21), w);

	imagePainter->drawImage(img, get(px, 0), get(py, 0), get(px, 1), get(py, 1), get(px, 2), get(py, 2), get(px, 3), get(py, 3), opacity, color);
}

void Graphics2::Graphics2::drawScaledSubImage(Graphics4::Texture* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	coloredPainter->end();
	textPainter->end();
	vec2 p1 = transformation * vec3(dx, dy + dh, 1.0f);
	vec2 p2 = transformation * vec3(dx, dy, 1.0f);
	vec2 p3 = transformation * vec3(dx + dw, dy, 1.0f);
	vec2 p4 = transformation * vec3(dx + dw, dy + dh, 1.0f);

	imagePainter->drawImage2(img, sx, sy, sw, sh, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), opacity, color);
}

void Graphics2::Graphics2::drawImage(Graphics4::RenderTarget* img, float x, float y) {
	coloredPainter->end();
	textPainter->end();

	float xw = x + img->width;
	float yh = y + img->height;

	float32x4 xx = load(x, x, xw, xw);
	float32x4 yy = load(yh, y, y, yh);

	float32x4 _00 = loadAll(transformation.get(0, 0));
	float32x4 _01 = loadAll(transformation.get(1, 0));
	float32x4 _02 = loadAll(transformation.get(2, 0));
	float32x4 _10 = loadAll(transformation.get(0, 1));
	float32x4 _11 = loadAll(transformation.get(1, 1));
	float32x4 _12 = loadAll(transformation.get(2, 1));
	float32x4 _20 = loadAll(transformation.get(0, 2));
	float32x4 _21 = loadAll(transformation.get(1, 2));
	float32x4 _22 = loadAll(transformation.get(2, 2));

	// matrix multiply
	float32x4 w = add(add(mul(_02, xx), mul(_12, yy)), _22);
	float32x4 px = div(add(add(mul(_00, xx), mul(_10, yy)), _20), w);
	float32x4 py = div(add(add(mul(_01, xx), mul(_11, yy)), _21), w);

	imagePainter->drawImage(img, get(px, 0), get(py, 0), get(px, 1), get(py, 1), get(px, 2), get(py, 2), get(px, 3), get(py, 3), opacity, color);
}

void Graphics2::Graphics2::drawScaledSubImage(Graphics4::RenderTarget* img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	coloredPainter->end();
	textPainter->end();
	vec2 p1 = transformation * vec3(dx, dy + dh, 1.0f);
	vec2 p2 = transformation * vec3(dx, dy, 1.0f);
	vec2 p3 = transformation * vec3(dx + dw, dy, 1.0f);
	vec2 p4 = transformation * vec3(dx + dw, dy + dh, 1.0f);

	imagePainter->drawImage2(img, sx, sy, sw, sh, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), opacity, color);
}

void Graphics2::Graphics2::drawRect(float x, float y, float width, float height, float strength) {
	imagePainter->end();
	textPainter->end();

	vec2 p1 = transformation * vec3(x - strength / 2, y + strength / 2, 1.0f);                                // bottom-left
	vec2 p2 = transformation * vec3(x - strength / 2, y - strength / 2, 1.0f);                                // top-left
	vec2 p3 = transformation * vec3(x + width + strength / 2, y - strength / 2, 1.0f);                        // top-right
	vec2 p4 = transformation * vec3(x + width + strength / 2, y + strength / 2, 1.0f);                        // bottom-right
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

void Graphics2::Graphics2::fillRect(float x, float y, float width, float height) {
	imagePainter->end();
	textPainter->end();

	vec2 p1 = transformation * vec3(x, y + height, 1.0f);
	vec2 p2 = transformation * vec3(x, y, 1.0f);
	vec2 p3 = transformation * vec3(x + width, y, 1.0f);
	vec2 p4 = transformation * vec3(x + width, y + height, 1.0f);
	coloredPainter->fillRect(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y());
}

void Graphics2::Graphics2::drawString(const char* text, float x, float y) {
	drawString(text, 0, strlen(text), x, y);
}


void Graphics2::Graphics2::drawString(const char* text, int length, float x, float y) {
	drawString(text, 0, length, x, y);
}

void Graphics2::Graphics2::drawString(const char* text, int start, int length, float x, float y) {
	imagePainter->end();
	coloredPainter->end();

	textPainter->drawString(text, start, length, opacity, fontColor, x, y, transformation, fontGlyphs);
}

void Graphics2::Graphics2::drawLine(float x1, float y1, float x2, float y2, float strength) {
	imagePainter->end();
	textPainter->end();

	vec3 vec;
	if (y2 == y1)
		vec = vec3(0.0f, -1.0f, 1.0f);
	else
		vec = vec3(1.0f, -(x2 - x1) / (y2 - y1), 1.0f);
	vec.setLength(strength);
	vec3 p1 = vec3(x1 + 0.5f * vec.x(), y1 + 0.5f * vec.y(), 1.0f);
	vec3 p2 = vec3(x2 + 0.5f * vec.x(), y2 + 0.5f * vec.y(), 1.0f);
	vec3 p3 = vec3(p1.x() - vec.x(), p1.y() - vec.y(), 1.0f);
	vec3 p4 = vec3(p2.x() - vec.x(), p2.y() - vec.y(), 1.0f);

	p1 = transformation * p1;
	p2 = transformation * p2;
	p3 = transformation * p3;
	p4 = transformation * p4;

	coloredPainter->fillTriangle(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
	coloredPainter->fillTriangle(opacity, color, p3.x(), p3.y(), p2.x(), p2.y(), p4.x(), p4.y());
}

void Graphics2::Graphics2::fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	imagePainter->end();
	textPainter->end();

	vec2 p1 = transformation * vec3(x1, y1, 1.0f);
	vec2 p2 = transformation * vec3(x2, y2, 1.0f);
	vec2 p3 = transformation * vec3(x3, y3, 1.0f);
	coloredPainter->fillTriangle(opacity, color, p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
}

Graphics2::ImageScaleQuality Graphics2::Graphics2::getImageScaleQuality() const {
	return myImageScaleQuality;
}

void Graphics2::Graphics2::setImageScaleQuality(Kore::Graphics2::ImageScaleQuality value) {
	imagePainter->setBilinearFilter(value == High);
	textPainter->setBilinearFilter(value == High);
	myImageScaleQuality = value;
}

Graphics2::ImageScaleQuality Graphics2::Graphics2::getMipmapScaleQuality() const {
	return myMipmapScaleQuality;
}

void Graphics2::Graphics2::setMipmapScaleQuality(Kore::Graphics2::ImageScaleQuality value) {
	imagePainter->setBilinearMipmapFilter(value == High);
	// textPainter->setBilinearMipmapFilter(value == High); // TODO (DK) implement for fonts as well?
	myMipmapScaleQuality = value;
}

void Graphics2::Graphics2::setPipeline(Graphics4::PipelineState* pipeline) {
	if (pipeline != lastPipeline) {
		flush();
		imagePainter->set_pipeline(pipeline);
		coloredPainter->set_pipeline(pipeline);
		textPainter->set_pipeline(pipeline);
		lastPipeline = pipeline;
	}
}

void Graphics2::Graphics2::scissor(int x, int y, int width, int height) {
	Graphics2::flush();
	Graphics4::scissor(x, y, width, height);
}

void Graphics2::Graphics2::disableScissor() {
	Graphics2::flush();
	Graphics4::disableScissor();
}

void Graphics2::Graphics2::begin(bool renderTargets, int width, int height, bool clear, uint clearColor) {
	//    Graphics::begin();
	if (width > 0) {
		screenWidth = width;
		screenHeight = height;
	}
	this->renderTargets = renderTargets;
	if (clear) Graphics2::clear(clearColor);
	setProjection();
}

void Graphics2::Graphics2::clear(uint color) {
	Graphics4::clear(Graphics4::ClearColorFlag, color);
}

void Graphics2::Graphics2::flush() {
	imagePainter->end();
	textPainter->end();
	coloredPainter->end();
}

void Graphics2::Graphics2::end() {
	flush();
	//    Graphics::end();
}

void Graphics2::Graphics2::drawVideoInternal(/*Video video,*/ float x, float y, float width, float height) {}

void Graphics2::Graphics2::drawVideo(/*Video video,*/ float x, float y, float width, float height) {
	//    setPipeline(videoPipeline);
	drawVideoInternal(/*video,*/ x, y, width, height);
	//    setPipeline(nullptr);
}

uint Graphics2::Graphics2::getColor() const {
	return color;
}

void Graphics2::Graphics2::setColor(uint color) {
	this->color = color;
}

float Graphics2::Graphics2::getOpacity() const {
	return opacity;
}

void Graphics2::Graphics2::setOpacity(float opacity) {
	this->opacity = opacity;
}

Kravur* Graphics2::Graphics2::getFont() const {
	return font;
}

void Graphics2::Graphics2::setFont(Kravur* font) {
	textPainter->setFont(font);
	this->font = font;
}

int Graphics2::Graphics2::getFontSize() const {
	return fontSize;
}

void Graphics2::Graphics2::setFontSize(int value) {
	this->fontSize = value;
}

uint Graphics2::Graphics2::getFontColor() const {
	return fontColor;
}

void Graphics2::Graphics2::setFontColor(uint color) {
	this->fontColor = color;
}

Graphics2::Graphics2::~Graphics2() {
	delete imagePainter;
	delete coloredPainter;
	delete textPainter;
	delete videoPipeline;
}

#endif
