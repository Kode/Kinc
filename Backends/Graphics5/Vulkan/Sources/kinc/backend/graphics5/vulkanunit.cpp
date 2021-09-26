#ifdef KORE_WINDOWS

// Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
//#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
//#define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#endif

#include <vulkan/vulkan.h>

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/image.h>
#include <kinc/math/matrix.h>

namespace Kore {
	namespace Vulkan {
		struct SwapchainBuffers {
			VkImage image;
			VkCommandBuffer cmd;
			VkImageView view;
		};

		extern SwapchainBuffers *buffers;

		struct DepthBuffer {
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		};

		extern DepthBuffer depth;

		// buffer hack
		extern VkBuffer *vertexUniformBuffer;
		extern VkBuffer *fragmentUniformBuffer;
	}
}

#include "Vulkan.cpp.h"
#include "commandlist.cpp.h"
#include "constantbuffer.cpp.h"
#include "indexbuffer.cpp.h"
#include "pipeline.cpp.h"
#include "raytrace.cpp.h"
#include "rendertarget.cpp.h"
#include "shader.cpp.h"
#include "texture.cpp.h"
#include "vertexbuffer.cpp.h"
