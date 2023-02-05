// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

namespace D3D12TranslationLayer
{
    //----------------------------------------------------------------------------------------------------------------------------------
    void VideoDevice::Initialize()
    {
        if (!m_pParent->m_pDevice12_1)
        {
            ThrowFailure(E_NOINTERFACE);
        }
        m_pParent->InitializeVideo(&m_spVideoDevice);

        D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT decodeProfileData = {};
        decodeProfileData.NodeIndex = m_pParent->GetNodeIndex();
        CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, &decodeProfileData, sizeof(decodeProfileData));
        m_decodeProfiles.resize(decodeProfileData.ProfileCount);     //throw( bad_alloc )

        // get profiles
        std::unique_ptr<GUID[]> spGUIDs;
        spGUIDs.reset(new GUID[decodeProfileData.ProfileCount]);    //throw( bad_alloc )
        D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES profiles = {};
        profiles.NodeIndex = m_pParent->GetNodeIndex();
        profiles.ProfileCount = decodeProfileData.ProfileCount;
        profiles.pProfiles = spGUIDs.get();
        CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILES, &profiles, sizeof(profiles));

        // fill formats for each profile
        for (UINT i = 0; i < decodeProfileData.ProfileCount; i++)
        {
            m_decodeProfiles[i].profileGUID = spGUIDs[i];

            D3D12_VIDEO_DECODE_CONFIGURATION decodeConfig = {
                m_decodeProfiles[i].profileGUID,
                D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE,
                D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE
            };

            // format count
            D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT decodeProfileFormatData = {};
            decodeProfileFormatData.NodeIndex = m_pParent->GetNodeIndex();
            decodeProfileFormatData.Configuration = decodeConfig;
            CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT, &decodeProfileFormatData, sizeof(decodeProfileFormatData));

            // decoder formats
            D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS formats = {};
            formats.NodeIndex = m_pParent->GetNodeIndex();
            formats.Configuration = decodeConfig;
            formats.FormatCount = decodeProfileFormatData.FormatCount;
            m_decodeProfiles[i].formats.resize(formats.FormatCount);    //throw( bad_alloc ))
            formats.pOutputFormats = m_decodeProfiles[i].formats.data();
            CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMATS, &formats, sizeof(formats));
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDevice::~VideoDevice()  noexcept
    {
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderProfileCount(UINT *ProfileCount)
    {
        if (!ProfileCount)
        {
            ThrowFailure(E_POINTER);
        }
        *ProfileCount = (UINT)m_decodeProfiles.size();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderProfile(UINT Index, GUID *pDecodeProfile)
    {
        if (Index >= m_decodeProfiles.size())
        {
            ThrowFailure(E_INVALIDARG);
        }
        else
        {
            *pDecodeProfile = m_decodeProfiles[Index].profileGUID;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderFormatCount(const GUID *pDecodeProfile, UINT *pFormatCount)
    {
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == *pDecodeProfile)
            {
                *pFormatCount = (UINT)m_decodeProfiles[i].formats.size();
                return;
            }
        }
        ThrowFailure(E_INVALIDARG);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderFormat(const GUID *pDecodeProfile, UINT Index, DXGI_FORMAT *pFormat)
    {
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == *pDecodeProfile)
            {
                *pFormat = m_decodeProfiles[i].formats[Index];
                return;
            }
        }
        ThrowFailure(E_INVALIDARG);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::CheckVideoDecoderFormat(const GUID *pDecodeProfile, DXGI_FORMAT format, BOOL *pSupported)
    {
        std::unique_ptr<DXGI_FORMAT[]> spFormats;
        if (!pSupported)
        {
            ThrowFailure(E_POINTER);
        }
        *pSupported = FALSE;
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == *pDecodeProfile)
            {
                for (DWORD j = 0; j < m_decodeProfiles[i].formats.size(); j++)
                {
                    if (format == m_decodeProfiles[i].formats[j] &&
                        CD3D11FormatHelper::GetTypeLevel(format) == D3D11FTL_FULL_TYPE)
                    {
                        *pSupported = TRUE;
                        break;
                    }
                }
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    bool VideoDevice::IsProfileSupported(REFGUID DecodeProfile) noexcept
    {
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == DecodeProfile)
            {
                return true;
            }
        }
        return false;
    }


    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderBufferTypeCount(const VIDEO_DECODE_DESC *pDesc, UINT *pBufferTypeCount)
    {
        if (!IsProfileSupported(pDesc->DecodeProfile))
        {
            ThrowFailure(E_INVALIDARG);
        }
        ThrowFailure(VideoDecode::GetVideoDecoderBufferTypeCount(pDesc, pBufferTypeCount));
    }


    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderBufferInfo(const VIDEO_DECODE_DESC *pDesc, UINT Index, VIDEO_DECODE_BUFFER_TYPE *pType, UINT *pSize)
    {
        if (!IsProfileSupported(pDesc->DecodeProfile))
        {
            ThrowFailure(E_INVALIDARG);
        }
        VideoDecode::GetVideoDecoderBufferInfo(pDesc, Index, pType, pSize, m_pParent->IsXbox());
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderConfigCount(const VIDEO_DECODE_DESC *pDesc, UINT *pConfigCount)
    {
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == pDesc->DecodeProfile)
            {
                VideoDecode::GetVideoDecoderConfigCount(m_pParent->m_pDevice12.get(), m_pParent->GetNodeIndex(), pDesc, pConfigCount);
                return;
            }
        }
        *pConfigCount = 0;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::GetVideoDecoderConfig(const VIDEO_DECODE_DESC *pDesc, UINT Index, VIDEO_DECODE_CONFIG *pConfig)
    {
        for (DWORD i = 0; i < m_decodeProfiles.size(); i++)
        {
            if (m_decodeProfiles[i].profileGUID == pDesc->DecodeProfile)
            {
                VideoDecode::GetVideoDecoderConfig(m_pParent->m_pDevice12.get(), m_pParent->GetNodeIndex(), pDesc, Index, pConfig, m_pParent->IsXbox());
                return;
            }
        }
        ThrowFailure(E_INVALIDARG);
    }

// Some of the 'enum values' below are actually #defines, which generates a warning when used in a case statement.
#pragma warning(disable: 4063)
    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDevice::CheckFeatureSupport(D3D12_FEATURE_VIDEO FeatureVideo, void* pFeatureSupportData, UINT FeatureSupportDataSize)
    {
        switch (FeatureVideo)
        {
        case D3D12_FEATURE_VIDEO_DECODE_SUPPORT:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_CONVERSION_SUPPORT:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_CONVERSION_SUPPORT>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_PROFILES:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_FORMATS:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        case D3D12_FEATURE_VIDEO_DECODE_HISTOGRAM:
        {
            SetFeatureDataNodeIndex<D3D12_FEATURE_DATA_VIDEO_DECODE_HISTOGRAM>(pFeatureSupportData, FeatureSupportDataSize, m_pParent->GetNodeIndex());
            break;
        }

        default:
            ThrowFailure(E_NOTIMPL);
            break;
        }
        ThrowFailure(m_spVideoDevice->CheckFeatureSupport(FeatureVideo, pFeatureSupportData, FeatureSupportDataSize));
    }
};
