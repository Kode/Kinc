#pragma once

namespace Kore {
	namespace Graphics4 {
		enum VertexData {
			VertexDataNone = 0,
			VertexDataF32x1 = 1,
			VertexDataF32x2 = 2,
			VertexDataF32x3 = 3,
			VertexDataF32x4 = 4,
			VertexDataF32x4x4 = 5,
			VertexDataI8x1 = 6,
			VertexDataU8x1 = 7,
			VertexDataI8x1Normalized = 8,
			VertexDataU8x1Normalized = 9,
			VertexDataI8x2 = 10,
			VertexDataU8x2 = 11,
			VertexDataI8x2Normalized = 12,
			VertexDataU8x2Normalized = 13,
			VertexDataI8x4 = 14,
			VertexDataU8x4 = 15,
			VertexDataI8x4Normalized = 16,
			VertexDataU8x4Normalized = 17,
			VertexDataI16x1 = 18,
			VertexDataU16x1 = 19,
			VertexDataI16x1Normalized = 20,
			VertexDataU16x1Normalized = 21,
			VertexDataI16x2 = 22,
			VertexDataU16x2 = 23,
			VertexDataI16x2Normalized = 24,
			VertexDataU16x2Normalized = 25,
			VertexDataI16x4 = 26,
			VertexDataU16x4 = 27,
			VertexDataI16x4Normalized = 28,
			VertexDataU16x4Normalized = 29,
			VertexDataI32x1 = 30,
			VertexDataU32x1 = 31,
			VertexDataI32x2 = 32,
			VertexDataU32x2 = 33,
			VertexDataI32x3 = 34,
			VertexDataU32x3 = 35,
			VertexDataI32x4 = 36,
			VertexDataU32x4 = 37,

			// deprecated
			NoVertexData = VertexDataNone,
			Float1VertexData = VertexDataF32x1,
			Float2VertexData = VertexDataF32x2,
			Float3VertexData = VertexDataF32x3,
			Float4VertexData = VertexDataF32x4,
			Float4x4VertexData = VertexDataF32x4x4,
			Short2NormVertexData = VertexDataI16x2Normalized,
			Short4NormVertexData = VertexDataI16x4Normalized,
			ColorVertexData = VertexDataU8x4Normalized
		};

		enum Usage { StaticUsage, DynamicUsage, ReadableUsage };

		// Fixed-function vertex attributes
		enum VertexAttribute {
			NoVertexAttribute,
			VertexCoord,
			VertexNormal,
			VertexColor0,
			VertexColor1,
			VertexTexCoord0,
			VertexTexCoord1,
			VertexTexCoord2,
			VertexTexCoord3,
			VertexTexCoord4,
			VertexTexCoord5,
			VertexTexCoord6,
			VertexTexCoord7,
		};

		class VertexElement {
		public:
			const char *name;
			VertexAttribute attribute; // for fixed-function (OpenGL 1.x)
			VertexData data;

			VertexElement() : name(nullptr), data(NoVertexData) {}

			VertexElement(const char *name, VertexData data) : name(name), data(data) {}

			VertexElement(VertexAttribute attribute, VertexData data) : name(""), attribute(attribute), data(data) {}
		};

		class VertexStructure {
		public:
			const static int maxElementsCount = 16;
			VertexElement elements[maxElementsCount];
			int size;
			bool instanced;

			VertexStructure() : size(0), instanced(false) {}

			void add(const char *name, VertexData data) {
				elements[size++] = VertexElement(name, data);
			}

			void add(VertexAttribute attribute, VertexData data) {
				elements[size++] = VertexElement(attribute, data);
			}
		};
	}
}
