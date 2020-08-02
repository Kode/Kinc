/********************************************************************************/ /**
 \file      OVR_CAPI_Vk.h
 \brief     Vulkan specific structures used by the CAPI interface.
 \copyright Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.
 ************************************************************************************/

#ifndef OVR_CAPI_Vk_h
#define OVR_CAPI_Vk_h

#include "OVR_CAPI.h"
#include "OVR_Version.h"

#if !defined(OVR_EXPORTING_CAPI)

//-----------------------------------------------------------------------------------
// ***** Vulkan Specific

/// Get a list of Vulkan vkInstance extensions required for VR.
///
/// Returns a list of strings delimited by a single space identifying Vulkan extensions that must
/// be enabled in order for the VR runtime to support Vulkan-based applications. The returned
/// list reflects the current runtime version and the GPU the VR system is currently connected to.
///
/// \param[in]  luid Specifies the luid for the relevant GPU, which is returned from ovr_Create.
/// \param[in]  extensionNames is a character buffer which will receive a list of extension name
///               strings, separated by a single space char between each extension.
/// \param[in]  inoutExtensionNamesSize indicates on input the capacity of extensionNames in chars.
///               On output it returns the number of characters written to extensionNames,
///               including the terminating 0 char. In the case of this function returning
///               ovrError_InsufficientArraySize, the required inoutExtensionNamesSize is returned.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information. Returns ovrError_InsufficientArraySize in
///         the case that inoutExtensionNameSize didn't have enough space, in which case
///         inoutExtensionNameSize will return the required inoutExtensionNamesSize.
///
/// <b>Example code</b>
///     \code{.cpp}
///         char extensionNames[4096];
///         uint32_t extensionNamesSize = sizeof(extensionNames);
///         ovr_GetInstanceExtensionsVk(luid, extensionsnames, &extensionNamesSize);
///
///         uint32_t extensionCount = 0;
///         const char* extensionNamePtrs[256];
///         for(const char* p = extensionNames; *p; ++p) {
///             if((p == extensionNames) || (p[-1] == ' ')) {
///                 extensionNamePtrs[extensionCount++] = p;
///                 if (p[-1] == ' ')
///                     p[-1] = '\0';
///             }
///         }
///
///         VkInstanceCreateInfo info = { ... };
///         info.enabledExtensionCount = extensionCount;
///         info.ppEnabledExtensionNames = extensionNamePtrs;
///         [...]
///     \endcode
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetInstanceExtensionsVk(
    ovrGraphicsLuid luid,
    char* extensionNames,
    uint32_t* inoutExtensionNamesSize);

/// Get a list of Vulkan vkDevice extensions required for VR.
///
/// Returns a list of strings delimited by a single space identifying Vulkan extensions that must
/// be enabled in order for the VR runtime to support Vulkan-based applications. The returned
/// list reflects the current runtime version and the GPU the VR system is currently connected to.
///
/// \param[in]  luid Specifies the luid for the relevant GPU, which is returned from ovr_Create.
/// \param[in]  extensionNames is a character buffer which will receive a list of extension name
///               strings, separated by a single space char between each extension.
/// \param[in]  inoutExtensionNamesSize indicates on input the capacity of extensionNames in chars.
///               On output it returns the number of characters written to extensionNames,
///               including the terminating 0 char. In the case of this function returning
///               ovrError_InsufficientArraySize, the required inoutExtensionNamesSize is returned.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information. Returns ovrError_InsufficientArraySize in
///         the case that inoutExtensionNameSize didn't have enough space, in which case
///         inoutExtensionNameSize will return the required inoutExtensionNamesSize.
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetDeviceExtensionsVk(
    ovrGraphicsLuid luid,
    char* extensionNames,
    uint32_t* inoutExtensionNamesSize);

