#pragma once

#include <Kore/Graphics1/Image.h>
#include <kinc/graphics5/texture.h>

namespace Kore {
	namespace Graphics5 {
		typedef Graphics1::Image Image;

		class TextureUnit {
		public:
			kinc_g5_texture_unit_t kincTextureUnit;
		};

		class Texture : public Image {
		public:
			Texture(int width, int height, Format format, bool readable);
			Texture(int width, int height, int depth, Format format, bool readable = false);
			Texture(Kore::Reader &reader, const char *format, bool readable = false);
			Texture(const char *filename, bool readable = false);
			// Texture(void *data, int size, const char *format, bool readable = false);
			Texture(void *data, int width, int height, int format, bool readable = false);
			Texture(void *data, int width, int height, int depth, int format, bool readable = false);
			virtual ~Texture();
#if defined(KORE_ANDROID) && !defined(KORE_VULKAN)
			Texture(unsigned texid);
#endif
			// void _set(TextureUnit unit);
			// void _setImage(TextureUnit unit);
			u8 *lock();
			void unlock();
			void clear(int x, int y, int z, int width, int height, int depth, uint color);
#if defined(KORE_IOS) || defined(KORE_MACOS)
			void upload(u8 *data, int stride);
#endif
			void generateMipmaps(int levels);
			void setMipmap(Texture *mipmap, int level);

			int stride();
			int texWidth;
			int texHeight;
			int texDepth;

			// void _init(const char* format, bool readable = false);

			kinc_g5_texture_t kincTexture;
		};
	}
}
