// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

namespace D3D12TranslationLayer
{
    class Resource;

    constexpr UINT MAX_OUTSTANDING_DECODER_COMPRESSED_BUFFERS = 8;

    typedef enum {
        VIDEO_DECODE_CONFIG_SPECIFIC_NONE = 0,
        VIDEO_DECODE_CONFIG_SPECIFIC_BUFFER_REMAP = 0x5B,              // set by accelerator, VC1 only
        VIDEO_DECODE_CONFIG_SPECIFIC_POST_PROCESSING_OFF = 0x03,       // set by accelerator, VC1 only.
        VIDEO_DECODE_CONFIG_SPECIFIC_SP_FRAME = 1 << 7,                // set by accelerator, VC1 only (MEDIASUBTYPE_VC1S)
        VIDEO_DECODE_CONFIG_SPECIFIC_ALIGNMENT_HEIGHT = 1 << 12,       // set by accelerator
        VIDEO_DECODE_CONFIG_SPECIFIC_DOWNSAMPLING = 1 << 13,           // set by host decoder
        VIDEO_DECODE_CONFIG_SPECIFIC_ARRAY_OF_TEXTURES = 1 << 14,      // set by accelerator
        VIDEO_DECODE_CONFIG_SPECIFIC_REUSE_DECODER = 1 << 15,          // set by accelerator - This bit means that the decoder can be re-used with resolution change and bit depth change (including profile GUID change from 8bit to 10bit and vice versa).
    } VIDEO_DECODE_CONFIG_SPECIFIC_FLAGS;

    typedef enum {
        VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_8_BIT = 0,
        VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_10_BIT = 1,
        VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_16_BIT = 2,

        VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_MAX, // Keep at end to inform array size.
    } VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX;

    typedef enum {
        VIDEO_DECODE_PROFILE_BIT_DEPTH_NONE = 0, 
        VIDEO_DECODE_PROFILE_BIT_DEPTH_8_BIT = (1 << VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_8_BIT),
        VIDEO_DECODE_PROFILE_BIT_DEPTH_10_BIT = (1 << VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_10_BIT),
        VIDEO_DECODE_PROFILE_BIT_DEPTH_16_BIT = (1 << VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_16_BIT),
    } VIDEO_DECODE_PROFILE_BIT_DEPTH;

     constexpr VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX GetIndex(VIDEO_DECODE_PROFILE_BIT_DEPTH BitDepth) { return static_cast<VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX>(BitDepth >> 1);}

    typedef enum {
        VIDEO_DECODE_BUFFER_TYPE_PICTURE_PARAMETERS,
        VIDEO_DECODE_BUFFER_TYPE_INVERSE_QUANTIZATION_MATRIX,
        VIDEO_DECODE_BUFFER_TYPE_SLICE_CONTROL,
        VIDEO_DECODE_BUFFER_TYPE_BITSTREAM,
        VIDEO_DECODE_BUFFER_TYPE_SIZEOF
    } VIDEO_DECODE_BUFFER_TYPE;

    typedef struct {
        GUID   DecodeProfile;
        UINT   Width;
        UINT   Height;
        DXGI_FORMAT DecodeFormat;
    } VIDEO_DECODE_DESC;

    typedef struct {
        USHORT ConfigDecoderSpecific;
        D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE InterlaceType;
    } VIDEO_DECODE_CONFIG;

    typedef struct {
        VIDEO_DECODE_DESC Desc;
        VIDEO_DECODE_CONFIG Config;
    } VideoDecodeCreationArgs;

    typedef struct VIDEO_DECODE_COMPRESSED_BITSTREAM
    {
        Resource* pBuffer;
        UINT64 Offset;
        UINT Size;
    } VIDEO_DECODE_COMPRESSED_BITSTREAM;

    typedef struct VIDEO_DECODE_OUTPUT_CONVERSION_ARGUMENTS
    {
        BOOL Enable;
        DXGI_COLOR_SPACE_TYPE OutputColorSpace;
        D3D12_VIDEO_SAMPLE ReferenceInfo;
        UINT ReferenceFrameCount;
    } VIDEO_DECODE_OUTPUT_CONVERSION_ARGUMENTS;

