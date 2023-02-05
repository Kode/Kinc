// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include "VideoDecode.hpp"

#include <vector>

namespace D3D12TranslationLayer {
	//==================================================================================================================================
	// VideoDevice
	// Stores data responsible for remapping D3D11 video functionality to underlying D3D12 video functionality
	//==================================================================================================================================
	class VideoDevice {
	public:
		friend class ImmediateContext;
		VideoDevice(_In_ ImmediateContext *pDevice) {
			Initialize();
		}
		virtual ~VideoDevice() noexcept;

	public:
		void GetVideoDecoderProfileCount(_Out_ UINT *pProfileCount);
		void GetVideoDecoderProfile(_In_ UINT Index, _Out_ GUID *pProfile);
		void GetVideoDecoderFormatCount(_In_ const GUID *pDecodeProfile, _Out_ UINT *pFormatCount);
		void GetVideoDecoderFormat(_In_ const GUID *pDecodeProfile, UINT Index, _Out_ DXGI_FORMAT *pFormat);
		void CheckVideoDecoderFormat(_In_ const GUID *pDecodeProfile, _In_ DXGI_FORMAT format, _Out_ BOOL *pSupported);
		void GetVideoDecoderConfigCount(_In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pCount);
		void GetVideoDecoderConfig(_In_ const VIDEO_DECODE_DESC *pDesc, _In_ UINT Index, _Out_ VIDEO_DECODE_CONFIG *pConfig);
		void GetVideoDecoderBufferTypeCount(_In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pCount);
		void GetVideoDecoderBufferInfo(_In_ const VIDEO_DECODE_DESC *pDesc, _In_ UINT Index, _Out_ VIDEO_DECODE_BUFFER_TYPE *pType, _Out_ UINT *pSize);
		void CheckFeatureSupport(D3D12_FEATURE_VIDEO FeatureVideo, _Inout_updates_bytes_(FeatureSupportDataSize) void *pFeatureSupportData,
		                         UINT FeatureSupportDataSize);

	protected:
		ID3D12VideoDevice *GetUnderlyingVideoDevice() noexcept {
			return m_spVideoDevice.get();
		}
		bool IsProfileSupported(REFGUID DecodeProfile) noexcept;

		struct ProfileInfo {
			GUID profileGUID;
			std::vector<DXGI_FORMAT> formats;
		};

		unique_comptr<ID3D12VideoDevice> m_spVideoDevice;
		std::vector<ProfileInfo> m_decodeProfiles;

	private:
		void Initialize();
	};
};
