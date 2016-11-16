#pragma once

#include <Kore/Graphics/Graphics.h>
#include <Kore/IO/Reader.h>
#include <vector>

struct FontStyle {
	bool bold;
	bool italic;
	bool underlined;

	FontStyle() : bold(false), italic(false), underlined(false) {}
	FontStyle(bool bold, bool italic, bool underlined) : bold(bold), italic(italic), underlined(underlined) {}
};

struct BakedChar {
	BakedChar() {
		x0 = -1;
	}

	// coordinates of bbox in bitmap
	int x0;
	int y0;
	int x1;
	int y1;

	float xoff;
	float yoff;
	float xadvance;
};

struct AlignedQuad {
	AlignedQuad() {
		x0 = -1;
	}

	// top-left
	float x0;
	float y0;
	float s0;
	float t0;

	// bottom-right
	float x1;
	float y1;
	float s1;
	float t1;

	float xadvance;
};

namespace Kore {
	class Kravur {
	private:
		Kravur(Kore::Reader* reader);

		const char* name;
		FontStyle style;
		float size;

		float mySize;
		std::vector<BakedChar> chars;
		Texture* texture;
		float baseline;
		float getCharWidth(int charIndex);
		float charWidth(char ch);

	public:
		int width;
		int height;

		static Kravur* load(const char* name, FontStyle style, float size);

		Texture* getTexture();
		AlignedQuad getBakedQuad(int char_index, float xpos, float ypos);

		float getHeight();
		float charsWidth(const char* ch, int offset, int length);
		float stringWidth(const char* string, int length = -1);
		float getBaselinePosition();
	};
}
