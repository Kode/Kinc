// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"

namespace D3D12TranslationLayer
{
    //----------------------------------------------------------------------------------------------------------------------------------
    static UINT16 GetInvalidReferenceIndex(VIDEO_DECODE_PROFILE_TYPE DecodeProfileType)
    {
        assert(DecodeProfileType <= VIDEO_DECODE_PROFILE_TYPE_MAX_VALID);
        static_assert(VIDEO_DECODE_PROFILE_TYPE_H264_MVC + 1 == VIDEO_DECODE_PROFILE_TYPE_MAX_VALID);

        switch (DecodeProfileType)
        {
            case VIDEO_DECODE_PROFILE_TYPE_VC1:
            case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
            case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
                return DXVA_INVALID_PICTURE_INDEX;
            
            case VIDEO_DECODE_PROFILE_TYPE_H264:
            case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
                return H264_INVALID_PICTURE_INDEX;
            
            case VIDEO_DECODE_PROFILE_TYPE_HEVC:
                return HEVC_INVALID_PICTURE_INDEX;
            
            case VIDEO_DECODE_PROFILE_TYPE_VP8:
            case VIDEO_DECODE_PROFILE_TYPE_VP9:
                return VPX_INVALID_PICTURE_INDEX;

            default:
                return 0;
        };
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    ReferenceDataManager::ReferenceDataManager(
        ImmediateContext *pImmediateContext, VIDEO_DECODE_PROFILE_TYPE profileType)
            : m_pImmediateContext(pImmediateContext)
            , m_invalidIndex(GetInvalidReferenceIndex(profileType))
        {}

    //----------------------------------------------------------------------------------------------------------------------------------
    UINT16 ReferenceDataManager::FindRemappedIndex(UINT16 originalIndex)
    {
        // Check if the index is already mapped.
        for (UINT16 remappedIndex = 0; remappedIndex < referenceDatas.size(); remappedIndex++)
        {
            if (referenceDatas[remappedIndex].originalIndex == originalIndex)
            {
                return remappedIndex;
            }
        }

        return m_invalidIndex;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    UINT16 ReferenceDataManager::UpdateEntry(UINT16 index)
    {
        UINT16 remappedIndex = m_invalidIndex;

        if (index != m_invalidIndex)
        {
            remappedIndex = FindRemappedIndex(index);

            bool fTransitionSubresource = true;
            if (   remappedIndex == m_invalidIndex
                || remappedIndex == m_currentOutputIndex)
            {
                // Caller specified an invalid reference index.  Remap it to the current
                // picture index to avoid crashing and still attempt to decode.
                if (g_hTracelogging)
                {
                    TraceLoggingWrite(g_hTracelogging,
                        "Decode - Invalid Reference Index",
                        TraceLoggingValue(index, "Index"),
                        TraceLoggingValue(m_currentOutputIndex, "OutputIndex"));
                }

                remappedIndex = m_currentOutputIndex;

                // The output resource has already been transitioned to the DECODE_WRITE state when
                // set as the current output.  For use as a reference, the resource should be in a DECODE_READ state, 
                // but we can't express both so leave it in the WRITE state.  This is an error condition, so this is 
                // an attempt to keep the decoder producing output until we start getting correct reference indices again.
                fTransitionSubresource = false;
            }

            ReferenceData& referenceData = referenceDatas[remappedIndex];

            decoderHeapsParameter[remappedIndex] = referenceData.decoderHeap->GetForUse(COMMAND_LIST_TYPE::VIDEO_DECODE);

            if (fTransitionSubresource)
            {
                TransitionReference(referenceData, D3D12_RESOURCE_STATE_VIDEO_DECODE_READ);
            }

            textures[remappedIndex] = referenceData.referenceTexture->GetUnderlyingResource();
            texturesSubresources[remappedIndex] = referenceData.subresourceIndex;
        }

        return remappedIndex;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    UINT16 ReferenceDataManager::GetUpdatedEntry(UINT16 index)
    {
        UINT16 remappedIndex = m_invalidIndex;

        if (index != m_invalidIndex)
        {
            remappedIndex = FindRemappedIndex(index);

            if (remappedIndex == m_invalidIndex)
            {
                // Caller specified an invalid reference index.  Remap it to the current
                // picture index to avoid crashing and still attempt to decode.
                remappedIndex = m_currentOutputIndex;
            }
        }

        return remappedIndex;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    UINT16 ReferenceDataManager::StoreFutureReference(UINT16 index, std::shared_ptr<VideoDecoderHeap>& decoderHeap, Resource* pTexture2D, UINT subresourceIndex)
    {
        // Check if the index was in use.
        UINT16 remappedIndex = FindRemappedIndex(index);

        if (remappedIndex == m_invalidIndex)
        {
            // If not already mapped, see if the same index in the remapped space is available.
            if (   index < referenceDatas.size()
                && referenceDatas[index].originalIndex == m_invalidIndex)
            {
                remappedIndex = index;
            }
        }

        if (remappedIndex == m_invalidIndex)
        {
            // The current output index was not used last frame.  Get an unused entry.
            remappedIndex = FindRemappedIndex(m_invalidIndex);
        }

        if (remappedIndex == m_invalidIndex)
        {
            // No unused entry exists.  Indicates a problem with MaxDPB.
            if (g_hTracelogging)
            {
                TraceLoggingWrite(g_hTracelogging,
                    "Decode - No available reference map entry for output.");
            }
            ThrowFailure(E_INVALIDARG);
        }

        ReferenceData& referenceData = referenceDatas[remappedIndex];

        // Set the index as the key in this map entry.
        referenceData.originalIndex = index;

        referenceData.decoderHeap = decoderHeap;

        // When IsReferenceOnly is true, then the translation layer is managing references
        // either becasue the layout is incompatible with other texture usage (REFERENCE_ONLY), or because and/or 
        // decode output conversion is enabled.
        if (!IsReferenceOnly())
        {
            referenceData.referenceTexture = pTexture2D;
            referenceData.subresourceIndex = subresourceIndex;
        }

        // Store the index to use for error handling when caller specifies and invalid reference index.
        m_currentOutputIndex = remappedIndex;

        return remappedIndex;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void ReferenceDataManager::TransitionReferenceOnlyOutput(ID3D12Resource*& pOutputReferenceNoRef, UINT& OutputSubresource)
    {
        assert(IsReferenceOnly());

        ReferenceData& referenceData = referenceDatas[m_currentOutputIndex];

        TransitionReference(referenceData, D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE);

        pOutputReferenceNoRef = referenceData.referenceTexture->GetUnderlyingResource();
        OutputSubresource = referenceData.subresourceIndex;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void ReferenceDataManager::MarkReferenceInUse(UINT16 index)
    {
        if (index != m_invalidIndex)
        {
            UINT16 remappedIndex = FindRemappedIndex(index);
            if (remappedIndex != m_invalidIndex)
            {
                referenceDatas[remappedIndex].fUsed = true;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void ReferenceDataManager::ReleaseUnusedReferences()
    {
        for (ReferenceData& referenceData : referenceDatas)
        {
            if (!referenceData.fUsed)
            {
                referenceData.decoderHeap = nullptr;

                if (!IsReferenceOnly())
                {
                    referenceData.referenceTexture = nullptr;
                    referenceData.subresourceIndex = 0;
                }

                referenceData.originalIndex = m_invalidIndex;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void ReferenceDataManager::Resize(UINT16 dpb, ReferenceOnlyDesc* pReferenceOnly, bool fArrayOfTexture)
    {
        m_fArrayOfTexture = fArrayOfTexture;

        ResizeDataStructures(dpb);
        ResetInternalTrackingReferenceUsage();
        ResetReferenceFramesInformation();
        ReleaseUnusedReferences();

        m_fReferenceOnly = pReferenceOnly != nullptr;

        if (m_fReferenceOnly)
        {
            ResourceCreationArgs ResourceArgs = {};

            if (fArrayOfTexture)
            {
                ResourceArgs.m_desc12 = CD3DX12_RESOURCE_DESC::Tex2D(pReferenceOnly->Format, pReferenceOnly->Width, pReferenceOnly->Height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
                ResourceArgs.m_appDesc = AppResourceDesc(ResourceArgs.m_desc12, RESOURCE_USAGE_DEFAULT, RESOURCE_CPU_ACCESS_NONE, RESOURCE_BIND_DECODER);

                UINT64 resourceSize = 0;
                m_pImmediateContext->m_pDevice12->GetCopyableFootprints(&ResourceArgs.m_desc12, 0, 1, 0, nullptr, nullptr, nullptr, &resourceSize);
                ResourceArgs.m_heapDesc = CD3DX12_HEAP_DESC(resourceSize, m_pImmediateContext->GetHeapProperties(D3D12_HEAP_TYPE_DEFAULT));

                for (ReferenceData& referenceData : referenceDatas)
                {
                    if (   !referenceData.referenceOnlyTexture
                        || 0 != memcmp(referenceData.referenceOnlyTexture->Parent(), &ResourceArgs, sizeof(ResourceCreationArgs)))
                    {
                        referenceData.referenceOnlyTexture = Resource::CreateResource(m_pImmediateContext, ResourceArgs, ResourceAllocationContext::ImmediateContextThreadLongLived);
                        assert(0 == memcmp(referenceData.referenceOnlyTexture->Parent(), &ResourceArgs, sizeof(ResourceCreationArgs)));
                    }

                    referenceData.referenceTexture = referenceData.referenceOnlyTexture.get();
                    referenceData.subresourceIndex = 0u;
                }
            }
            else
            {
                ResourceArgs.m_desc12 = CD3DX12_RESOURCE_DESC::Tex2D(pReferenceOnly->Format, pReferenceOnly->Width, pReferenceOnly->Height, dpb, 1, 1, 0, D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
                ResourceArgs.m_appDesc = AppResourceDesc(ResourceArgs.m_desc12, RESOURCE_USAGE_DEFAULT, RESOURCE_CPU_ACCESS_NONE, RESOURCE_BIND_DECODER);

                UINT64 resourceSize = 0;
                m_pImmediateContext->m_pDevice12->GetCopyableFootprints(&ResourceArgs.m_desc12, 0, 1, 0, nullptr, nullptr, nullptr, &resourceSize);
                ResourceArgs.m_heapDesc = CD3DX12_HEAP_DESC(resourceSize, m_pImmediateContext->GetHeapProperties(D3D12_HEAP_TYPE_DEFAULT));

                unique_comptr<Resource> spReferenceOnlyTextureArray = Resource::CreateResource(m_pImmediateContext, ResourceArgs, ResourceAllocationContext::ImmediateContextThreadLongLived);

                for (size_t i = 0; i < referenceDatas.size(); i++)
                {
                    referenceDatas[i].referenceOnlyTexture = spReferenceOnlyTextureArray.get();
                    referenceDatas[i].referenceTexture = spReferenceOnlyTextureArray.get();
                    referenceDatas[i].subresourceIndex = static_cast<UINT>(i);
                }
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void ReferenceDataManager::ResizeDataStructures(UINT size)
    {
        textures.resize(size);
        texturesSubresources.resize(size);
        decoderHeapsParameter.resize(size);
        referenceDatas.resize(size);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void ReferenceDataManager::ResetReferenceFramesInformation()
    {
        for (UINT index = 0; index < Size(); index++)
        {
            textures[index] = nullptr;
            texturesSubresources[index] = 0;
            decoderHeapsParameter[index] = nullptr;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void ReferenceDataManager::ResetInternalTrackingReferenceUsage()
    {
        for (UINT index = 0; index < Size(); index++)
        {
            referenceDatas[index].fUsed = false;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void ReferenceDataManager::TransitionReference(ReferenceData& referenceData, D3D12_RESOURCE_STATES decodeState)
    {
        AppResourceDesc* pAppDesc = referenceData.referenceTexture->AppDesc();
        VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL viewDesc = { pAppDesc->Format(), /*MipSlice=*/ 0, /*ArraySlice=*/ referenceData.subresourceIndex};

        const UINT8 MipLevels = pAppDesc->MipLevels();
        const UINT16 ArraySize = pAppDesc->ArraySize(); 
        const UINT8 PlaneCount = (referenceData.referenceTexture->SubresourceMultiplier() * pAppDesc->NonOpaquePlaneCount());

        CViewSubresourceSubset SubresourceSubset(viewDesc, MipLevels, ArraySize, PlaneCount);

        m_pImmediateContext->GetResourceStateManager().TransitionSubresources(
            referenceData.referenceTexture, 
            SubresourceSubset, 
            decodeState, 
            COMMAND_LIST_TYPE::VIDEO_DECODE);
    }
};
