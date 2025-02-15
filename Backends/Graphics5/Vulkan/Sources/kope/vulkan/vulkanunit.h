#ifndef KOPE_VULKAN_UNIT_HEADER
#define KOPE_VULKAN_UNIT_HEADER

#include <kope/graphics5/textureformat.h>

#ifdef KINC_WINDOWS

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
// #define NOMB
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
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

// avoids a warning in the Windows headers
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#endif

#ifndef NDEBUG
#define VALIDATE
#endif

#include <vulkan/vulkan.h>

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR vulkan_GetPhysicalDeviceSurfaceSupportKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vulkan_GetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vulkan_GetPhysicalDeviceSurfaceFormatsKHR = NULL;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vulkan_GetPhysicalDeviceSurfacePresentModesKHR = NULL;
static PFN_vkCreateSwapchainKHR vulkan_CreateSwapchainKHR = NULL;
static PFN_vkDestroySwapchainKHR vulkan_DestroySwapchainKHR = NULL;
static PFN_vkGetSwapchainImagesKHR vulkan_GetSwapchainImagesKHR = NULL;
static PFN_vkDestroySurfaceKHR vulkan_DestroySurfaceKHR = NULL;

static PFN_vkCreateDebugUtilsMessengerEXT vulkan_CreateDebugUtilsMessengerEXT = NULL;
static PFN_vkDestroyDebugUtilsMessengerEXT vulkan_DestroyDebugUtilsMessengerEXT = NULL;

static PFN_vkAcquireNextImageKHR vulkan_AcquireNextImageKHR = NULL;
static PFN_vkQueuePresentKHR vulkan_QueuePresentKHR = NULL;

static PFN_vkDebugMarkerSetObjectNameEXT vulkan_DebugMarkerSetObjectNameEXT = NULL;
static PFN_vkCmdDebugMarkerBeginEXT vulkan_CmdDebugMarkerBeginEXT = NULL;
static PFN_vkCmdDebugMarkerEndEXT vulkan_CmdDebugMarkerEndEXT = NULL;
static PFN_vkCmdDebugMarkerInsertEXT vulkan_CmdDebugMarkerInsertEXT = NULL;

static VkFormat convert_format(kope_g5_texture_format format);

#endif
