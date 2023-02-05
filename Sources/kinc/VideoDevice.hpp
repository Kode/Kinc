// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D12TranslationLayer
{
    //==================================================================================================================================
    // VideoDevice
    // Stores data responsible for remapping D3D11 video functionality to underlying D3D12 video functionality
    //==================================================================================================================================
    class VideoDevice : public DeviceChild
    {
    public:
        friend class ImmediateContext;
        VideoDevice(_In_ ImmediateContext *pDevice)
            : DeviceChild(pDevice)
        {
            Initialize();
        }
        virtual ~VideoDevice() noexcept;

    public:
        void TRANSLATION_API GetVideoDecoderProfileCount(_Out_ UINT *pProfileCount);
        void TRANSLATION_API GetVideoDecoderProfile(_In_ UINT Index, _Out_ GUID *pProfile) ;
        void TRANSLATION_API GetVideoDecoderFormatCount(_In_ const GUID *pDecodeProfile, _Out_ UINT *pFormatCount);
        void TRANSLATION_API GetVideoDecoderFormat(_In_ const GUID *pDecodeProfile, UINT Index, _Out_ DXGI_FORMAT *pFormat);
        void TRANSLATION_API CheckVideoDecoderFormat(_In_ const GUID *pDecodeProfile, _In_ DXGI_FORMAT format, _Out_ BOOL *pSupported);
        void TRANSLATION_API GetVideoDecoderConfigCount(_In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pCount);
        void TRANSLATION_API GetVideoDecoderConfig(_In_ const VIDEO_DECODE_DESC *pDesc, _In_ UINT Index, _Out_ VIDEO_DECODE_CONFIG *pConfig);
        void TRANSLATION_API GetVideoDecoderBufferTypeCount(_In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pCount);
        void TRANSLATION_API GetVideoDecoderBufferInfo(_In_ const VIDEO_DECODE_DESC *pDesc, _In_ UINT Index, _Out_ VIDEO_DECODE_BUFFER_TYPE *pType, _Out_ UINT *pSize);
        void TRANSLATION_API CheckFeatureSupport(D3D12_FEATURE_VIDEO FeatureVideo, _Inout_updates_bytes_(FeatureSupportDataSize)void* pFeatureSupportData, UINT FeatureSupportDataSize);
        
    protected:
        ID3D12VideoDevice* GetUnderlyingVideoDevice() noexcept { return m_spVideoDevice.get(); }
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