    typedef struct VIDEO_DECODE_DECRYPTION_ARGUMENTS
    {
        _Field_size_opt_(KeyInfoSize) void* pKeyInfo;
        UINT  KeyInfoSize;
        _Field_size_(IVSize) const void* pIV;
        UINT IVSize;
        _Field_size_opt_(SubSampleMappingCount) const void*pSubSampleMappingBlock;
        UINT SubSampleMappingCount; 
        UINT cBlocksStripeEncrypted;
        UINT cBlocksStripeClear;
    } VIDEO_DECODE_DECRYPTION_ARGUMENTS;

    typedef struct VIDEO_DECODE_INPUT_STREAM_ARGUMENTS
    {
        D3D12_VIDEO_DECODE_FRAME_ARGUMENT FrameArguments[D3D12_VIDEO_DECODE_MAX_ARGUMENTS];
        UINT FrameArgumentsCount;
        VIDEO_DECODE_COMPRESSED_BITSTREAM CompressedBitstream;
        VIDEO_DECODE_DECRYPTION_ARGUMENTS DecryptionArguments;
    } VIDEO_DECODE_INPUT_STREAM_ARGUMENTS;

    typedef struct VIDEO_DECODE_COMPONENT_HISTOGRAM
    {
        UINT64 Offset;
        Resource* pBuffer;
    } VIDEO_DECODE_COMPONENT_HISTOGRAM;

    const UINT VIDEO_DECODE_MAX_HISTOGRAM_COMPONENTS = 4;

    typedef struct VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS
    {
        Resource* pOutputTexture2D;
        CViewSubresourceSubset SubresourceSubset;
        VIDEO_DECODE_OUTPUT_CONVERSION_ARGUMENTS ConversionArguments;
        VIDEO_DECODE_COMPONENT_HISTOGRAM Histograms[VIDEO_DECODE_MAX_HISTOGRAM_COMPONENTS];
        
    } VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS;

    class VideoDecode : public DeviceChild
    {
    public:
        friend class ImmediateContext;
        VideoDecode(_In_ ImmediateContext *pDevice, VideoDecodeCreationArgs const& args);
        virtual ~VideoDecode() noexcept;

        static HRESULT GetVideoDecoderBufferTypeCount(_In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pBufferTypeCount) noexcept;
        static void GetVideoDecoderBufferInfo(_In_ const VIDEO_DECODE_DESC *pDesc, _In_ UINT Index, _Out_ VIDEO_DECODE_BUFFER_TYPE *pType, _Out_ UINT *pSize, bool IsXbox);
        static void GetVideoDecoderConfigCount(_In_ ID3D12Device *pDevice12, UINT NodeIndex, _In_ const VIDEO_DECODE_DESC *pDesc, _Out_ UINT *pConfigCount);
        static void GetVideoDecoderConfig(_In_ ID3D12Device *pDevice12, UINT NodeIndex, _In_ const VIDEO_DECODE_DESC *pDesc, UINT configIndex, _Out_ VIDEO_DECODE_CONFIG *pConfig, bool IsXbox);

        void DecodeFrame(_In_ const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments, _In_ const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments);
        HRESULT GetDecodingStatus(_Out_writes_bytes_(dataSize) void* pData, UINT dataSize) noexcept;

        bool IsArrayOfTexturesEnabled() const { return (m_ConfigDecoderSpecific & VIDEO_DECODE_CONFIG_SPECIFIC_ARRAY_OF_TEXTURES) == VIDEO_DECODE_CONFIG_SPECIFIC_ARRAY_OF_TEXTURES; }

    protected:

