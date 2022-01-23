#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/textureunit.h>

#include <Kore/Graphics1/Image.h>

namespace Kore {
	namespace Graphics4 {
		typedef Graphics1::Image Image;

		class TextureUnit {
		public:
			TextureUnit() {}

			kinc_g4_texture_unit_t kincUnit;
		};

		class Texture : public Image {
		public:
			Texture(kinc_g4_texture_t texture);
			Texture(int width, int height, Format format, bool readable = false);
			Texture(int width, int height, int depth, Format format, bool readable = false);
			Texture(Kore::Reader &reader, const char *format, bool readable = false);
			Texture(const char *filename, bool readable = false);
			Texture(void *data, int size, const char *format, bool readable = false);
			Texture(void *data, int width, int height, Format format, bool readable = false);
			Texture(void *data, int width, int height, int depth, Format format, bool readable = false);
			virtual ~Texture();
#if defined(KORE_ANDROID) && !defined(KORE_VULKAN)
			Texture(unsigned texid);
#endif
			void _set(TextureUnit unit);
			void _setImage(TextureUnit unit);
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

			kinc_g4_texture_t kincTexture;

		private:
			// void init(const char* format, bool readable = false);
			// void init3D(bool readable = false);
		};
	}
}
