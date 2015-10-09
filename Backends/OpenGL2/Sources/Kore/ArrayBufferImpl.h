#pragma once

namespace Kore {
	class ArrayBuffer;
	
	class ArrayBufferImpl {
	protected:
		ArrayBufferImpl(int indexCount, int structureSize, int structureCount);

		void unset();
		float* data;
		int mySize;
		int structureSize;
		int structureCount;
		uint bufferId;

	public:
		static ArrayBuffer* current;
	};
}
