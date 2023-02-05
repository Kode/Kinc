// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D12TranslationLayer
{
    constexpr UINT16 DXVA_INVALID_PICTURE_INDEX = 0xFFFF;
    constexpr UINT16 HEVC_INVALID_PICTURE_INDEX = 0x7F;
    constexpr UINT16 H264_INVALID_PICTURE_INDEX = 0x7F;
    constexpr UINT16 VPX_INVALID_PICTURE_INDEX  = 0x7F;

    class VideoDecoder : public DeviceChildImpl<ID3D12VideoDecoder>
    {
    public:
        VideoDecoder(ImmediateContext *pContext, ID3D12VideoDevice* pVideoDeviceNoRef, const D3D12_VIDEO_DECODER_DESC& desc);

        D3D12_VIDEO_DECODER_DESC GetDesc() { return GetForImmediateUse()->GetDesc(); }
    };

    class VideoDecoderHeap : public DeviceChildImpl<ID3D12VideoDecoderHeap>
    {
    public:
        VideoDecoderHeap(ImmediateContext *pContext, ID3D12VideoDevice* pVideoDeviceNoRef, const D3D12_VIDEO_DECODER_HEAP_DESC& desc);

        D3D12_VIDEO_DECODER_HEAP_DESC GetDesc() { return GetForImmediateUse()->GetDesc(); }
    };

    struct ReferenceOnlyDesc
    {
        DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
        UINT64 Width = 0;
        UINT Height = 0;
    };

    struct ReferenceDataManager
    {
        ReferenceDataManager(
            _In_ ImmediateContext *pImmediateContext, VIDEO_DECODE_PROFILE_TYPE profileType);
    
        UINT Size() const { return (UINT)textures.size(); }
        bool IsReferenceOnly() { return m_fReferenceOnly; }
    
        void Resize(UINT16 dbp, _In_opt_ ReferenceOnlyDesc* pReferenceOnly, bool fArrayOfTexture);
        
        void ResetInternalTrackingReferenceUsage();
        void ResetReferenceFramesInformation();

        template<typename T, size_t size> 
        void MarkReferencesInUse(const T (&picEntries)[size]);
        void MarkReferenceInUse(UINT16 index);
    
        void ReleaseUnusedReferences();
        
        UINT16 StoreFutureReference(UINT16 index, _In_ std::shared_ptr<VideoDecoderHeap>& decoderHeap, Resource* pTexture2D, UINT subresourceIndex);
        
        template<typename T, size_t size> 
        void UpdateEntries(T (&picEntries)[size]);
        UINT16 UpdateEntry(UINT16 index);        

        template<typename T, size_t size> 
        void GetUpdatedEntries(T (&picEntries)[size]);
        UINT16 GetUpdatedEntry(UINT16 index);
    
        void TransitionReferenceOnlyOutput(_Out_ ID3D12Resource*& pOutputReferenceNoRef, _Out_ UINT& OutputSubresource);
    
        // D3D12 DecodeFrame Parameters.
        std::vector<ID3D12Resource *>                        textures;
        std::vector<UINT>                                    texturesSubresources;
        std::vector<ID3D12VideoDecoderHeap *>                decoderHeapsParameter;
    
    protected:
    
        struct ReferenceData
        {
            std::shared_ptr<VideoDecoderHeap>   decoderHeap;
            unique_comptr<Resource>             referenceOnlyTexture; // Allocated and lifetime managed by translation layer
            Resource*                           referenceTexture;     // May point to caller allocated resource or referenceOnlyTexture
            UINT                                subresourceIndex;
            UINT16                              originalIndex;
            bool                                fUsed;
        };
    
        void TransitionReference(_In_ ReferenceData& referenceData, D3D12_RESOURCE_STATES decodeState);
        void ResizeDataStructures(UINT size);
        UINT16 FindRemappedIndex(UINT16 originalIndex);
    
        std::vector<ReferenceData>                           referenceDatas;
    
        ImmediateContext*                                    m_pImmediateContext;
        UINT16                                               m_invalidIndex;
        UINT16                                               m_currentOutputIndex = 0;
        bool                                                 m_fReferenceOnly = false;
        bool                                                 m_fArrayOfTexture = false;
    };

    //----------------------------------------------------------------------------------------------------------------------------------
    template<typename T, size_t size>
    inline void ReferenceDataManager::UpdateEntries(T (&picEntries)[size])
    {
        for (auto& picEntry : picEntries)
        {
            picEntry.Index7Bits = UpdateEntry(picEntry.Index7Bits);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template<typename T, size_t size>
    inline void ReferenceDataManager::GetUpdatedEntries(T (&picEntries)[size])
    {
        for (auto& picEntry : picEntries)
        {
            picEntry.Index7Bits = GetUpdatedEntry(picEntry.Index7Bits);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------    
    template<typename T, size_t size> 
    inline void ReferenceDataManager::MarkReferencesInUse(const T (&picEntries)[size])
    {
        for (auto& picEntry : picEntries)
        {
            MarkReferenceInUse(picEntry.Index7Bits);
        }
    }
};