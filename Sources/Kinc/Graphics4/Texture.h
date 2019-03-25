#pragma once

#include <Kore/Graphics1/Image.h>
#include <Kore/TextureImpl.h>

typedef Kinc_G1_Image Kinc_G4_Image;

typedef struct {
	int texWidth;
	int texHeight;
	int texDepth;
//private:
	//void init(const char* format, bool readable = false);
	//void init3D(bool readable = false);
	Kinc_G4_TextureImpl impl;
} Kinc_G4_Texture;

Kinc_G4_Texture *Kinc_G4_Texture_Create(int width, int height, Format format, bool readable);
Texture(int width, int height, int depth, Format format, bool readable = false);
Texture(Kore::Reader &reader, const char *format, bool readable = false);
Texture(const char *filename, bool readable = false);
Texture(void *data, int size, const char *format, bool readable = false);
Texture(void *data, int width, int height, int format, bool readable = false);
Texture(void *data, int width, int height, int depth, int format, bool readable = false);
#ifdef KORE_ANDROID
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

int Kinc_G4_Texture_Stride(Kinc_G4_Texture *texture);
