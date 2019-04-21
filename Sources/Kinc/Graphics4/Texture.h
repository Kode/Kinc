#pragma once

#include <Kinc/Graphics1/Image.h>
#include <Kore/TextureImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef Kinc_Image Kinc_G4_Image;

typedef struct _Kinc_G4_Texture {
	int texWidth;
	int texHeight;
	int texDepth;
	// private:
	// void init(const char* format, bool readable = false);
	// void init3D(bool readable = false);
	Kinc_Image image;
	Kinc_G4_TextureImpl impl;
} Kinc_G4_Texture;

void Kinc_G4_Texture_Create(Kinc_G4_Texture *texture, int width, int height, Kinc_ImageFormat format, bool readable);
void Kinc_G4_Texture_Create3D(Kinc_G4_Texture *texture, int width, int height, int depth, Kinc_ImageFormat format, bool readable);
void Kinc_G4_Texture_Destroy(Kinc_G4_Texture *texture);
// Texture(Kore::Reader &reader, const char *format, bool readable = false);
Kinc_G4_Texture *Kinc_G4_Texture_CreateFromFile(const char *filename, bool readable);
Kinc_G4_Texture *Kinc_G4_Texture_CreateFromBytes(void *data, int size, const char *format, bool readable);
// Kinc_G4_Texture *Kinc_G4_Texture_CreateFromBytes(void *data, int width, int height, int format, bool readable);
Kinc_G4_Texture *Kinc_G4_Texture_CreateFromBytes3D(void *data, int width, int height, int depth, int format, bool readable = false);
#ifdef KORE_ANDROID
Texture(unsigned texid);
#endif
// void _set(TextureUnit unit);
// void _setImage(TextureUnit unit);
unsigned char *Kinc_G4_Texture_Lock(Kinc_G4_Texture *texture);
void Kinc_G4_Texture_Unlock(Kinc_G4_Texture *texture);
void Kinc_G4_Texture_Clear(Kinc_G4_Texture *texture, int x, int y, int z, int width, int height, int depth, unsigned color);
#if defined(KORE_IOS) || defined(KORE_MACOS)
void upload(u8 *data, int stride);
#endif
void Kinc_G4_Texture_GenerateMipmaps(Kinc_G4_Texture *texture, int levels);
void Kinc_G4_Texture_setMipmap(Kinc_G4_Texture *texture, Kinc_G4_Texture *mipmap, int level);

int Kinc_G4_Texture_Stride(Kinc_G4_Texture *texture);

#ifdef __cplusplus
}
#endif
