#include "pch.h"

#include "Graphics.h"

#include <Kore/IO/FileReader.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/IO/FileReader.h>
#include <limits>

using namespace Kore;

namespace {
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	Graphics4::TextureUnit tex;
	Graphics4::VertexBuffer* vb;
	Graphics4::IndexBuffer* ib;
	Graphics4::Texture* texture;
	int* image;
	int w, h;
}

void Graphics1::begin() {
	Graphics4::begin();
	image = (int*)texture->lock();
}

void Graphics1::setPixel(int x, int y, float red, float green, float blue) {
	if (x < 0 || x >= w || y < 0 || y >= h) return;
	int r = (int)(red * 255);
	int g = (int)(green * 255);
	int b = (int)(blue * 255);
	image[y * texture->texWidth + x] = 0xff << 24 | b << 16 | g << 8 | r;
}

void Graphics1::end() {
	texture->unlock();

	Graphics4::clear(Graphics4::ClearColorFlag, 0xff000000);

	Graphics4::setPipeline(pipeline);
	Graphics4::setTexture(tex, texture);
	Graphics4::setVertexBuffer(*vb);
	Graphics4::setIndexBuffer(*ib);
	Graphics4::drawIndexedVertices();

	Graphics4::end();
	Graphics4::swapBuffers();
}

void Graphics1::init(int width, int height) {
	w = width;
	h = height;
	FileReader vs("g1.vert");
	FileReader fs("g1.frag");
	vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
	fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);
	Graphics4::VertexStructure structure;
	structure.add("pos", Graphics4::Float3VertexData);
	structure.add("tex", Graphics4::Float2VertexData);
	pipeline = new Graphics4::PipelineState;
	pipeline->inputLayout[0] = &structure;
	pipeline->inputLayout[1] = nullptr;
	pipeline->vertexShader = vertexShader;
	pipeline->fragmentShader = fragmentShader;
	pipeline->compile();

	tex = pipeline->getTextureUnit("tex");

	texture = new Graphics4::Texture(width, height, Image::RGBA32, false);
	image = (int*)texture->lock();
	for (int y = 0; y < texture->texHeight; ++y) {
		for (int x = 0; x < texture->texWidth; ++x) {
			image[y * texture->texWidth + x] = 0;
		}
	}
	texture->unlock();

	// Correct for the difference between the texture's desired size and the actual power of 2 size
	float xAspect = (float)texture->width / texture->texWidth;
	float yAspect = (float)texture->height / texture->texHeight;
	
	vb = new Graphics4::VertexBuffer(4, structure, 0);
	float* v = vb->lock();
	{
		int i = 0;
		v[i++] = -1; v[i++] = 1; v[i++] = 0.5; v[i++] = 0; v[i++] = 0;
		v[i++] = 1;  v[i++] = 1; v[i++] = 0.5; v[i++] = xAspect; v[i++] = 0;
		v[i++] = 1; v[i++] = -1;  v[i++] = 0.5; v[i++] = xAspect; v[i++] = yAspect;
		v[i++] = -1; v[i++] = -1;  v[i++] = 0.5; v[i++] = 0; v[i++] = yAspect;
	}
	vb->unlock();

	ib = new Graphics4::IndexBuffer(6);
	int* ii = ib->lock();
	{
		int i = 0;
		ii[i++] = 0; ii[i++] = 1; ii[i++] = 3;
		ii[i++] = 1; ii[i++] = 2; ii[i++] = 3;
	}
	ib->unlock();
}

int Graphics1::width() {
	return w;
}

int Graphics1::height() {
	return h;
}