/// Find VkPhysicalDevice matching ovrGraphicsLuid
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  luid Specifies the luid returned from ovr_Create.
/// \param[in]  instance Specifies a VkInstance to search for matching luids in.
/// \param[out] out_physicalDevice Returns the VkPhysicalDevice matching the instance and luid.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// \note This function enumerates the current physical devices and returns the one matching the
/// luid. It must be called at least once prior to any ovr_CreateTextureSwapChainVk or
/// ovr_CreateMirrorTextureWithOptionsVk calls, and the instance must remain valid for the lifetime
/// of the returned objects. It is assumed the VkDevice created by the application will be for the
/// returned physical device.
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetSessionPhysicalDeviceVk(
    ovrSession session,
    ovrGraphicsLuid luid,
    VkInstance instance,
    VkPhysicalDevice* out_physicalDevice);

/// Select VkQueue to block on till rendering is complete
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  queue Specifies a VkQueue to add a VkFence operation to and wait on.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// \note The queue may be changed at any time but only the value at the time ovr_SubmitFrame
/// is called will be used. ovr_SetSynchronizationQueueVk must be called with a valid VkQueue
/// created on the same VkDevice the texture sets were created on prior to the first call to
/// ovr_SubmitFrame. An internally created VkFence object will be signalled by the completion
/// of operations on queue and waited on to synchronize the VR compositor.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_SetSynchronizationQueueVk(ovrSession session, VkQueue queue);
// Backwards compatibility for the original typoed version
#define ovr_SetSynchonizationQueueVk ovr_SetSynchronizationQueueVk
// Define OVR_PREVIEW_DEPRECATION to generate warnings for upcoming API deprecations
#if defined(OVR_PREVIEW_DEPRECATION)
#pragma deprecated("ovr_SetSynchonizationQueueVk")
#endif

/// Create Texture Swap Chain suitable for use with Vulkan
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  device Specifies the application's VkDevice to create resources with.
/// \param[in]  desc Specifies requested texture properties. See notes for more info
///             about texture format.
/// \param[out] out_TextureSwapChain Returns the created ovrTextureSwapChain, which will be valid
///             upon a successful return value, else it will be NULL.
///             This texture chain must be eventually destroyed via ovr_DestroyTextureSwapChain
///             before destroying the session with ovr_Destroy.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// \note The texture format provided in \a desc should be thought of as the format the
///       distortion-compositor will use for the ShaderResourceView when reading the contents
///       of the texture. To that end, it is highly recommended that the application
///       requests texture swapchain formats that are in sRGB-space
///       (e.g. OVR_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor does sRGB-correct rendering.
///       As such, the compositor relies on the GPU's hardware sampler to do the sRGB-to-linear
///       conversion. If the application still prefers to render to a linear format (e.g.
///       OVR_FORMAT_R8G8B8A8_UNORM) while handling the linear-to-gamma conversion via
///       SPIRV code, then the application must still request the corresponding sRGB format and
///       also use the \a ovrTextureMisc_DX_Typeless flag in the ovrTextureSwapChainDesc's
///       Flag field. This will allow the application to create a RenderTargetView that is the
///       desired linear format while the compositor continues to treat it as sRGB. Failure to
///       do so will cause the compositor to apply unexpected gamma conversions leading to
///       gamma-curve artifacts. The \a ovrTextureMisc_DX_Typeless flag for depth buffer formats
///       (e.g. OVR_FORMAT_D32_FLOAT) is ignored as they are always
///       converted to be typeless.
///
/// \see ovr_GetTextureSwapChainLength
/// \see ovr_GetTextureSwapChainCurrentIndex
/// \see ovr_GetTextureSwapChainDesc
/// \see ovr_GetTextureSwapChainBufferVk
/// \see ovr_DestroyTextureSwapChain
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_CreateTextureSwapChainVk(
    ovrSession session,
    VkDevice device,
    const ovrTextureSwapChainDesc* desc,
    ovrTextureSwapChain* out_TextureSwapChain);

