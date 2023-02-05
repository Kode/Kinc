// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <dxva.h>
#include <intsafe.h>

namespace D3D12TranslationLayer
{
    // Arbitrary constants needed for buffer size computation. Worst case scenario is assumed to be 8K.
    const UINT MAX_WIDTH = 8192;
    const UINT MAX_HEIGHT = 4096;
    const UINT MIN_WIDTH = 256;
    const UINT MIN_HEIGHT = 256;
    const UINT MIN_ALIGN = 64;

    /////////////////////////////////////////
    // Decoder configuration
    /////////////////////////////////////////
    typedef struct {
        UINT BufferTypeCount;
        struct {
            VIDEO_DECODE_BUFFER_TYPE Type;
            UINT BaseSize;
        } Data[VIDEO_DECODE_BUFFER_TYPE_SIZEOF];
        struct {
            UINT Num;
            UINT Denom;
            UINT MinWidth;          // if clip resolution is greater than this, use multiplier, otherwise, use 1:1 ratio
            UINT MinHeight;
        } CompressedStreamMultiplier;
    } ProfileBufferInfo;

    //
    // As per codec team, we will be assuming 50% compression over max YUV size for the newer codecs, 0% compression for older codecs.
    //
    ProfileBufferInfo VP9BufferInfo =
    {
        3,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_VP9),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_Slice_VPx_Short),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        // Assuming compresion ratio 1:1 for VP9 as we got bug reports for VP9 clips hitting corruption due to insufficient bitstream size
        { 1, 1, 0, 0 }
    };
    ProfileBufferInfo VP8BufferInfo =
    {
        3,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_VP8),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_Slice_VPx_Short),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 2, 0, 0 },
    };
    ProfileBufferInfo H264BufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_H264),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_Qmatrix_H264),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_Slice_H264_Short),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 2, 1280, 720 },
    };
    ProfileBufferInfo H264MVCBufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_H264_MVC),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_Qmatrix_H264),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_Slice_H264_Short),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 2, 1280, 720 },
    };
    ProfileBufferInfo HEVCBufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_HEVC),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_Qmatrix_HEVC),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_Slice_HEVC_Short),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 2, 1280, 720 },
    };

    ProfileBufferInfo MPEG2BufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PictureParameters),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_QmatrixData),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_SliceInfo),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 1, 0, 0 },
    };
    ProfileBufferInfo VC1BufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PictureParameters),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_QmatrixData),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_SliceInfo),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 1, 0, 0 },
    };
    ProfileBufferInfo MPEG4PT2BufferInfo =
    {
        4,
        {
            VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS, sizeof(DXVA_PicParams_MPEG4_PART2),
            VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX, sizeof(DXVA_QmatrixData),
            VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL, sizeof(DXVA_SliceInfo),
            VIDEO_DECODE_BUFFER_TYPE_BITSTREAM, 0
        },
        { 1, 1, 0, 0 },
    };

    // Profile Info
    // Each DecodeProfile must be unique
    // Each unique combination of DecodeProfileType and DecodeProfileBitDepth must map to exactly one DecodeProfile.
    struct ProfileInfo {
        GUID DecodeProfile;
        VIDEO_DECODE_PROFILE_TYPE DecodeProfileType;
        VIDEO_DECODE_PROFILE_BIT_DEPTH DecodeProfileBitDepth;
        ProfileBufferInfo BufferInfo;
    } AvailableProfiles[] =
    {
        //  DecodeProfile                                           DecodeProfileType                   DecodeProfileBitDepth                       BufferInfo
        {   D3D12_VIDEO_DECODE_PROFILE_MPEG2,                       VIDEO_DECODE_PROFILE_TYPE_MPEG2,    VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       MPEG2BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_MPEG1_AND_MPEG2,             VIDEO_DECODE_PROFILE_TYPE_MPEG2,    VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       MPEG2BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_H264,                        VIDEO_DECODE_PROFILE_TYPE_H264,     VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       H264BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_H264_STEREO_PROGRESSIVE,     VIDEO_DECODE_PROFILE_TYPE_H264,     VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       H264BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_H264_STEREO,                 VIDEO_DECODE_PROFILE_TYPE_H264,     VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       H264BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_H264_MULTIVIEW,              VIDEO_DECODE_PROFILE_TYPE_H264_MVC, VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       H264MVCBufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_VC1,                         VIDEO_DECODE_PROFILE_TYPE_VC1,      VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       VC1BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_VC1_D2010,                   VIDEO_DECODE_PROFILE_TYPE_VC1,      VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       VC1BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_SIMPLE,             VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2, VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       MPEG4PT2BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_MPEG4PT2_ADVSIMPLE_NOGMC,    VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2, VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       MPEG4PT2BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN,                   VIDEO_DECODE_PROFILE_TYPE_HEVC,     VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       HEVCBufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN10,                 VIDEO_DECODE_PROFILE_TYPE_HEVC,     VIDEO_DECODE_PROFILE_BIT_DEPTH_10_BIT,      HEVCBufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_VP9,                         VIDEO_DECODE_PROFILE_TYPE_VP9,      VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       VP9BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_VP9_10BIT_PROFILE2,          VIDEO_DECODE_PROFILE_TYPE_VP9,      VIDEO_DECODE_PROFILE_BIT_DEPTH_10_BIT,      VP9BufferInfo },
        {   D3D12_VIDEO_DECODE_PROFILE_VP8,                         VIDEO_DECODE_PROFILE_TYPE_VP8,      VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT,       VP8BufferInfo },
    };

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDecoder::VideoDecoder(
        ImmediateContext *pContext, 
        ID3D12VideoDevice* pVideoDeviceNoRef,
        const D3D12_VIDEO_DECODER_DESC& desc
        ) : DeviceChildImpl(pContext)
    {
        ThrowFailure(pVideoDeviceNoRef->CreateVideoDecoder(&desc, IID_PPV_ARGS(GetForCreate())));
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDecoderHeap::VideoDecoderHeap(
        ImmediateContext *pContext, 
        ID3D12VideoDevice* pVideoDeviceNoRef,
        const D3D12_VIDEO_DECODER_HEAP_DESC& desc
        ) : DeviceChildImpl(pContext)
    {
        ThrowFailure(pVideoDeviceNoRef->CreateVideoDecoderHeap(&desc, IID_PPV_ARGS(GetForCreate())));
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDecode::VideoDecode(_In_ ImmediateContext *pDevice, VideoDecodeCreationArgs const& args)
            : DeviceChild(pDevice)
            , m_decodingStatus(pDevice)
            , m_profileType(GetProfileType(args.Desc.DecodeProfile))
            , m_decodeFormat(args.Desc.DecodeFormat)
    {
        if (!m_pParent->m_pDevice12_1)
        {
            ThrowFailure(E_NOINTERFACE);
        }
        ThrowFailure(m_pParent->m_pDevice12_1->QueryInterface(&m_spVideoDevice));

        D3D12_VIDEO_DECODE_CONFIGURATION decodeConfiguration = { args.Desc.DecodeProfile, D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE, args.Config.InterlaceType };

        D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT decodeSupport = {};
        decodeSupport.NodeIndex = m_pParent->GetNodeIndex();
        decodeSupport.Configuration = decodeConfiguration;
        decodeSupport.Width = args.Desc.Width;
        decodeSupport.Height = args.Desc.Height;
        decodeSupport.DecodeFormat = args.Desc.DecodeFormat;
        // no info from DX11 on framerate/bitrate
        decodeSupport.FrameRate.Numerator = 0;
        decodeSupport.FrameRate.Denominator = 0;
        decodeSupport.BitRate = 0;

        ThrowFailure(m_spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &decodeSupport, sizeof(decodeSupport)));
        if (!(decodeSupport.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED))
        {
            ThrowFailure(E_INVALIDARG);
        }

        m_configurationFlags = decodeSupport.ConfigurationFlags;
        m_tier = decodeSupport.DecodeTier;

        m_decoderDesc.NodeMask = m_pParent->GetNodeMask();
        m_decoderDesc.Configuration = decodeConfiguration;
        m_spVideoDecoder = std::make_unique<VideoDecoder>(m_pParent, m_spVideoDevice.get(), m_decoderDesc);

        m_decoderHeapDesc.NodeMask = m_pParent->GetNodeMask();
        m_decoderHeapDesc.Configuration = decodeConfiguration;
        m_decoderHeapDesc.DecodeWidth = args.Desc.Width;
        m_decoderHeapDesc.DecodeHeight = args.Desc.Height;
        m_decoderHeapDesc.Format = args.Desc.DecodeFormat;
        m_decoderHeapDesc.MaxDecodePictureBufferCount = 0;

        m_ConfigDecoderSpecific = args.Config.ConfigDecoderSpecific;

        VIDEO_DECODE_PROFILE_BIT_DEPTH bitDepth = GetProfileBitDepth(args.Desc.DecodeProfile);
        m_DecodeProfilePerBitDepth[GetIndex(bitDepth)] = args.Desc.DecodeProfile;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VideoDecode::~VideoDecode() noexcept
    {
        // Stop exception here, as destructor is noexcept
        try {
            m_pParent->Flush(COMMAND_LIST_TYPE_VIDEO_DECODE_MASK); // throws
        }
        catch (_com_error&)
        {
            // success = false;
        }
        catch (std::bad_alloc&)
        {
            // success = false;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void VideoDecode::ManageResolutionChange(const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments)
    {
        UINT width;
        UINT height;
        UINT16 maxDPB;
        ThrowFailure(GetDecodeFrameInfo(&width, &height, &maxDPB));

        ID3D12Resource *pTextureArray = pOutputArguments->pOutputTexture2D->GetUnderlyingResource();
        if (!pTextureArray)
        {
            ThrowFailure(E_INVALIDARG);
        }

        D3D12_RESOURCE_DESC outputResourceDesc = pTextureArray->GetDesc();
        VIDEO_DECODE_PROFILE_BIT_DEPTH resourceBitDepth = GetFormatBitDepth(outputResourceDesc.Format);

        if (m_decodeFormat != outputResourceDesc.Format)
        {
            D3D12_VIDEO_DECODER_DESC decoderDesc = m_decoderDesc;
            decoderDesc.Configuration.DecodeProfile = GetDecodeProfile(m_profileType, resourceBitDepth);
            m_spVideoDecoder = std::make_unique<VideoDecoder>(m_pParent, m_spVideoDevice.get(), decoderDesc);
            m_decoderDesc = decoderDesc;
        }

        if (   !m_spCurrentDecoderHeap
            || m_decodeFormat != outputResourceDesc.Format
            || m_decoderHeapDesc.DecodeWidth != width
            || m_decoderHeapDesc.DecodeHeight != height
            || m_decoderHeapDesc.MaxDecodePictureBufferCount < maxDPB)
        {
            
            UINT16 referenceCount = maxDPB;
            bool fArrayOfTexture = false;
            bool fReferenceOnly = (m_configurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_REFERENCE_ONLY_ALLOCATIONS_REQUIRED) != 0;

            ReferenceOnlyDesc* pReferenceOnlyDesc = nullptr;
            ReferenceOnlyDesc referenceOnlyDesc;
            referenceOnlyDesc.Width = outputResourceDesc.Width;
            referenceOnlyDesc.Height = outputResourceDesc.Height;
            referenceOnlyDesc.Format = outputResourceDesc.Format;

            if (pOutputArguments->ConversionArguments.Enable)
            {
                // Decode output conversion is on, create a DPB only array to hold the references. 
                // All indices are re-mapped in host decoder to address just the size of the DPB array (given by ReferenceFrameCount).
                referenceCount = (UINT16)pOutputArguments->ConversionArguments.ReferenceFrameCount;

                referenceOnlyDesc.Width = pOutputArguments->ConversionArguments.ReferenceInfo.Width;
                referenceOnlyDesc.Height = pOutputArguments->ConversionArguments.ReferenceInfo.Height;
                referenceOnlyDesc.Format = pOutputArguments->ConversionArguments.ReferenceInfo.Format.Format;
                pReferenceOnlyDesc = &referenceOnlyDesc;
                
            }
            else if (fReferenceOnly)
            {
                pReferenceOnlyDesc = &referenceOnlyDesc;
            }
            
            if (outputResourceDesc.DepthOrArraySize != 1)
            {
                // When DepthOrArraySize is not 1 Enable Texture Array Mode.  This selection
                // is made regardless of ConfigDecoderSpecific during decode creation.
                // The reference indices are in a range of zero to the ArraySize and refer 
                // directly to array subresources.
                referenceCount = outputResourceDesc.DepthOrArraySize;
            }
            else
            {
                // A DepthOrArraySize of 1 indicates that Array of Texture Mode is enabled.
                // The reference indices are not in the range of 0 to MaxDPB, but instead 
                // are in a range determined by the caller that the driver doesn't appear to have
                // a way of knowing.  To optimize the reference only case, 11on12 must support 
                // a level of indirection to map the callers indices into references.

                assert(m_tier >= D3D12_VIDEO_DECODE_TIER_2 || fReferenceOnly);
                fArrayOfTexture = m_tier >= D3D12_VIDEO_DECODE_TIER_2;
            }

            m_referenceDataManager.Resize(referenceCount, pReferenceOnlyDesc, fArrayOfTexture);           // throw( bad alloc )

            D3D12_VIDEO_DECODER_HEAP_DESC decoderHeapDesc = m_decoderHeapDesc;
            decoderHeapDesc.Configuration.DecodeProfile = GetDecodeProfile(m_profileType, resourceBitDepth);
            decoderHeapDesc.DecodeWidth = width;
            decoderHeapDesc.DecodeHeight = height;
            decoderHeapDesc.Format = outputResourceDesc.Format;
            decoderHeapDesc.MaxDecodePictureBufferCount = maxDPB;
            m_spCurrentDecoderHeap = std::make_shared<VideoDecoderHeap>(m_pParent, m_spVideoDevice.get(), decoderHeapDesc);
            m_decoderHeapDesc = decoderHeapDesc;
        }

        m_decodeFormat = outputResourceDesc.Format;
    }

    struct ETW_Pic_Entry
    {
        UINT8 Index7Bits;
        UINT8 AssociatedFlag;
        UINT8 bPicEntry;
    };

    //----------------------------------------------------------------------------------------------------------------------------------
    template <typename T>
    static ETW_Pic_Entry LogCopyPicEntry(const T& src)
    {
        ETW_Pic_Entry dest;
        dest.Index7Bits = src.Index7Bits;
        dest.AssociatedFlag = src.AssociatedFlag;
        dest.bPicEntry = src.bPicEntry;

        return dest;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template<size_t dstPicEntriesSize, typename T, size_t srcPicEntriesSize> 
    static void LogCopyPicEntries(ETW_Pic_Entry (&dstPicEntries)[dstPicEntriesSize], T (&srcPicEntries)[srcPicEntriesSize], UINT16& copiedPicEntries)
    {
        static_assert(dstPicEntriesSize >= srcPicEntriesSize, "Dst must be large enough to hold all of src.");                

        for (UINT16 i(0); i < srcPicEntriesSize; ++i)
        {
            dstPicEntries[i] = LogCopyPicEntry(srcPicEntries[i]);
        }

        copiedPicEntries = static_cast<UINT16>(srcPicEntriesSize);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    void VideoDecode::LogPicParams() const
    {
        if (   g_hTracelogging
            && TraceLoggingProviderEnabled(g_hTracelogging, 0, 0))
        {    
            static_assert(sizeof(ETW_Pic_Entry) == 3);
    
            //const size_t MaxRefPicListLength = max(_countof(DXVA_PicParams_H264::RefFrameList), max(_countof(DXVA_PicParams_HEVC::RefPicList), _countof(DXVA_PicParams_VP9::ref_frame_map)));
            //constexpr size_t MaxRefPicListLength = max(std::extent<decltype(DXVA_PicParams_H264::RefFrameList)>::value, max(std::extent<decltype(DXVA_PicParams_HEVC::RefPicList)>::value, std::extent<decltype(DXVA_PicParams_VP9::ref_frame_map)>::value));

            const size_t MaxRefPicListLength = 
                _countof(DXVA_PicParams_H264::RefFrameList) > _countof(DXVA_PicParams_H264_MVC::RefFrameList) ? _countof(DXVA_PicParams_H264::RefFrameList) 
                : _countof(DXVA_PicParams_H264_MVC::RefFrameList) > _countof(DXVA_PicParams_HEVC::RefPicList) ? _countof(DXVA_PicParams_H264_MVC::RefFrameList) 
                : _countof(DXVA_PicParams_HEVC::RefPicList) > _countof(DXVA_PicParams_VP9::ref_frame_map) ? _countof(DXVA_PicParams_HEVC::RefPicList) : _countof(DXVA_PicParams_VP9::ref_frame_map);
    
            ETW_Pic_Entry CurrPic = {};
            ETW_Pic_Entry RefPicList[MaxRefPicListLength] = {};
    
            UINT16 RefPicListLength = 0;
    
            switch (m_profileType)
            {
            case VIDEO_DECODE_PROFILE_TYPE_VP9:
            {
                // From the VP9 DXVA Spec:
                // If bPicEntry is not 0xFF, the entry may be used as a reference surface for decoding the current picture or 
                // a subsequent picture in decoding order. All uncompressed surfaces that correspond to frames that may be used for reference 
                // in the decoding process of the current picture or any subsequent picture shall be present in the ref_frame_map[] array 
                auto pPicParams = GetPicParams<DXVA_PicParams_VP9>();
                LogCopyPicEntries(RefPicList, pPicParams->ref_frame_map, RefPicListLength);
                CurrPic = LogCopyPicEntry(pPicParams->CurrPic);
            }
            break;
    
            case VIDEO_DECODE_PROFILE_TYPE_VP8:
            {
                // From the VP8 DXVA Spec:
                // Specify the frame buffer/surface indices for the altref frame, the golden frame, and the previous reconstructed frame. 
                // In this context, the AssociatedFlag has no meaning and shall be 0, and the accelerator shall ignore its value. 
                // The VP8 decoder needs to maintain four YUV frame buffers/surfaces for decoding purposes. 
                // These buffers hold the current frame being reconstructed, the previous reconstructed frame, the most recent golden frame, 
                // and the most recent altref frame 
                auto pPicParams = GetPicParams<DXVA_PicParams_VP8>();
                RefPicListLength = 3;
                RefPicList[0] = LogCopyPicEntry(pPicParams->alt_fb_idx);
                RefPicList[1] = LogCopyPicEntry(pPicParams->gld_fb_idx);
                RefPicList[2] = LogCopyPicEntry(pPicParams->lst_fb_idx);
                CurrPic = LogCopyPicEntry(pPicParams->CurrPic);
            }
            break;
    
            case VIDEO_DECODE_PROFILE_TYPE_HEVC:
            {
                // From the H265 DXVA Spec:
                // Index7Bits
                //     An index that identifies an uncompressed surface for the CurrPic or RefPicList member of the picture parameters structure(section 4.0).
                //     When Index7Bits is used in the CurrPic and RefPicList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
                //     When Index7Bits is 127 (0x7F), this indicates that it does not contain a valid index.
                auto pPicParams = GetPicParams<DXVA_PicParams_HEVC>();
                LogCopyPicEntries(RefPicList, pPicParams->RefPicList, RefPicListLength);
                CurrPic = LogCopyPicEntry(pPicParams->CurrPic);
            }
            break;
    
            case VIDEO_DECODE_PROFILE_TYPE_H264:
            {
                // From H264 DXVA spec:
                // Index7Bits
                //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture parameters structure(section 4.0) or the RefPicList member of the slice control data structure(section 6.0)
                //     When Index7Bits is used in the CurrPic and RefFrameList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
                //     When Index7Bits is used in the RefPicList member of the slice control data structure, the value identifies the surface indirectly, as an index into the RefFrameList array of the associated picture parameters structure.For more information, see section 6.2.
                //     In all cases, when Index7Bits does not contain a valid index, the value is 127.
                auto pPicParams = GetPicParams<DXVA_PicParams_H264>();
                LogCopyPicEntries(RefPicList, pPicParams->RefFrameList, RefPicListLength);
                CurrPic = LogCopyPicEntry(pPicParams->CurrPic);
            }
            break;

            
            case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
            {
                // From H264 DXVA spec:
                // Index7Bits
                //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture parameters structure(section 4.0) or the RefPicList member of the slice control data structure(section 6.0)
                //     When Index7Bits is used in the CurrPic and RefFrameList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
                //     When Index7Bits is used in the RefPicList member of the slice control data structure, the value identifies the surface indirectly, as an index into the RefFrameList array of the associated picture parameters structure.For more information, see section 6.2.
                //     In all cases, when Index7Bits does not contain a valid index, the value is 127.
                auto pPicParams = GetPicParams<DXVA_PicParams_H264_MVC>();
                LogCopyPicEntries(RefPicList, pPicParams->RefFrameList, RefPicListLength);
                CurrPic = LogCopyPicEntry(pPicParams->CurrPic);
            }
            break;
    
            case VIDEO_DECODE_PROFILE_TYPE_VC1:
            case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
            {
                auto pPicParams = GetPicParams<DXVA_PictureParameters>();
                RefPicListLength = 2;
    
                RefPicList[0].Index7Bits = static_cast<BYTE>(pPicParams->wForwardRefPictureIndex);
                RefPicList[1].Index7Bits = static_cast<BYTE>(pPicParams->wBackwardRefPictureIndex);
                
                CurrPic.Index7Bits = static_cast<BYTE>(pPicParams->wDecodedPictureIndex);
            }
            break;
    
            case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
            {
                auto pPicParams = GetPicParams<DXVA_PicParams_MPEG4_PART2>();
                RefPicListLength = 2;
    
                RefPicList[0].Index7Bits = static_cast<BYTE>(pPicParams->wForwardRefPictureIndex);
                RefPicList[1].Index7Bits = static_cast<BYTE>(pPicParams->wBackwardRefPictureIndex);
                
                CurrPic.Index7Bits = static_cast<BYTE>(pPicParams->wDecodedPictureIndex);
            }
            break;
    
            default:
                ThrowFailure(E_NOTIMPL);
                break;
            }
    
            TraceLoggingWrite(g_hTracelogging,
                "DecodePictureLists",
                TraceLoggingPointer(m_spVideoDecoder->GetForImmediateUse(), "pID3D12Decoder"),
                TraceLoggingStruct(3, "CurrPic"),
                    TraceLoggingUInt8(CurrPic.Index7Bits, "Index7Bits"),
                    TraceLoggingUInt8(CurrPic.AssociatedFlag, "AssociatedFlag"),
                    TraceLoggingUInt8(CurrPic.bPicEntry, "bPicEntry"),
                TraceLoggingPackedData(&RefPicListLength, sizeof(RefPicListLength)), // Data for the array count
                TraceLoggingPackedData(RefPicList, sizeof(RefPicList)), // Data for the array content
                TraceLoggingPackedStructArray(3, "RefPicList"), // Structure metadata 
                    TraceLoggingPackedMetadata(TlgInUINT8, "Index7Bits"),
                    TraceLoggingPackedMetadata(TlgInUINT8, "AssociatedFlag"),
                    TraceLoggingPackedMetadata(TlgInUINT8, "bPicEntry"));
        }
    }
    
    //----------------------------------------------------------------------------------------------------------------------------------
    void VideoDecode::ReleaseUnusedReferences()
    {
        // Method overview
        // 1. Clear the following m_referenceDataManager descriptors: textures, textureSubresources and decoder heap by calling m_referenceDataManager.ResetReferenceFramesInformation()        
        // 2. Codec specific strategy in switch statement regarding reference frames eviction policy
        // 3. Call m_referenceDataManager.ReleaseUnusedReferences(); at the end of this method. Any references (and texture allocations associated) that were left not marked as used in m_referenceDataManager by step (2) are lost.
        
        m_referenceDataManager.ResetReferenceFramesInformation();

        switch (m_profileType)
        {
        case VIDEO_DECODE_PROFILE_TYPE_VP9:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();

            m_referenceDataManager.MarkReferencesInUse(GetPicParams<DXVA_PicParams_VP9>()->ref_frame_map);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VP8:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();
            
            auto pPicParams = GetPicParams<DXVA_PicParams_VP8>();
            m_referenceDataManager.MarkReferenceInUse(pPicParams->alt_fb_idx.Index7Bits);
            m_referenceDataManager.MarkReferenceInUse(pPicParams->gld_fb_idx.Index7Bits);
            m_referenceDataManager.MarkReferenceInUse(pPicParams->lst_fb_idx.Index7Bits);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_HEVC:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();

            m_referenceDataManager.MarkReferencesInUse(GetPicParams<DXVA_PicParams_HEVC>()->RefPicList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();

            m_referenceDataManager.MarkReferencesInUse(GetPicParams<DXVA_PicParams_H264>()->RefFrameList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();

            m_referenceDataManager.MarkReferencesInUse(GetPicParams<DXVA_PicParams_H264>()->RefFrameList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VC1:
        case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
        {
            auto pPicParams = GetPicParams<DXVA_PictureParameters>();

            // If the current frame uses no references, don't evict the current active references as future frames might use them as references again
            if ((pPicParams->wForwardRefPictureIndex != DXVA_INVALID_PICTURE_INDEX)
                || (pPicParams->wBackwardRefPictureIndex != DXVA_INVALID_PICTURE_INDEX))
            {
                // References residency policy for frames that use at least one reference: Mark all references as unused and only mark again as used the ones used by this frame
                m_referenceDataManager.ResetInternalTrackingReferenceUsage();
            }
            
            m_referenceDataManager.MarkReferenceInUse(pPicParams->wForwardRefPictureIndex);
            m_referenceDataManager.MarkReferenceInUse(pPicParams->wBackwardRefPictureIndex);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
        {
            // References residency policy: Mark all references as unused and only mark again as used the ones used by this frame
            m_referenceDataManager.ResetInternalTrackingReferenceUsage();

            auto pPicParams = GetPicParams<DXVA_PicParams_MPEG4_PART2>();
            m_referenceDataManager.MarkReferenceInUse(pPicParams->wForwardRefPictureIndex);
            m_referenceDataManager.MarkReferenceInUse(pPicParams->wBackwardRefPictureIndex);
        }
        break;

        default:
            ThrowFailure(E_NOTIMPL);
            break;
        }

        // Releases the underlying reference picture texture objects of all references that were not marked as used in this method.
        m_referenceDataManager.ReleaseUnusedReferences();
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::PrepareForDecodeFrame(const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments, const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments)
    {
        UNREFERENCED_PARAMETER(pInputArguments);

        if (!pOutputArguments->pOutputTexture2D)
        {
            ThrowFailure(E_INVALIDARG);
        }

        LogPicParams();

        ReleaseUnusedReferences();

        {
            ManageResolutionChange(pOutputArguments);

            UpdateCurrPic(
                pOutputArguments->pOutputTexture2D, 
                pOutputArguments->SubresourceSubset.MinSubresource());
        }

        switch (m_profileType)
        {
        case VIDEO_DECODE_PROFILE_TYPE_VP9:
        {
            // From the VP9 DXVA Spec:
            // If bPicEntry is not 0xFF, the entry may be used as a reference surface for decoding the current picture or 
            // a subsequent picture in decoding order. All uncompressed surfaces that correspond to frames that may be used for reference 
            // in the decoding process of the current picture or any subsequent picture shall be present in the ref_frame_map[] array 
            m_referenceDataManager.UpdateEntries(GetPicParams<DXVA_PicParams_VP9>()->ref_frame_map);

            // frame_refs lists the references used for the current decode operation
            m_referenceDataManager.GetUpdatedEntries(GetPicParams<DXVA_PicParams_VP9>()->frame_refs);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VP8:
        {
            // From the VP8 DXVA Spec:
            // Specify the frame buffer/surface indices for the altref frame, the golden frame, and the previous reconstructed frame. 
            // In this context, the AssociatedFlag has no meaning and shall be 0, and the accelerator shall ignore its value. 
            // The VP8 decoder needs to maintain four YUV frame buffers/surfaces for decoding purposes. 
            // These buffers hold the current frame being reconstructed, the previous reconstructed frame, the most recent golden frame, 
            // and the most recent altref frame 
            auto pPicParams = GetPicParams<DXVA_PicParams_VP8>();
            pPicParams->alt_fb_idx.Index7Bits = m_referenceDataManager.UpdateEntry(pPicParams->alt_fb_idx.Index7Bits);
            pPicParams->gld_fb_idx.Index7Bits = m_referenceDataManager.UpdateEntry(pPicParams->gld_fb_idx.Index7Bits);
            pPicParams->lst_fb_idx.Index7Bits = m_referenceDataManager.UpdateEntry(pPicParams->lst_fb_idx.Index7Bits);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_HEVC:
        {
            // From the H265 DXVA Spec:
            // Index7Bits
            //     An index that identifies an uncompressed surface for the CurrPic or RefPicList member of the picture parameters structure(section 4.0).
            //     When Index7Bits is used in the CurrPic and RefPicList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
            //     When Index7Bits is 127 (0x7F), this indicates that it does not contain a valid index.
            m_referenceDataManager.UpdateEntries(GetPicParams<DXVA_PicParams_HEVC>()->RefPicList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264:
        {
            // From H264 DXVA spec:
            // Index7Bits
            //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture parameters structure(section 4.0) or the RefPicList member of the slice control data structure(section 6.0)
            //     When Index7Bits is used in the CurrPic and RefFrameList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
            //     When Index7Bits is used in the RefPicList member of the slice control data structure, the value identifies the surface indirectly, as an index into the RefFrameList array of the associated picture parameters structure.For more information, see section 6.2.
            //     In all cases, when Index7Bits does not contain a valid index, the value is 127.
            m_referenceDataManager.UpdateEntries(GetPicParams<DXVA_PicParams_H264>()->RefFrameList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
        {
            // From H264 DXVA spec:
            // Index7Bits
            //     An index that identifies an uncompressed surface for the CurrPic or RefFrameList member of the picture parameters structure(section 4.0) or the RefPicList member of the slice control data structure(section 6.0)
            //     When Index7Bits is used in the CurrPic and RefFrameList members of the picture parameters structure, the value directly specifies the DXVA index of an uncompressed surface.
            //     When Index7Bits is used in the RefPicList member of the slice control data structure, the value identifies the surface indirectly, as an index into the RefFrameList array of the associated picture parameters structure.For more information, see section 6.2.
            //     In all cases, when Index7Bits does not contain a valid index, the value is 127.
            m_referenceDataManager.UpdateEntries(GetPicParams<DXVA_PicParams_H264>()->RefFrameList);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VC1:
        case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
        {
            auto pPicParams = GetPicParams<DXVA_PictureParameters>();

            constexpr UINT VC1_PICDEBLOCKED_DEBLOCKING_BIT = 2;
            constexpr UINT VC1_PICDEBLOCKED_DERINGING_BIT = 3;
            constexpr UINT VC1_PICDEBLOCKED_REDUCED_DYNAMIC_RANGE_BIT = 5;
            constexpr UINT postProcessingOptions = (1 << VC1_PICDEBLOCKED_DEBLOCKING_BIT) |
                                               (1 << VC1_PICDEBLOCKED_DERINGING_BIT) |
                                               (1 << VC1_PICDEBLOCKED_REDUCED_DYNAMIC_RANGE_BIT);

            // No post-processing allowed, so we need to modify picture parameters for certain profiles where it can be turned on
            pPicParams->wDeblockedPictureIndex = DXVA_INVALID_PICTURE_INDEX;
            pPicParams->bPicDeblocked &= ~postProcessingOptions;
            pPicParams->bPicOBMC = 0;
            pPicParams->bPicBinPB = 0;

            pPicParams->wForwardRefPictureIndex = m_referenceDataManager.UpdateEntry(pPicParams->wForwardRefPictureIndex);
            pPicParams->wBackwardRefPictureIndex = m_referenceDataManager.UpdateEntry(pPicParams->wBackwardRefPictureIndex);
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_MPEG4_PART2>();

            // No post-processing allowed, so we need to modify picture parameters for certain profiles where it can be turned on
            pPicParams->wDeblockedPictureIndex = DXVA_INVALID_PICTURE_INDEX;
            pPicParams->unPicPostProc = 0;

            pPicParams->wForwardRefPictureIndex = m_referenceDataManager.UpdateEntry(pPicParams->wForwardRefPictureIndex);
            pPicParams->wBackwardRefPictureIndex = m_referenceDataManager.UpdateEntry(pPicParams->wBackwardRefPictureIndex);
        }
        break;

        default:
            ThrowFailure(E_NOTIMPL);
            break;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::DecodeFrame(const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments, const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments)
    {
        CachePicParams(pInputArguments);

        // translate input D3D12 structure
        D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS d3d12InputArguments = {};

        d3d12InputArguments.CompressedBitstream.pBuffer = pInputArguments->CompressedBitstream.pBuffer->GetUnderlyingResource();
        d3d12InputArguments.CompressedBitstream.Offset = pInputArguments->CompressedBitstream.Offset + pInputArguments->CompressedBitstream.pBuffer->GetSubresourcePlacement(0).Offset;
        d3d12InputArguments.CompressedBitstream.Size = pInputArguments->CompressedBitstream.Size;

        m_pParent->GetResourceStateManager().TransitionResource(pInputArguments->CompressedBitstream.pBuffer, D3D12_RESOURCE_STATE_VIDEO_DECODE_READ, COMMAND_LIST_TYPE::VIDEO_DECODE);

        PrepareForDecodeFrame(pInputArguments, pOutputArguments);

        d3d12InputArguments.NumFrameArguments = pInputArguments->FrameArgumentsCount;
        for (UINT i = 0; i < d3d12InputArguments.NumFrameArguments; i++)
        {
            D3D12_VIDEO_DECODE_FRAME_ARGUMENT& frameArgument = d3d12InputArguments.FrameArguments[i];
            frameArgument = pInputArguments->FrameArguments[i];

            if (frameArgument.Type == D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS)
            {
                assert(frameArgument.Size == m_modifiablePicParamsAllocationSize);
                frameArgument.pData = GetPicParams();
            }
        }
        d3d12InputArguments.ReferenceFrames.ppTexture2Ds = m_referenceDataManager.textures.data();
        d3d12InputArguments.ReferenceFrames.pSubresources = m_referenceDataManager.texturesSubresources.data();
        d3d12InputArguments.ReferenceFrames.NumTexture2Ds = static_cast<UINT>(m_referenceDataManager.Size());
        d3d12InputArguments.pHeap = m_spCurrentDecoderHeap->GetForUse(COMMAND_LIST_TYPE::VIDEO_DECODE);

        // translate output D3D12 structure
        D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS1 d3d12OutputArguments = {};
        d3d12OutputArguments.pOutputTexture2D = pOutputArguments->pOutputTexture2D->GetUnderlyingResource();
        d3d12OutputArguments.OutputSubresource = pOutputArguments->SubresourceSubset.MinSubresource();

        if (m_referenceDataManager.IsReferenceOnly())
        {
            d3d12OutputArguments.ConversionArguments.Enable = TRUE;

            m_referenceDataManager.TransitionReferenceOnlyOutput(d3d12OutputArguments.ConversionArguments.pReferenceTexture2D, d3d12OutputArguments.ConversionArguments.ReferenceSubresource);

            const D3D12_RESOURCE_DESC &descReference = d3d12OutputArguments.ConversionArguments.pReferenceTexture2D->GetDesc();
            d3d12OutputArguments.ConversionArguments.DecodeColorSpace = CDXGIColorSpaceHelper::ConvertFromLegacyColorSpace(!CD3D11FormatHelper::YUV(descReference.Format), CD3D11FormatHelper::GetBitsPerUnit(descReference.Format), /* StudioRGB= */ false, /* P709= */ true, /* StudioYUV= */ true);

            const D3D12_RESOURCE_DESC &descOutput = d3d12OutputArguments.pOutputTexture2D->GetDesc();
            d3d12OutputArguments.ConversionArguments.OutputColorSpace = CDXGIColorSpaceHelper::ConvertFromLegacyColorSpace(!CD3D11FormatHelper::YUV(descOutput.Format), CD3D11FormatHelper::GetBitsPerUnit(descOutput.Format), /* StudioRGB= */ false, /* P709= */ true, /* StudioYUV= */ true);

            const D3D12_VIDEO_DECODER_HEAP_DESC& HeapDesc = m_spCurrentDecoderHeap->GetDesc();
            d3d12OutputArguments.ConversionArguments.OutputWidth = HeapDesc.DecodeWidth;
            d3d12OutputArguments.ConversionArguments.OutputHeight = HeapDesc.DecodeHeight;
        }
        else
        {
            d3d12OutputArguments.ConversionArguments.Enable = FALSE;
        }
        m_pParent->GetResourceStateManager().TransitionSubresources(pOutputArguments->pOutputTexture2D, pOutputArguments->SubresourceSubset, D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE, COMMAND_LIST_TYPE::VIDEO_DECODE);
        
        static_assert(_countof(pOutputArguments->Histograms) == _countof(d3d12OutputArguments.Histograms), "Must keep histogram component count in sync");

        for (UINT i = 0; i < _countof(d3d12OutputArguments.Histograms); i++)
        {
            if (pOutputArguments->Histograms[i].pBuffer)
            {
                d3d12OutputArguments.Histograms[i].pBuffer = pOutputArguments->Histograms[i].pBuffer->GetUnderlyingResource();
                d3d12OutputArguments.Histograms[i].Offset = pOutputArguments->Histograms[i].Offset + pOutputArguments->Histograms[i].pBuffer->GetSubresourcePlacement(0).Offset;

                m_pParent->GetResourceStateManager().TransitionResource(pOutputArguments->Histograms[i].pBuffer, D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE, COMMAND_LIST_TYPE::VIDEO_DECODE);
            }
            else
            {
                d3d12OutputArguments.Histograms[i].pBuffer = nullptr;
                d3d12OutputArguments.Histograms[i].Offset = 0;
            }
        }

        // submit DecodeFrame
        // decode barrier for the output buffer
        m_pParent->GetResourceStateManager().ApplyAllResourceTransitions();

        m_pParent->GetVideoDecodeCommandList()->DecodeFrame1(
            m_spVideoDecoder->GetForUse(COMMAND_LIST_TYPE::VIDEO_DECODE),
            &d3d12OutputArguments,
            &d3d12InputArguments);
        
        UINT statusReportFeedbackNumber;
        DXVA_PicEntry CurrPic;
        UCHAR field_pic_flag;
        GetStatusReportFeedbackNumber(/*_Out_*/statusReportFeedbackNumber, /*_Out_*/CurrPic, /*_Out_*/field_pic_flag);  // throw( _com_error )
        m_decodingStatus.EndQuery(statusReportFeedbackNumber, CurrPic, field_pic_flag);  // throw( _com_error )

        if (g_hTracelogging)
        {
            TraceLoggingWrite(g_hTracelogging,
                "Decode - StatusReportFeedbackNumber",
                TraceLoggingPointer(m_spVideoDecoder->GetForImmediateUse(), "pID3D12Decoder"),
                TraceLoggingValue(statusReportFeedbackNumber, "statusReportFeedbackNumber"));
        }

        m_pParent->SubmitCommandList(COMMAND_LIST_TYPE::VIDEO_DECODE);  // throws
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::CachePicParams(const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments)
    {
        UINT i = 0;
        for (; i < pInputArguments->FrameArgumentsCount; i++)
        {
            const D3D12_VIDEO_DECODE_FRAME_ARGUMENT& frameArgument = pInputArguments->FrameArguments[i];
            if (frameArgument.Type == D3D12_VIDEO_DECODE_ARGUMENT_TYPE_PICTURE_PARAMETERS)
            {
                break;
            }
        }

        if (i >= pInputArguments->FrameArgumentsCount)
        {
            // No pic params.
            ThrowFailure(E_INVALIDARG);
        }

        const D3D12_VIDEO_DECODE_FRAME_ARGUMENT& frameArgument = pInputArguments->FrameArguments[i];

        if (   frameArgument.pData == nullptr
            || frameArgument.Size == 0)
        {
            // Invalid pic params.
            ThrowFailure(E_INVALIDARG);
        }

        if (m_modifiablePicParamsAllocationSize < frameArgument.Size)
        {   
            m_modifiablePicParams.reset(new char[frameArgument.Size]);
            m_modifiablePicParamsAllocationSize = frameArgument.Size;
        }

        memcpy(m_modifiablePicParams.get(), frameArgument.pData, frameArgument.Size);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static inline int LengthFromMinCb(int length, int cbsize)
    {
        return length * (1 << cbsize);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static inline bool IsAdvancedProfile(DXVA_PictureParameters *pPicParams)
    {
        return (((pPicParams->bBidirectionalAveragingMode >> 3) & 1) != 0);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    HRESULT VideoDecode::GetDecodeFrameInfo(UINT *pWidth, UINT *pHeight, UINT16 *pMaxDPB) noexcept
    {
        HRESULT hr = S_OK;

        *pWidth = 0;
        *pHeight = 0;
        *pMaxDPB = 0;

        switch (m_profileType)
        {
        case VIDEO_DECODE_PROFILE_TYPE_VC1:
        {
            auto pPicParams = GetPicParams<DXVA_PictureParameters>();

            if (IsAdvancedProfile(pPicParams))
            {
                *pWidth = pPicParams->wPicWidthInMBminus1 + 1;
                *pHeight = pPicParams->wPicHeightInMBminus1 + 1;
            }
            else
            {
                *pWidth = (pPicParams->wPicWidthInMBminus1 + 1) * (pPicParams->bMacroblockWidthMinus1 + 1);
                *pHeight = (pPicParams->wPicHeightInMBminus1 + 1) * (pPicParams->bMacroblockHeightMinus1 + 1);
            }
            *pMaxDPB = 2 + 1;
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
        {
            auto pPicParams = GetPicParams<DXVA_PictureParameters>();

            if (IsAdvancedProfile(pPicParams))
            {
                *pWidth = pPicParams->wPicWidthInMBminus1 + 1;
                *pHeight = pPicParams->wPicHeightInMBminus1 + 1;
            }
            else
            {
                *pWidth = (pPicParams->wPicWidthInMBminus1 + 1) * (pPicParams->bMacroblockWidthMinus1 + 1);
                *pHeight = (pPicParams->wPicHeightInMBminus1 + 1) * (pPicParams->bMacroblockHeightMinus1 + 1);
            }
            *pMaxDPB = 2 + 1;

            // Code below adjusts pHeight if necessary for interlaced video

            // These constants below correspond to picture_structure parameter of the MPEG2 spec.
            // bPicStructure

            // Indicates whether the current picture is a top - field picture(a value 1), a bottom - field picture(a value 2), or a frame picture(a value 3).
            // In progressive - scan frame - structured coding such as in H.261, bPicStructure is 3. 
            // A derived value PicCurrentField is defined as zero unless bPicStructure is 2 (bottom field).In which case, it is 1. 
            // This member has the same meaning as the picture_structure variable defined in Section 6.3.10 and Table 6 - 14 of MPEG - 2 (H.262).
            [[maybe_unused]] constexpr BYTE TOP_FIELD = 1;
            [[maybe_unused]] constexpr BYTE BOTTOM_FIELD = 2;
            constexpr BYTE FRAME_PICTURE = 3;
            
            // sample field picture has half as many macroblocks as frame
            // but the display height used for the D3D12 decoder must be the full non-interlaced video height, not the interlaced sample height (half as the video display size)
            if (pPicParams->bPicStructure != FRAME_PICTURE)
            {                
                *pHeight <<= 1;
            }
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_MPEG4_PART2>();
            *pWidth = pPicParams->vop_width;
            *pHeight = pPicParams->vop_height;
            *pMaxDPB = 2 + 1;
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_H264>();

            // wFrameWidthInMbsMinus1 Width of the frame containing this picture, in units of macroblocks, minus 1. (The width in macroblocks is wFrameWidthInMbsMinus1 plus 1.)
            // wFrameHeightInMbsMinus1 Height of the frame containing this picture, in units of macroblocks, minus 1. 
            // (The height in macroblocks is wFrameHeightInMbsMinus1 plus 1.) When the picture is a field, the height of the frame is 
            // twice the height of the picture and is an integer multiple of 2 in units of macroblocks.
            *pWidth = (pPicParams->wFrameWidthInMbsMinus1 + 1) * 16;
            *pHeight = (pPicParams->wFrameHeightInMbsMinus1 + 1)/ (pPicParams->frame_mbs_only_flag ? 1 : 2);
            *pHeight = (2 - pPicParams->frame_mbs_only_flag) * *pHeight;
            *pHeight = *pHeight * 16;
            *pMaxDPB = pPicParams->num_ref_frames + 1;
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_H264_MVC>();

            // wFrameWidthInMbsMinus1 Width of the frame containing this picture, in units of macroblocks, minus 1. (The width in macroblocks is wFrameWidthInMbsMinus1 plus 1.)
            // wFrameHeightInMbsMinus1 Height of the frame containing this picture, in units of macroblocks, minus 1. 
            // (The height in macroblocks is wFrameHeightInMbsMinus1 plus 1.) When the picture is a field, the height of the frame is 
            // twice the height of the picture and is an integer multiple of 2 in units of macroblocks.
            *pWidth = (pPicParams->wFrameWidthInMbsMinus1 + 1) * 16;
            *pHeight = (pPicParams->wFrameHeightInMbsMinus1 + 1)/ (pPicParams->frame_mbs_only_flag ? 1 : 2);
            *pHeight = (2 - pPicParams->frame_mbs_only_flag) * *pHeight;
            *pHeight = *pHeight * 16;
            *pMaxDPB = pPicParams->num_ref_frames + 1;
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_HEVC:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_HEVC>();
            UINT log2_min_luma_coding_block_size = pPicParams->log2_min_luma_coding_block_size_minus3 + 3;
            *pWidth = LengthFromMinCb(pPicParams->PicWidthInMinCbsY, log2_min_luma_coding_block_size);
            *pHeight = LengthFromMinCb(pPicParams->PicHeightInMinCbsY, log2_min_luma_coding_block_size);
            *pMaxDPB = pPicParams->sps_max_dec_pic_buffering_minus1 + 1;
        }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VP9:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_VP9>();
            *pWidth = pPicParams->width;
            *pHeight = pPicParams->height;
            *pMaxDPB = _countof(pPicParams->ref_frame_map) + 1;
            }
        break;

        case VIDEO_DECODE_PROFILE_TYPE_VP8:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_VP8>();
            *pWidth = pPicParams->width;
            *pHeight = pPicParams->height;
            *pMaxDPB = 3 + 1;
            }
        break;

        default:
            hr = E_INVALIDARG;
            break;
        }

        if (m_configurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_HEIGHT_ALIGNMENT_MULTIPLE_32_REQUIRED)
        {
            const UINT AlignmentMask = 31;
            *pHeight = (*pHeight + AlignmentMask) & ~AlignmentMask;
        }

        return hr;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::UpdateCurrPic(Resource* pTexture2D, UINT subresourceIndex)
    {
        switch (m_profileType)
        {
        case VIDEO_DECODE_PROFILE_TYPE_VC1:
        case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
        {
            auto pPicParams = GetPicParams<DXVA_PictureParameters>();
            pPicParams->wDecodedPictureIndex = m_referenceDataManager.StoreFutureReference(
                pPicParams->wDecodedPictureIndex, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_MPEG4_PART2>();
            pPicParams->wDecodedPictureIndex = m_referenceDataManager.StoreFutureReference(
                pPicParams->wDecodedPictureIndex, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_H264:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_H264>();
            pPicParams->CurrPic.Index7Bits = m_referenceDataManager.StoreFutureReference(
                pPicParams->CurrPic.Index7Bits, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_H264_MVC>();
            pPicParams->CurrPic.Index7Bits = m_referenceDataManager.StoreFutureReference(
                pPicParams->CurrPic.Index7Bits, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_HEVC:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_HEVC>();
            pPicParams->CurrPic.Index7Bits = m_referenceDataManager.StoreFutureReference(
                pPicParams->CurrPic.Index7Bits, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_VP9:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_VP9>();
            pPicParams->CurrPic.Index7Bits = m_referenceDataManager.StoreFutureReference(
                pPicParams->CurrPic.Index7Bits, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        case VIDEO_DECODE_PROFILE_TYPE_VP8:
        {
            auto pPicParams = GetPicParams<DXVA_PicParams_VP8>();
            pPicParams->CurrPic.Index7Bits = m_referenceDataManager.StoreFutureReference(
                pPicParams->CurrPic.Index7Bits, 
                m_spCurrentDecoderHeap, 
                pTexture2D, 
                subresourceIndex);
        } break;

        default:
            ThrowFailure(E_UNEXPECTED);
            break;
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    template <typename T>
    static void CopyNewStylePicParams(UINT& statusReportFeedbackNumber, DXVA_PicEntry& CurrPic, void* pParams)
    {
        const T* pPicParams = static_cast<const T*>(pParams);
        statusReportFeedbackNumber = pPicParams->StatusReportFeedbackNumber;
        CurrPic.Index7Bits = pPicParams->CurrPic.Index7Bits;
        CurrPic.AssociatedFlag = pPicParams->CurrPic.AssociatedFlag;
        CurrPic.bPicEntry = pPicParams->CurrPic.bPicEntry;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::GetStatusReportFeedbackNumber(UINT& statusReportFeedbackNumber, DXVA_PicEntry& CurrPic, UCHAR& field_pic_flag) noexcept
    {
        void *pParams = GetPicParams();

        if (!pParams)
        {
            return;
        }

        statusReportFeedbackNumber = 0;
        CurrPic.Index7Bits = 0;
        CurrPic.AssociatedFlag = 0;
        CurrPic.bPicEntry = 0;
        field_pic_flag = 0;

        switch (m_profileType)
        {
            case VIDEO_DECODE_PROFILE_TYPE_VC1:
            case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
                {
                    DXVA_PictureParameters *pPicParams = (DXVA_PictureParameters *)pParams;
    
                    // From VC1 spec: StatusReportFeedbackNumber
                    // Shall equal the value of(bPicScanFixed << 8) + bPicSanMethods in the picture parameters structure that the 
                    // host decoder sent in the Execute call for which the accelerator is reporting status information.
                    statusReportFeedbackNumber = (pPicParams->bPicScanFixed << 8) + pPicParams->bPicScanMethod;
                    CurrPic.Index7Bits = static_cast<BYTE>(pPicParams->wDecodedPictureIndex);
                } break;

            case VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2:
                {
                    DXVA_PicParams_MPEG4_PART2 *pPicParams = (DXVA_PicParams_MPEG4_PART2 *)pParams;
                    statusReportFeedbackNumber = pPicParams->StatusReportFeedbackNumber;
                    CurrPic.Index7Bits = static_cast<BYTE>(pPicParams->wDecodedPictureIndex);
                } break;

            case VIDEO_DECODE_PROFILE_TYPE_H264:
                CopyNewStylePicParams<DXVA_PicParams_H264>(statusReportFeedbackNumber, CurrPic, pParams);
                field_pic_flag = static_cast<DXVA_PicParams_H264 *>(pParams)->field_pic_flag;
                break;

            case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
                CopyNewStylePicParams<DXVA_PicParams_H264_MVC>(statusReportFeedbackNumber, CurrPic, pParams);
                field_pic_flag = static_cast<DXVA_PicParams_H264_MVC *>(pParams)->field_pic_flag;
                break;

            case VIDEO_DECODE_PROFILE_TYPE_HEVC:
                CopyNewStylePicParams<DXVA_PicParams_HEVC>(statusReportFeedbackNumber, CurrPic, pParams);
                break;

            case VIDEO_DECODE_PROFILE_TYPE_VP9:
                CopyNewStylePicParams<DXVA_PicParams_VP9>(statusReportFeedbackNumber, CurrPic, pParams);
                break;

            case VIDEO_DECODE_PROFILE_TYPE_VP8:
                CopyNewStylePicParams<DXVA_PicParams_VP8>(statusReportFeedbackNumber, CurrPic, pParams);
                break;

            default:
                ThrowFailure(E_UNEXPECTED);
                break;
            }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static ProfileInfo* GetProfileInfo(_In_ REFGUID DecodeProfile) noexcept
    {
        for (auto& profile : AvailableProfiles)
        {
            if (DecodeProfile == profile.DecodeProfile)
            {
                return &profile;
            }
        }

        return nullptr;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    VIDEO_DECODE_PROFILE_TYPE VideoDecode::GetProfileType(REFGUID DecodeProfile) noexcept
    {
        ProfileInfo* pProfileInfo = GetProfileInfo(DecodeProfile);
        return pProfileInfo ? pProfileInfo->DecodeProfileType : VIDEO_DECODE_PROFILE_TYPE_NONE;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    VIDEO_DECODE_PROFILE_BIT_DEPTH VideoDecode::GetProfileBitDepth(REFGUID DecodeProfile) noexcept
    {
        ProfileInfo* pProfileInfo = GetProfileInfo(DecodeProfile);
        return pProfileInfo ? pProfileInfo->DecodeProfileBitDepth : VIDEO_DECODE_PROFILE_BIT_DEPTH_NONE;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    VIDEO_DECODE_PROFILE_BIT_DEPTH VideoDecode::GetFormatBitDepth(DXGI_FORMAT Format) noexcept
    {
        switch (Format)
        {
            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_YUY2:
            case DXGI_FORMAT_AYUV:
            case DXGI_FORMAT_NV11:
            case DXGI_FORMAT_420_OPAQUE:
                return VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT;

            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_Y410:
            case DXGI_FORMAT_Y210:
                return VIDEO_DECODE_PROFILE_BIT_DEPTH_10_BIT;

            case DXGI_FORMAT_P016:
            case DXGI_FORMAT_Y416:
            case DXGI_FORMAT_Y216:
                return VIDEO_DECODE_PROFILE_BIT_DEPTH_16_BIT;
        }

        assert(false);
        return VIDEO_DECODE_PROFILE_BIT_DEPTH_NONE;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    GUID VideoDecode::GetDecodeProfile(VIDEO_DECODE_PROFILE_TYPE ProfileType, VIDEO_DECODE_PROFILE_BIT_DEPTH BitDepth) noexcept
    {
        // Cache the profile initially chosen for a type and bit depth.
        // When Reuse decoder is enabled, select the same profile for a given bit depth
        // that was previously selected.
        // Codecs that have an 8bit and 10bit representation (HEVC and VP9) currently only have
        // for each bit depth.
        // Only codecs like H264 and VC1 have ambiguity here by having multiple 8bit profiles, so this
        // ensures that the profile doesn't change when the resolution does.

        VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX bitDepthIndex = GetIndex(BitDepth);
        if (!m_DecodeProfilePerBitDepth[bitDepthIndex].has_value())
        {
            m_DecodeProfilePerBitDepth[bitDepthIndex] = GUID_NULL;
            for (auto& profile : AvailableProfiles)
            {
                if (   ProfileType == profile.DecodeProfileType
                    && BitDepth == profile.DecodeProfileBitDepth)
                {
                    m_DecodeProfilePerBitDepth[bitDepthIndex] = profile.DecodeProfile;
                    break;
                }
            }
        }

        return m_DecodeProfilePerBitDepth[bitDepthIndex].value_or(GUID_NULL);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static ProfileBufferInfo *GetProfileBufferInfo(_In_ REFGUID DecodeProfile) noexcept
    {
        ProfileInfo* pProfileInfo = GetProfileInfo(DecodeProfile);
        return pProfileInfo ? &pProfileInfo->BufferInfo : nullptr;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    HRESULT VideoDecode::GetVideoDecoderBufferTypeCount(const VIDEO_DECODE_DESC *pDesc, UINT *pBufferTypeCount)  noexcept
    {
        HRESULT hr = S_OK;
        *pBufferTypeCount = 0;
        ProfileBufferInfo *pProfileBufferInfo = GetProfileBufferInfo(pDesc->DecodeProfile);
        if (pProfileBufferInfo)
        {
            *pBufferTypeCount = pProfileBufferInfo->BufferTypeCount;
        }
        else
        {
            hr = E_INVALIDARG;
        }
        return hr;
    }

    
    //----------------------------------------------------------------------------------------------------------------------------------
    static bool IsXboxReuseDecoderProfileType(VIDEO_DECODE_PROFILE_TYPE ProfileType)
    {
        return ProfileType == VIDEO_DECODE_PROFILE_TYPE_H264
            || ProfileType == VIDEO_DECODE_PROFILE_TYPE_H264_MVC
            || ProfileType == VIDEO_DECODE_PROFILE_TYPE_HEVC
            || ProfileType == VIDEO_DECODE_PROFILE_TYPE_VP9;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::GetVideoDecoderBufferInfo(const VIDEO_DECODE_DESC *pDesc, UINT Index, VIDEO_DECODE_BUFFER_TYPE *pType, UINT *pSize, bool IsXbox)
    {
        *pSize = 0;
        ProfileInfo* pProfileInfo = GetProfileInfo(pDesc->DecodeProfile);
        if (pProfileInfo)
        {
            ProfileBufferInfo *pProfileBufferInfo = &pProfileInfo->BufferInfo;

            UINT Width = pDesc->Width ? ((pDesc->Width < MIN_WIDTH) ? MIN_WIDTH : pDesc->Width) : MAX_WIDTH;
            UINT Height = pDesc->Height ? ((pDesc->Height < MIN_HEIGHT) ? MIN_HEIGHT : pDesc->Height) : MAX_HEIGHT;

            Width = Align(Width, MIN_ALIGN);
            Height = Align(Height, MIN_ALIGN);

            if (Index >= pProfileBufferInfo->BufferTypeCount)
            {
                ThrowFailure(E_INVALIDARG);
            }
            *pType = pProfileBufferInfo->Data[Index].Type;
            *pSize = pProfileBufferInfo->Data[Index].BaseSize;
            switch (*pType)
            {
                case VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL:
                {
                    const UINT MacroblockMinSize = 8;

                    static_assert(MIN_ALIGN % MacroblockMinSize == 0, "MIN_ALIGN must be divisible by MacroblockMinSize");

                    UINT SliceControlBlocks = 0;
                    switch (pProfileInfo->DecodeProfileType)
                    {
                    case VIDEO_DECODE_PROFILE_TYPE_MPEG2:
                        // worst case scenario for MPEG2 is one slice control per macroblock in the picture.
                        SliceControlBlocks = (Width * Height) / MacroblockMinSize;
                        break;

                    case VIDEO_DECODE_PROFILE_TYPE_H264:
                        // worst case scenario for H264 per tables A-1 for Level 6.2 (MaxMBPS = 16711680) and Table A-4 for SliceRate for level 6.2 (SliceRate = 24)
                        // Taking a 4K resolution (3840*2160) @ 300 fps (allowed in level 6.2)                        
                        // From spec ...satisfy the constraint that the number of slices in picture n is less than or equal to MaxMBPB * ( tr(n) - tr(n-1) ) / SliceRate
                        // where MaxMBPS and SliceRate are the values specified in Tables A-1 and A-4...
                        // tr(n) = tr(n-1) = 1 / FPSRate = 1/300
                        // MaxSliceNumber = 2319
                        SliceControlBlocks = 2319;
                        break;

                    case VIDEO_DECODE_PROFILE_TYPE_HEVC:
                        // worst case scenario for HEVC is 600 slices per Table A.4 of the HEVC spec for profile 6.2. Setting to 1024 just in case.
                        SliceControlBlocks = 1024;
                        break;

                    default:
                        // assume worst case scenario is one slice control per row of macroblocks
                        SliceControlBlocks = Height / MacroblockMinSize;
                        break;
                    }
                    //*pSize *= SliceControlBlocks;
                    HRESULT hr = UIntMult(*pSize, SliceControlBlocks, pSize);
                    ThrowFailure(hr);
                }
                break;

                case VIDEO_DECODE_BUFFER_TYPE_BITSTREAM:
                {
                    if (   IsXbox
                        && IsXboxReuseDecoderProfileType(pProfileInfo->DecodeProfileType))
                    {
                        // The width and height queried here are initial sizes for the decoder.
                        // When the decoder supports REUSE_DECODER (xbox only), the resolution
                        // can go up without recreating the decoder or the bitstream buffer, so
                        // a larger size than might be calculated below.  Xbox has found the following
                        // Dimensions to be sufficient for that device regardless of content dimensions.
                        switch(pProfileInfo->DecodeProfileType)
                        {
                            case VIDEO_DECODE_PROFILE_TYPE_H264:
                            case VIDEO_DECODE_PROFILE_TYPE_H264_MVC:
                                *pSize = 1800 * 1024;
                                break;
                            default:
                                assert(pProfileInfo->DecodeProfileType == VIDEO_DECODE_PROFILE_TYPE_HEVC || pProfileInfo->DecodeProfileType == VIDEO_DECODE_PROFILE_TYPE_VP9);
                                *pSize = 6912 * 1024;
                                break;
                        }
                    }
                    else
                    {
                        // *pSize = (Width * Height * 2 * pProfileBufferInfo->CompressedStreamMultiplier.Num) / pProfileBufferInfo->CompressedStreamMultiplier.Denom;
                        // Smaller streams use a 1:1 compressed stream multiplier.  Memory optimization is not as important at these smaller sizes.

                        // Use a 1:1 ratio to calculate a minimum size against the MinWidth/MinHeight
                        UINT minSize;
                        ThrowFailure(UIntMult(pProfileBufferInfo->CompressedStreamMultiplier.MinWidth, pProfileBufferInfo->CompressedStreamMultiplier.MinHeight, &minSize));
                        ThrowFailure(UIntMult(minSize, 2, &minSize));

                        // Use the specified multipliers to calculate a size based on the stream width and height.
                        UINT calcSize;
                        ThrowFailure(UIntMult(Width, Height, &calcSize));
                        ThrowFailure(UIntMult(calcSize, 2 * pProfileBufferInfo->CompressedStreamMultiplier.Num, &calcSize));
                        calcSize /= pProfileBufferInfo->CompressedStreamMultiplier.Denom;

                        // Take the larger of the two.
                        *pSize = std::max(minSize, calcSize);
                    }
                }
                break;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::GetVideoDecoderSupport(ID3D12Device *pDevice12, UINT NodeIndex, const VIDEO_DECODE_DESC *pDesc, D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT &decodeSupport)
    {
        unique_comptr<ID3D12VideoDevice> spVideoDevice;
        ThrowFailure(pDevice12->QueryInterface(&spVideoDevice));
            
        decodeSupport.NodeIndex = NodeIndex;
        decodeSupport.Configuration.DecodeProfile = pDesc->DecodeProfile;
        decodeSupport.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
        decodeSupport.Configuration.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
        decodeSupport.Width = pDesc->Width;
        decodeSupport.Height = pDesc->Height;
        decodeSupport.DecodeFormat = pDesc->DecodeFormat;
        decodeSupport.FrameRate.Numerator = 30;
        decodeSupport.FrameRate.Denominator = 1;
        decodeSupport.BitRate = 0;
        ThrowFailure(spVideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &decodeSupport, sizeof(decodeSupport)));
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    static bool SupportsArrayOfTexture(const D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT& decodeSupport, VIDEO_DECODE_PROFILE_TYPE ProfileType)
    {
        return (   (   decodeSupport.DecodeTier >= D3D12_VIDEO_DECODE_TIER_2
                    || (decodeSupport.ConfigurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_REFERENCE_ONLY_ALLOCATIONS_REQUIRED) != 0)
                && (   ProfileType == VIDEO_DECODE_PROFILE_TYPE_H264
                    || ProfileType == VIDEO_DECODE_PROFILE_TYPE_H264_MVC
                    || ProfileType == VIDEO_DECODE_PROFILE_TYPE_HEVC
                    || ProfileType == VIDEO_DECODE_PROFILE_TYPE_VP9));
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::GetVideoDecoderConfigCount(ID3D12Device *pDevice12, UINT NodeIndex, const VIDEO_DECODE_DESC *pDesc, UINT *pConfigCount)
    {
        UINT configCount = 0;
        D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT decodeSupport = {};
        GetVideoDecoderSupport(pDevice12, NodeIndex, pDesc, decodeSupport);
        if (decodeSupport.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED)
        {
            configCount++;
        }
        *pConfigCount = configCount;
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    void VideoDecode::GetVideoDecoderConfig(ID3D12Device *pDevice12, UINT NodeIndex, const VIDEO_DECODE_DESC *pDesc, UINT configIndex, VIDEO_DECODE_CONFIG *pConfig, bool IsXbox)
    {
        pConfig->InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
        pConfig->ConfigDecoderSpecific = 0;

        size_t supportedConfigIndex = 0;
        D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT decodeSupport = {};
        GetVideoDecoderSupport(pDevice12, NodeIndex, pDesc, decodeSupport);
        if (decodeSupport.SupportFlags & D3D12_VIDEO_DECODE_SUPPORT_FLAG_SUPPORTED)
        {
            if (supportedConfigIndex == configIndex)     // found supported matching configIndex
            {
                // REUSE_DECODER indicates that the decoder may be re-used in the event of a resolution change
                // and also a bit depth change (and therefore a Profile change).  D3D12 only supports the former.
                // VP9 is disabled because  the resolution may change on non-key frames which is tested.

                VIDEO_DECODE_PROFILE_TYPE ProfileType = GetProfileType(pDesc->DecodeProfile);

                if (   IsXbox
                    && IsXboxReuseDecoderProfileType(ProfileType))
                {
                    pConfig->ConfigDecoderSpecific |= VIDEO_DECODE_CONFIG_SPECIFIC_REUSE_DECODER;
                }

                if (SupportsArrayOfTexture(decodeSupport, ProfileType))
                {
                    pConfig->ConfigDecoderSpecific |= VIDEO_DECODE_CONFIG_SPECIFIC_ARRAY_OF_TEXTURES;
                }

                if (decodeSupport.ConfigurationFlags & D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_HEIGHT_ALIGNMENT_MULTIPLE_32_REQUIRED)
                {
                    pConfig->ConfigDecoderSpecific |= VIDEO_DECODE_CONFIG_SPECIFIC_ALIGNMENT_HEIGHT;
                }
                return;
            }
            ++supportedConfigIndex;
        }
        ThrowFailure(E_INVALIDARG);
    }

    //----------------------------------------------------------------------------------------------------------------------------------
    _Use_decl_annotations_
    HRESULT VideoDecode::GetDecodingStatus(void* pData, UINT dataSize) noexcept
    {
        m_decodingStatus.ReadAvailableData(m_spVideoDecoder->GetForImmediateUse(), m_profileType, static_cast<BYTE*>(pData), dataSize); // throw( _com_error )

        return S_OK;
    }
};