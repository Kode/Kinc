#pragma once

#include <Kore/Graphics1/Image.h>
#include <vulkan/vulkan.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef RegisterClass
#undef RegisterClass
#endif

namespace Kore {
	struct texture_object {
		VkSampler sampler;

		VkImage image;
		VkImageLayout imageLayout;

		VkDeviceMemory mem;
		VkImageView view;
		int32_t tex_width, tex_height;
	};

	namespace Graphics5 {
		class Texture;
	}

	class TextureUnit5Impl {
	public:
		int binding;
	};

	class Texture5Impl {
	protected:
		// static TreeMap<Image, Texture*> images;
	public:
		u8 pixfmt;

		texture_object texture;
		VkDeviceSize deviceSize;

		VkDescriptorSet desc_set;

		~Texture5Impl();
		u8* conversionBuffer; // Fuer wenn Textur aus Image erstellt wird
	};
}
