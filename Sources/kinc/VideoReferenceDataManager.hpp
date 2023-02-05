// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <memory>
#include <vector>

#include <Windows.h>

#include <d3d12video.h>

struct unique_comptr_deleter {
	template <typename T> void operator()(T *pUC) const {
		pUC->Release();
	}
};

template <typename T, class Deleter = unique_comptr_deleter> struct unique_comptr : protected std::unique_ptr<T, Deleter> {
	static_assert(std::is_empty<Deleter>::value, "unique_comptr doesn't support stateful deleter.");
	typedef std::unique_ptr<T, Deleter> parent_t;
	using pointer = typename parent_t::pointer;

	unique_comptr() : parent_t(nullptr) {}

	explicit unique_comptr(T *p) : parent_t(p) {
		if (p) {
			p->AddRef();
		}
	}

	template <typename Del2> unique_comptr(unique_comptr<T, Del2> &&other) : parent_t(other.release()) {}

	template <typename Del2> unique_comptr &operator=(unique_comptr<T, Del2> &&other) {
		parent_t::reset(other.release());
		return *this;
	}

	unique_comptr &operator=(pointer p) {
		reset(p);
		return *this;
	}

	unique_comptr &operator=(std::nullptr_t p) {
		reset(p);
		return *this;
	}

	void reset(pointer p = pointer()) {
		if (p) {
			p->AddRef();
		}
		parent_t::reset(p);
	}

	void reset(std::nullptr_t p) {
		parent_t::reset(p);
	}

	T **operator&() {
		assert(*this == nullptr);
		return reinterpret_cast<T **>(this);
	}

	T *const *operator&() const {
		return reinterpret_cast<T *const *>(this);
	}

	using parent_t::get;
	using parent_t::release;
	using parent_t::operator->;
	using parent_t::operator*;
	using parent_t::operator bool;

private:
	unique_comptr &operator=(unique_comptr const &) = delete;
	unique_comptr(unique_comptr const &) = delete;
};

namespace D3D12TranslationLayer {
	typedef enum {
		VIDEO_DECODE_PROFILE_TYPE_NONE,
		VIDEO_DECODE_PROFILE_TYPE_VC1,
		VIDEO_DECODE_PROFILE_TYPE_MPEG2,
		VIDEO_DECODE_PROFILE_TYPE_MPEG4PT2,
		VIDEO_DECODE_PROFILE_TYPE_H264,
		VIDEO_DECODE_PROFILE_TYPE_HEVC,
		VIDEO_DECODE_PROFILE_TYPE_VP9,
		VIDEO_DECODE_PROFILE_TYPE_VP8,
		VIDEO_DECODE_PROFILE_TYPE_H264_MVC,
		VIDEO_DECODE_PROFILE_TYPE_MAX_VALID // Keep at the end to inform static asserts
	} VIDEO_DECODE_PROFILE_TYPE;

	typedef struct _DXVA_PicEntry {
		union {
			struct {
				UCHAR Index7Bits : 7;
				UCHAR AssociatedFlag : 1;
			};
			UCHAR bPicEntry;
		};
	} DXVA_PicEntry;

	constexpr UINT16 DXVA_INVALID_PICTURE_INDEX = 0xFFFF;
	constexpr UINT16 HEVC_INVALID_PICTURE_INDEX = 0x7F;
	constexpr UINT16 H264_INVALID_PICTURE_INDEX = 0x7F;
	constexpr UINT16 VPX_INVALID_PICTURE_INDEX = 0x7F;

	class VideoDecoder {
	public:
		VideoDecoder(ID3D12VideoDevice *pVideoDeviceNoRef, const D3D12_VIDEO_DECODER_DESC &desc);

		D3D12_VIDEO_DECODER_DESC GetDesc() {
			return desc;
		}

		ID3D12VideoDecoder *decoder;
		D3D12_VIDEO_DECODER_DESC desc;
	};

