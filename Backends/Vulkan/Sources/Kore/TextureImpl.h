#pragma once

#include <Kore/Graphics/Image.h>
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

	class Texture;

	class TextureUnitImpl {
	public:
		int binding;
	};
	
	class TextureImpl {
	protected:
		//static TreeMap<Image, Texture*> images;
	public:
		u8 pixfmt;

		texture_object texture;
		VkDeviceSize deviceSize;

		VkDescriptorSet desc_set;

		~TextureImpl();
		u8* conversionBuffer; // Fuer wenn Textur aus Image erstellt wird
	};
}