/// Get a specific VkImage within the chain
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies an ovrTextureSwapChain previously returned by
///             ovr_CreateTextureSwapChainVk
/// \param[in]  index Specifies the index within the chain to retrieve.
///             Must be between 0 and length (see ovr_GetTextureSwapChainLength),
///             or may pass -1 to get the buffer at the CurrentIndex location (saving a
///             call to GetTextureSwapChainCurrentIndex).
/// \param[out] out_Image Returns the VkImage retrieved.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetTextureSwapChainBufferVk(
    ovrSession session,
    ovrTextureSwapChain chain,
    int index,
    VkImage* out_Image);

/// Create Mirror Texture which is auto-refreshed to mirror Rift contents produced by this
/// application.
///
/// A second call to ovr_CreateMirrorTextureWithOptionsVk for a given ovrSession before destroying
/// the first one is not supported and will result in an error return.
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  device Specifies the VkDevice to create resources with.
/// \param[in]  desc Specifies requested texture properties. See notes for more info
///             about texture format.
/// \param[out] out_MirrorTexture Returns the created ovrMirrorTexture, which will be
///             valid upon a successful return value, else it will be NULL.
///             This texture must be eventually destroyed via ovr_DestroyMirrorTexture before
///             destroying the session with ovr_Destroy.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// \note The texture format provided in \a desc should be thought of as the format the
///       compositor will use for the VkImageView when writing into mirror texture. To that end,
///       it is highly recommended that the application requests a mirror texture format that is
///       in sRGB-space (e.g. OVR_FORMAT_R8G8B8A8_UNORM_SRGB) as the compositor does sRGB-correct
///       rendering. If however the application wants to still read the mirror texture as a
///       linear format (e.g. OVR_FORMAT_R8G8B8A8_UNORM) and handle the sRGB-to-linear conversion
///       in SPIRV code, then it is recommended the application still requests an sRGB format and
///       also use the \a ovrTextureMisc_DX_Typeless flag in the ovrMirrorTextureDesc's
///       Flags field. This will allow the application to bind a ShaderResourceView that is a
///       linear format while the compositor continues to treat is as sRGB. Failure to do so will
///       cause the compositor to apply unexpected gamma conversions leading to
///       gamma-curve artifacts.
///
/// <b>Example code</b>
///     \code{.cpp}
///         ovrMirrorTexture     mirrorTexture = nullptr;
///         ovrMirrorTextureDesc mirrorDesc = {};
///         mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
///         mirrorDesc.Width  = mirrorWindowWidth;
///         mirrorDesc.Height = mirrorWindowHeight;
///         ovrResult result = ovr_CreateMirrorTextureWithOptionsVk(session, vkDevice, &mirrorDesc,
///         &mirrorTexture);
///         [...]
///         // Destroy the texture when done with it.
///         ovr_DestroyMirrorTexture(session, mirrorTexture);
///         mirrorTexture = nullptr;
///     \endcode
///
/// \see ovr_GetMirrorTextureBufferVk
/// \see ovr_DestroyMirrorTexture
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_CreateMirrorTextureWithOptionsVk(
    ovrSession session,
    VkDevice device,
    const ovrMirrorTextureDesc* desc,
    ovrMirrorTexture* out_MirrorTexture);

/// Get a the underlying mirror VkImage
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  mirrorTexture Specifies an ovrMirrorTexture previously returned by
/// ovr_CreateMirrorTextureWithOptionsVk
/// \param[out] out_Image Returns the VkImage pointer retrieved.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use
///         ovr_GetLastErrorInfo to get more information.
///
/// <b>Example code</b>
///     \code{.cpp}
///         VkImage mirrorImage = VK_NULL_HANDLE;
///         ovr_GetMirrorTextureBufferVk(session, mirrorTexture, &mirrorImage);
///         ...
///         vkCmdBlitImage(commandBuffer, mirrorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
///         presentImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
///         ...
///         vkQueuePresentKHR(queue, &presentInfo);
///     \endcode
///
OVR_PUBLIC_FUNCTION(ovrResult)
ovr_GetMirrorTextureBufferVk(
    ovrSession session,
    ovrMirrorTexture mirrorTexture,
    VkImage* out_Image);

#endif // !defined(OVR_EXPORTING_CAPI)

#endif // OVR_CAPI_Vk_h
