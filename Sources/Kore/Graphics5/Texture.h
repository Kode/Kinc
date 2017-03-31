#pragma once

#include <Kore/Graphics1/Image.h>
#include <Kore/Texture5Impl.h>

namespace Kore {
	namespace Graphics5 {
		typedef Graphics1::Image Image;

		class TextureUnit : public TextureUnit5Impl {};

		class Texture : public Image, public Texture5Impl {
		public:
			Texture(int width, int height, Format format, bool readable);
			Texture(int width, int height, int depth, Format format, bool readable = false);
			Texture(Kore::Reader& reader, const char* format, bool readable = false);
			Texture(const char* filename, bool readable = false);
			Texture(void* data, int size, const char* format, bool readable = false);
			Texture(void* data, int width, int height, int format, bool readable = false);
#ifdef KORE_ANDROID
			Texture(unsigned texid);
#endif
			void _set(TextureUnit unit);
			void _setImage(TextureUnit unit);
			u8* lock();
			void unlock();
			void clear(int x, int y, int z, int width, int height, int depth, uint color);
#ifdef KORE_IOS
			void upload(u8* data);
#endif
			void generateMipmaps(int levels);
			void setMipmap(Texture* mipmap, int level);

			int stride();
			int texWidth;
			int texHeight;

			void _init(const char* format, bool readable = false);
		};
	}
}
