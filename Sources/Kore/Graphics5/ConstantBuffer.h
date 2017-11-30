#pragma once

#include <Kore/ConstantBuffer5Impl.h>
#include <Kore/Math/Matrix.h>
#include <Kore/Math/Vector.h>

namespace Kore {
	namespace Graphics5 {
		class ConstantBuffer : public ConstantBuffer5Impl {
		public:
			ConstantBuffer(int size);
			~ConstantBuffer();
			void lock();
			void lock(int start, int count);
			void unlock();
			int size();

			void setBool(int offset, bool value);
			void setInt(int offset, int value);
			void setFloat(int offset, float value);
			void setFloat2(int offset, float value1, float value2);
			void setFloat2(int offset, vec2 value);
			void setFloat3(int offset, float value1, float value2, float value3);
			void setFloat3(int offset, vec3 value);
			void setFloat4(int offset, float value1, float value2, float value3, float value4);
			void setFloat4(int offset, vec4 value);
			void setFloats(int offset, float* values, int count);
			void setMatrix(int offset, const mat3& value);
			void setMatrix(int offset, const mat4& value);
		private:
			u8* data;
		};
	}
}
