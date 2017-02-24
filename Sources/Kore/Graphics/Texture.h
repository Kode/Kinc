#pragma once

#include <Kore/Graphics/Image.h>
#include <Kore/TextureImpl.h>

namespace Kore {
	class TextureUnit : public TextureUnitImpl {};

	class Texture : public Image, public TextureImpl {
	public:
		Texture(int width, int height, Format format, bool readable);
		Texture(int width, int height, int depth, Format format, bool readable = false);
		Texture(Kore::Reader& reader, const char* format, bool readable = false);
		Texture(const char* filename, bool readable = false);
		Texture(void* data, int size, const char* format, bool readable = false);
#ifdef SYS_ANDROID
		Texture(unsigned texid);
#endif
		void _set(TextureUnit unit);
		u8* lock();
		void unlock();
		void clear(int x, int y, int z, int width, int height, int depth, uint color);
#ifdef SYS_IOS
		void upload(u8* data);
#endif
		void generateMipmaps(int levels);
		void setMipmap(Texture* mipmap, int level);

		int stride();
		int texWidth;
		int texHeight;
	private:
		void init(const char* format, bool readable = false);
	};
}
