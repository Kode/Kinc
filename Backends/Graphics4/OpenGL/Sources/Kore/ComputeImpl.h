#pragma once

namespace Kore {
	class ComputeConstantLocationImpl {
	public:
		int location;
		unsigned int type;
	};

	class ComputeTextureUnitImpl {
	public:
		int unit;
	};

	class ComputeShaderImpl {
	public:
		ComputeShaderImpl(void* source, int length);
		virtual ~ComputeShaderImpl();
		int findTexture(const char* name);
		char** textures;
		int* textureValues;
		int textureCount;
		uint _id;
		uint _programid;
		char* _source;
		int _length;
	};
}