	class VideoDecoderHeap {
	public:
		VideoDecoderHeap(ID3D12VideoDevice *pVideoDeviceNoRef, const D3D12_VIDEO_DECODER_HEAP_DESC &desc);

		D3D12_VIDEO_DECODER_HEAP_DESC GetDesc() {
			return desc;
		}

		ID3D12VideoDecoderHeap *heap;
		D3D12_VIDEO_DECODER_HEAP_DESC desc;
	};

	struct ReferenceOnlyDesc {
		DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
		UINT64 Width = 0;
		UINT Height = 0;
	};

	struct ReferenceDataManager {
		ReferenceDataManager(VIDEO_DECODE_PROFILE_TYPE profileType);

		UINT Size() const {
			return (UINT)textures.size();
		}
		bool IsReferenceOnly() {
			return m_fReferenceOnly;
		}

		void Resize(UINT16 dbp, _In_opt_ ReferenceOnlyDesc *pReferenceOnly, bool fArrayOfTexture);

		void ResetInternalTrackingReferenceUsage();
		void ResetReferenceFramesInformation();

		template <typename T, size_t size> void MarkReferencesInUse(const T (&picEntries)[size]);
		void MarkReferenceInUse(UINT16 index);

		void ReleaseUnusedReferences();

		UINT16 StoreFutureReference(UINT16 index, _In_ std::shared_ptr<VideoDecoderHeap> &decoderHeap, ID3D12Resource *pTexture2D, UINT subresourceIndex);

		template <typename T, size_t size> void UpdateEntries(T (&picEntries)[size]);
		UINT16 UpdateEntry(UINT16 index);

		template <typename T, size_t size> void GetUpdatedEntries(T (&picEntries)[size]);
		UINT16 GetUpdatedEntry(UINT16 index);

		void TransitionReferenceOnlyOutput(_Out_ ID3D12Resource *&pOutputReferenceNoRef, _Out_ UINT &OutputSubresource);

		// D3D12 DecodeFrame Parameters.
		std::vector<ID3D12Resource *> textures;
		std::vector<UINT> texturesSubresources;
		std::vector<ID3D12VideoDecoderHeap *> decoderHeapsParameter;

	protected:
		struct ReferenceData {
			std::shared_ptr<VideoDecoderHeap> decoderHeap;
			unique_comptr<ID3D12Resource> referenceOnlyTexture; // Allocated and lifetime managed by translation layer
			ID3D12Resource *referenceTexture;                   // May point to caller allocated resource or referenceOnlyTexture
			UINT subresourceIndex;
			UINT16 originalIndex;
			bool fUsed;
		};

		void TransitionReference(_In_ ReferenceData &referenceData, D3D12_RESOURCE_STATES decodeState);
		void ResizeDataStructures(UINT size);
		UINT16 FindRemappedIndex(UINT16 originalIndex);

		std::vector<ReferenceData> referenceDatas;

		UINT16 m_invalidIndex;
		UINT16 m_currentOutputIndex = 0;
		bool m_fReferenceOnly = false;
		bool m_fArrayOfTexture = false;
	};

	//----------------------------------------------------------------------------------------------------------------------------------
	template <typename T, size_t size> inline void ReferenceDataManager::UpdateEntries(T (&picEntries)[size]) {
		for (auto &picEntry : picEntries) {
			picEntry.Index7Bits = UpdateEntry(picEntry.Index7Bits);
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	template <typename T, size_t size> inline void ReferenceDataManager::GetUpdatedEntries(T (&picEntries)[size]) {
		for (auto &picEntry : picEntries) {
			picEntry.Index7Bits = GetUpdatedEntry(picEntry.Index7Bits);
		}
	}

	//----------------------------------------------------------------------------------------------------------------------------------
	template <typename T, size_t size> inline void ReferenceDataManager::MarkReferencesInUse(const T (&picEntries)[size]) {
		for (auto &picEntry : picEntries) {
			MarkReferenceInUse(picEntry.Index7Bits);
		}
	}
};