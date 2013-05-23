#pragma once

namespace Kore {
	class Image {
	public:
		enum Format {
			RGBA32,
			Grey8
		};
		
		static int sizeOf(Image::Format format);

		Image(int width, int height, Format format);
		Image(const char* filename);
		virtual ~Image();

		int width, height;
		Format format;
		void* data;
	};
}
