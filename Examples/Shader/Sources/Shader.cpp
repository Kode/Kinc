#include "pch.h"

#include <Kore/Application.h>
#include <Kore/Files/File.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Shader.h>
#include <limits>

using namespace Kore;

namespace {
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;
	VertexBuffer* vertices;
	IndexBuffer* indices;

	void update() {
		Graphics::begin();
		Graphics::clear(0);

		program->set();
		vertices->set();
		indices->set();
		Graphics::drawIndexedVertices();

		Graphics::end();
		Graphics::swapBuffers();
	}
}

int kore(int argc, char** argv) {
	Application app(argc, argv, 1024, 768, false, "ShaderTest");
	app.setCallback(update);

	DiskFile vs; vs.open("shader.vert", DiskFile::ReadMode);
	DiskFile fs; fs.open("shader.frag", DiskFile::ReadMode);
	vertexShader = new Shader(vs.readAll(), vs.getSize(), VertexShader);
	fragmentShader = new Shader(fs.readAll(), fs.getSize(), FragmentShader);
	VertexStructure structure;
	structure.add("pos", Float3VertexData);
	program = new Program;
	program->setVertexShader(vertexShader);
	program->setFragmentShader(fragmentShader);
	program->link(structure);
	
	vertices = new VertexBuffer(3, structure);
	float* v = vertices->lock();
	v[0] = -1; v[1] = -1; v[2] = 0.5;
	v[3] = 1;  v[4] = -1; v[5] = 0.5;
	v[6] = -1; v[7] = 1;  v[8] = 0.5;
	vertices->unlock();
	
	indices = new IndexBuffer(3);
	int* i = indices->lock();
	i[0] = 0; i[1] = 1; i[2] = 2;
	indices->unlock();

	app.start();

	return 0;
}
