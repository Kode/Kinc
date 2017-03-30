#include "pch.h"

#include "Kravur.h"

#include <Kore/IO/FileReader.h>
#include <map>
#include <sstream>
#include <string.h>

using namespace Kore;

namespace {
	std::map<std::string, Kravur*> fontCache;

	std::string createKey(const char* name, FontStyle style, float size) {
		std::stringstream key;
		key << name;
		if (style.bold) {
			key << "#Bold";
		}
		if (style.italic) {
			key << "#Italic";
		}
		key << size;
		key << ".kravur";
		return key.str();
	}
}

Kravur* Kravur::load(const char* name, FontStyle style, float size) {
	std::string key = createKey(name, style, size);
	Kravur* kravur = fontCache[key];
	if (kravur == nullptr) {
		FileReader reader(key.c_str());
		kravur = new Kravur(&reader);
		kravur->name = name;
		kravur->style = style;
		kravur->size = size;

		fontCache[key] = kravur;

		return kravur;
	}
	else {
		return kravur;
	}
}

Kravur::Kravur(Reader* reader) {
	reader->readS32LE(); // size
	int ascent = reader->readS32LE();
	reader->readS32LE(); // descent
	reader->readS32LE(); // lineGap
	baseline = static_cast<float>(ascent);
	for (int i = 0; i < 256 - 32; ++i) {
		BakedChar c;
		c.x0 = reader->readS16LE();
		c.y0 = reader->readS16LE();
		c.x1 = reader->readS16LE();
		c.y1 = reader->readS16LE();
		c.xoff = reader->readF32LE();
		c.yoff = reader->readF32LE() + baseline;
		c.xadvance = reader->readF32LE();
		chars.push_back(c);
	}
	width = reader->readS32LE();
	height = reader->readS32LE();
	int w = width;
	int h = height;
	while (w > 4096 || h > 4096) {
		reader->seek(reader->pos() + h * w);
		w = w / 2;
		h = h / 2;
	}
	texture = new Graphics4::Texture(w, h, Image::Grey8, true);
	u8* bytes = texture->lock();
	int pos = 0;
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x) {
			bytes[pos] = reader->readU8();

			// filter-test
			// if ((x + y) % 2 == 0) bytes.set(pos, 0xff);
			// else bytes.set(pos, 0);

			++pos;
		}
	texture->unlock();
	reader->seek(0);
}

Graphics4::Texture* Kravur::getTexture() {
	return texture;
}

AlignedQuad Kravur::getBakedQuad(int char_index, float xpos, float ypos) {
	if (char_index >= static_cast<int>(chars.size())) return AlignedQuad();
	float ipw = 1.0f / width;
	float iph = 1.0f / height;
	BakedChar b = chars[char_index];
	if (b.x0 < 0) return AlignedQuad();
	int round_x = static_cast<int>(Kore::round(xpos + b.xoff));
	int round_y = static_cast<int>(Kore::round(ypos + b.yoff));

	AlignedQuad q;
	q.x0 = static_cast<float>(round_x);
	q.y0 = static_cast<float>(round_y);
	q.x1 = static_cast<float>(round_x + b.x1 - b.x0);
	q.y1 = static_cast<float>(round_y + b.y1 - b.y0);

	q.s0 = b.x0 * ipw;
	q.t0 = b.y0 * iph;
	q.s1 = b.x1 * ipw;
	q.t1 = b.y1 * iph;

	q.xadvance = b.xadvance;

	return q;
}

float Kravur::getCharWidth(int charIndex) {
	if (charIndex < 32) return 0;
	if (charIndex - 32 >= static_cast<int>(chars.size())) return 0;
	return chars[charIndex - 32].xadvance;
}

float Kravur::getHeight() {
	return size;
}

float Kravur::charWidth(char ch) {
	return getCharWidth(ch);
}

float Kravur::charsWidth(const char* ch, int offset, int length) {
	return stringWidth(&ch[offset], length);
}

float Kravur::stringWidth(const char* string, int length) {
	float width = 0;
	if (length < 0) length = (int)strlen(string);
	for (int c = 0; c < length; ++c) {
		width += getCharWidth(string[c]);
	}
	return width;
}

float Kravur::getBaselinePosition() {
	return baseline;
}