        void ManageResolutionChange(_In_ const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments);
        HRESULT GetDecodeFrameInfo(_Out_ UINT *pWidth, _Out_ UINT *pHeight, _Out_ UINT16 *pMaxDPB) noexcept;
        static VIDEO_DECODE_PROFILE_TYPE GetProfileType(_In_ REFGUID DecodeProfile) noexcept;
        static VIDEO_DECODE_PROFILE_BIT_DEPTH GetProfileBitDepth(_In_ REFGUID DecodeProfile) noexcept;
        static VIDEO_DECODE_PROFILE_BIT_DEPTH GetFormatBitDepth(DXGI_FORMAT Format) noexcept;
        GUID GetDecodeProfile(VIDEO_DECODE_PROFILE_TYPE ProfileType, VIDEO_DECODE_PROFILE_BIT_DEPTH BitDepth) noexcept;
        void LogPicParams() const;
        void ReleaseUnusedReferences();
        void UpdateCurrPic(_In_ Resource* pTexture2D, UINT subresourceIndex);
        void PrepareForDecodeFrame(_In_ const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments, _In_ const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments);
        void CachePicParams(_In_ const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments);

        void *GetPicParams() { return m_modifiablePicParams.get(); }
        template <typename T> T *GetPicParams() { return static_cast<T*>(GetPicParams());}
        void *GetPicParams() const { return m_modifiablePicParams.get(); }
        template <typename T> T *GetPicParams() const { return static_cast<T*>(GetPicParams());}
        void GetStatusReportFeedbackNumber(_Out_ UINT& statusReportFeedbackNumber, _Out_ DXVA_PicEntry& CurrPic, _Out_ UCHAR& field_pic_flag) noexcept;
        static void GetVideoDecoderSupport(_In_ ID3D12Device *pDevice12, UINT NodeIndex, _In_ const VIDEO_DECODE_DESC *pDesc, _Out_ D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT &decodeSupport);

        std::optional<GUID>                                  m_DecodeProfilePerBitDepth[VIDEO_DECODE_PROFILE_BIT_DEPTH_INDEX_MAX];
        unique_comptr<ID3D12VideoDevice>                     m_spVideoDevice;
        std::unique_ptr<VideoDecoder>                        m_spVideoDecoder;
        D3D12_VIDEO_DECODER_DESC                             m_decoderDesc = {};
        D3D12_VIDEO_DECODER_HEAP_DESC                        m_decoderHeapDesc = {};
        D3D12_VIDEO_DECODE_TIER                              m_tier = D3D12_VIDEO_DECODE_TIER_NOT_SUPPORTED;
        DXGI_FORMAT                                          m_decodeFormat;
        D3D12_VIDEO_DECODE_CONFIGURATION_FLAGS               m_configurationFlags = D3D12_VIDEO_DECODE_CONFIGURATION_FLAG_NONE;
        const VIDEO_DECODE_PROFILE_TYPE                      m_profileType;
        ReferenceDataManager                                 m_referenceDataManager {DeviceChild::m_pParent, m_profileType};
        std::shared_ptr<VideoDecoderHeap>                    m_spCurrentDecoderHeap;
        std::unique_ptr<char[]>                              m_modifiablePicParams;
        UINT                                                 m_modifiablePicParamsAllocationSize = 0;
        USHORT                                               m_ConfigDecoderSpecific = 0;

        VideoDecodeStatistics m_decodingStatus;
    };

    class BatchedVideoDecode : public BatchedDeviceChildImpl<VideoDecode>
    {
    public:
        BatchedVideoDecode(BatchedContext& Context, VideoDecodeCreationArgs const& Args)
            : BatchedDeviceChildImpl(Context, Args)
        {
        }

        void DecodeFrame(_In_ const VIDEO_DECODE_INPUT_STREAM_ARGUMENTS *pInputArguments, _In_ const VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS *pOutputArguments)
        {
            FlushBatchAndGetImmediate().DecodeFrame(pInputArguments, pOutputArguments);
        }

        HRESULT GetDecodingStatus(_Out_writes_bytes_(dataSize) void* pData, UINT dataSize) noexcept
        {
            return FlushBatchAndGetImmediate().GetDecodingStatus(pData, dataSize);
        }
    };
};
