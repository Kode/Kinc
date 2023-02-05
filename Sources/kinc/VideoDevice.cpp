// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "VideoDevice.hpp"

namespace D3D12TranslationLayer {
	//----------------------------------------------------------------------------------------------------------------------------------
	void VideoDevice::Initialize() {
		D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT decodeProfileData = {};
		decodeProfileData.NodeIndex = 0;
		CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, &decodeProfileData, sizeof(decodeProfileData));
		m_decodeProfiles.resize(decodeProfileData.ProfileCount); // throw( bad_alloc )

		// get profiles
		std::unique_ptr<GUID[]> spGUIDs;
		spGUIDs.reset(new GUID[decodeProfileData.ProfileCount]); // throw( bad_alloc )
		D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES profiles = {};
		profiles.NodeIndex = 0;
		profiles.ProfileCount = decodeProfileData.ProfileCount;
		profiles.pProfiles = spGUIDs.get();
		CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILES, &profiles, sizeof(profiles));

		// fill formats for each profile
		for (UINT i = 0; i < decodeProfileData.ProfileCount; i++) {
			m_decodeProfiles[i].profileGUID = spGUIDs[i];

			D3D12_VIDEO_DECODE_CONFIGURATION decodeConfig = {m_decodeProfiles[i].profileGUID, D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE,
			                                                 D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE};

			// format count
			D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT decodeProfileFormatData = {};
			decodeProfileFormatData.NodeIndex = 0;
			decodeProfileFormatData.Configuration = decodeConfig;
			CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT, &decodeProfileFormatData, sizeof(decodeProfileFormatData));

			// decoder formats
			D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS formats = {};
			formats.NodeIndex = 0;
			formats.Configuration = decodeConfig;
			formats.FormatCount = decodeProfileFormatData.FormatCount;
			m_decodeProfiles[i].formats.resize(formats.FormatCount); // throw( bad_alloc ))
			formats.pOutputFormats = m_decodeProfiles[i].formats.data();
			CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMATS, &formats, sizeof(formats));
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	VideoDevice::~VideoDevice() noexcept {}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderProfileCount(UINT *ProfileCount) {
		if (!ProfileCount) {
			assert(false);
		}
		*ProfileCount = (UINT)m_decodeProfiles.size();
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderProfile(UINT Index, GUID *pDecodeProfile) {
		if (Index >= m_decodeProfiles.size()) {
			assert(false);
		}
		else {
			*pDecodeProfile = m_decodeProfiles[Index].profileGUID;
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderFormatCount(const GUID *pDecodeProfile, UINT *pFormatCount) {
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == *pDecodeProfile) {
				*pFormatCount = (UINT)m_decodeProfiles[i].formats.size();
				return;
			}
		}
		assert(false);
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderFormat(const GUID *pDecodeProfile, UINT Index, DXGI_FORMAT *pFormat) {
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == *pDecodeProfile) {
				*pFormat = m_decodeProfiles[i].formats[Index];
				return;
			}
		}
		assert(false);
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::CheckVideoDecoderFormat(const GUID *pDecodeProfile, DXGI_FORMAT format, BOOL *pSupported) {
		std::unique_ptr<DXGI_FORMAT[]> spFormats;
		if (!pSupported) {
			assert(false);
		}
		*pSupported = FALSE;
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == *pDecodeProfile) {
				for (DWORD j = 0; j < m_decodeProfiles[i].formats.size(); j++) {
					if (format == m_decodeProfiles[i].formats[j] /*&& CD3D11FormatHelper::GetTypeLevel(format) == D3D11FTL_FULL_TYPE*/) {
						*pSupported = TRUE;
						break;
					}
				}
				break;
			}
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	bool VideoDevice::IsProfileSupported(REFGUID DecodeProfile) noexcept {
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == DecodeProfile) {
				return true;
			}
		}
		return false;
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderBufferTypeCount(const VIDEO_DECODE_DESC *pDesc, UINT *pBufferTypeCount) {
		if (!IsProfileSupported(pDesc->DecodeProfile)) {
			assert(false);
		}
		// assert(false);ThrowFailure(VideoDecode::GetVideoDecoderBufferTypeCount(pDesc, pBufferTypeCount));
		assert(false);
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderBufferInfo(const VIDEO_DECODE_DESC *pDesc, UINT Index, VIDEO_DECODE_BUFFER_TYPE *pType,
	                                                                   UINT *pSize) {
		if (!IsProfileSupported(pDesc->DecodeProfile)) {
			assert(false);
		}
		VideoDecode::GetVideoDecoderBufferInfo(pDesc, Index, pType, pSize, false);
	}

	extern "C" {
	ID3D12Device *device = NULL;
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderConfigCount(const VIDEO_DECODE_DESC *pDesc, UINT *pConfigCount) {
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == pDesc->DecodeProfile) {
				VideoDecode::GetVideoDecoderConfigCount(device, 0, pDesc, pConfigCount);
				return;
			}
		}
		*pConfigCount = 0;
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::GetVideoDecoderConfig(const VIDEO_DECODE_DESC *pDesc, UINT Index, VIDEO_DECODE_CONFIG *pConfig) {
		for (DWORD i = 0; i < m_decodeProfiles.size(); i++) {
			if (m_decodeProfiles[i].profileGUID == pDesc->DecodeProfile) {
				VideoDecode::GetVideoDecoderConfig(device, 0, pDesc, Index, pConfig, false);
				return;
			}
		}
		assert(false);
	}

// Some of the 'enum values' below are actually #defines, which generates a warning when used in a case statement.
#pragma warning(disable : 4063)
	//----------------------------------------------------------------------------------------------------------------------------------
	_Use_decl_annotations_ void VideoDevice::CheckFeatureSupport(D3D12_FEATURE_VIDEO FeatureVideo, void *pFeatureSupportData, UINT FeatureSupportDataSize) {
		switch (FeatureVideo) {
			/*case D3D12_FEATURE_VIDEO_DECODE_SUPPORT: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_PROFILES: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_FORMATS: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}

			case D3D12_FEATURE_VIDEO_DECODE_HISTOGRAM: {
			    SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_HISTOGRAM>(pFeatureSupportData, FeatureSupportDataSize, 0);
			    break;
			}*/

		default:
			assert(false);
			break;
		}
		// ThrowFailure(m_spVideoDevice->CheckFeatureSupport(FeatureVideo, pFeatureSupportData, FeatureSupportDataSize));
		assert(false);
	}
};
