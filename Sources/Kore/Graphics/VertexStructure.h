#pragma once

namespace Kore {
	enum VertexData {
		NoVertexData,
		Float1VertexData,
		Float2VertexData,
		Float3VertexData,
		Float4VertexData,
		Float4x4VertexData,
		ColorVertexData
	};

	class VertexElement {
	public:
		const char* name;
		VertexData data;

		VertexElement() : name(nullptr), data(NoVertexData) { }
	
		VertexElement(const char* name, VertexData data) : name(name), data(data) { }
	};

	class VertexStructure {
	public:
		const static int maxElementsCount = 16;
		VertexElement elements[maxElementsCount];
		int size;
	
		VertexStructure() : size(0) { }

		void add(const char* name, VertexData data) {
			elements[size++] = VertexElement(name, data);
		}
	};
}
