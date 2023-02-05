// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <Windows.h>

#include <d3d12video.h>

#include <assert.h>
#include <utility>

struct VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL {
	DXGI_FORMAT Format;
	UINT ArraySlice;
};

struct VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL {
	DXGI_FORMAT Format; // TODO: verify FourCC usage when doing VP work
	UINT MipSlice;
	UINT ArraySlice;
};

struct VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL {
	DXGI_FORMAT Format;
	UINT MipSlice;
	UINT FirstArraySlice;
	UINT ArraySize;
};

namespace D3D12TranslationLayer {
	struct CBufferView {};

	class CSubresourceSubset {
	public:
		CSubresourceSubset() noexcept {}
		explicit CSubresourceSubset(UINT8 NumMips, UINT16 NumArraySlices, UINT8 NumPlanes, UINT8 FirstMip = 0, UINT16 FirstArraySlice = 0,
		                            UINT8 FirstPlane = 0) noexcept;
		explicit CSubresourceSubset(const CBufferView &);
		explicit CSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC &) noexcept;
		explicit CSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC &) noexcept;
		explicit CSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC &) noexcept;
		explicit CSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC &) noexcept;
		explicit CSubresourceSubset(const VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL &) noexcept;
		explicit CSubresourceSubset(const VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL &) noexcept;
		explicit CSubresourceSubset(const VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL &) noexcept;

		SIZE_T DoesNotOverlap(const CSubresourceSubset &) const noexcept;
		UINT Mask() const noexcept; // Only useable/used when the result will fit in 32 bits.

		UINT NumNonExtendedSubresources() const noexcept;
		UINT NumExtendedSubresources() const noexcept;

	public:
		UINT16 m_BeginArray; // Also used to store Tex3D slices.
		UINT16 m_EndArray;   // End - Begin == Array Slices
		UINT8 m_BeginMip;
		UINT8 m_EndMip; // End - Begin == Mip Levels
		UINT8 m_BeginPlane;
		UINT8 m_EndPlane;
	};

	inline void DecomposeSubresourceIdxNonExtended(UINT Subresource, UINT NumMips, _Out_ UINT &MipLevel, _Out_ UINT &ArraySlice) {
		MipLevel = Subresource % NumMips;
		ArraySlice = Subresource / NumMips;
	}

	inline void DecomposeSubresourceIdxNonExtended(UINT Subresource, UINT8 NumMips, _Out_ UINT8 &MipLevel, _Out_ UINT16 &ArraySlice) {
		MipLevel = Subresource % NumMips;
		ArraySlice = static_cast<UINT16>(Subresource / NumMips);
	}

	template <typename T, typename U, typename V>
	inline void DecomposeSubresourceIdxExtended(UINT Subresource, UINT NumMips, UINT ArraySize, _Out_ T &MipLevel, _Out_ U &ArraySlice, _Out_ V &PlaneSlice) {
		D3D12DecomposeSubresource(Subresource, NumMips, ArraySize, MipLevel, ArraySlice, PlaneSlice);
	}

	inline UINT DecomposeSubresourceIdxExtendedGetMip(UINT Subresource, UINT NumMips) {
		return Subresource % NumMips;
	}

	inline UINT D3D12CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize) {
		return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
	}

	inline UINT ComposeSubresourceIdxExtended(UINT MipLevel, UINT ArraySlice, UINT PlaneSlice, UINT NumMips, UINT ArraySize) {
		return D3D12CalcSubresource(MipLevel, ArraySlice, PlaneSlice, NumMips, ArraySize);
	}

	inline UINT ComposeSubresourceIdxArrayThenPlane(UINT NumMips, UINT PlaneCount, UINT MipLevel, UINT ArraySlice, UINT PlaneSlice) {
		return (ArraySlice * PlaneCount * NumMips) + (PlaneSlice * NumMips) + MipLevel;
	}

	inline UINT ConvertSubresourceIndexAddPlane(UINT Subresource, UINT NumSubresourcesPerPlane, UINT PlaneSlice) {
		assert(Subresource < NumSubresourcesPerPlane || PlaneSlice == 0);
		return (Subresource + NumSubresourcesPerPlane * PlaneSlice);
	}

	inline UINT ConvertSubresourceIndexRemovePlane(UINT Subresource, UINT NumSubresourcesPerPlane) {
		return (Subresource % NumSubresourcesPerPlane);
	}

	inline UINT GetPlaneIdxFromSubresourceIdx(UINT Subresource, UINT NumSubresourcesPerPlane) {
		return (Subresource / NumSubresourcesPerPlane);
	}

	class CViewSubresourceSubset : public CSubresourceSubset {
	public:
		enum DepthStencilMode { ReadOnly, WriteOnly, ReadOrWrite };

	public:
		CViewSubresourceSubset() {}
		explicit CViewSubresourceSubset(CSubresourceSubset const &Subresources, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		explicit CViewSubresourceSubset(const CBufferView &);
		CViewSubresourceSubset(const D3D12_SHADER_RESOURCE_VIEW_DESC &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		CViewSubresourceSubset(const D3D12_UNORDERED_ACCESS_VIEW_DESC &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		CViewSubresourceSubset(const D3D12_RENDER_TARGET_VIEW_DESC &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		CViewSubresourceSubset(const D3D12_DEPTH_STENCIL_VIEW_DESC &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount,
		                       DepthStencilMode DSMode = ReadOrWrite);
		CViewSubresourceSubset(const VIDEO_DECODER_OUTPUT_VIEW_DESC_INTERNAL &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		CViewSubresourceSubset(const VIDEO_PROCESSOR_INPUT_VIEW_DESC_INTERNAL &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);
		CViewSubresourceSubset(const VIDEO_PROCESSOR_OUTPUT_VIEW_DESC_INTERNAL &Desc, UINT8 MipLevels, UINT16 ArraySize, UINT8 PlaneCount);

		template <typename T> static CViewSubresourceSubset FromView(const T *pView);

	public:
		class CViewSubresourceIterator;

	public:
		CViewSubresourceIterator begin() const;
		CViewSubresourceIterator end() const;
		bool IsWholeResource() const;
		bool IsEmpty() const;
		UINT ArraySize() const;

		UINT MinSubresource() const;
		UINT MaxSubresource() const;

	private:
		void Reduce();

	protected:
		UINT8 m_MipLevels;
		UINT16 m_ArraySlices;
		UINT8 m_PlaneCount;
	};

	// This iterator iterates over contiguous ranges of subresources within a subresource subset. eg:
	//
	// // For each contiguous subresource range.
	// for( CViewSubresourceSubset::CViewSubresourceIterator it = ViewSubset.begin(); it != ViewSubset.end(); ++it )
	// {
	//      // StartSubresource and EndSubresource members of the iterator describe the contiguous range.
	//      for( UINT SubresourceIndex = it.StartSubresource(); SubresourceIndex < it.EndSubresource(); SubresourceIndex++ )
	//      {
	//          // Action for each subresource within the current range.
	//      }
	//  }
	//
	class CViewSubresourceSubset::CViewSubresourceIterator {
	public:
		CViewSubresourceIterator(CViewSubresourceSubset const &SubresourceSet, UINT16 ArraySlice, UINT8 PlaneCount);
		CViewSubresourceIterator &operator++();
		CViewSubresourceIterator &operator--();

		bool operator==(CViewSubresourceIterator const &other) const;
		bool operator!=(CViewSubresourceIterator const &other) const;

		UINT StartSubresource() const;
		UINT EndSubresource() const;
		std::pair<UINT, UINT> operator*() const;

	private:
		CViewSubresourceSubset const &m_Subresources;
		UINT16 m_CurrentArraySlice;
		UINT8 m_CurrentPlaneSlice;
	};

	template <bool Supported> struct ConvertToDescV1Support { static const bool supported = Supported; };

	typedef ConvertToDescV1Support<false> ConvertToDescV1NotSupported;
	typedef ConvertToDescV1Support<true> ConvertToDescV1Supported;

	template <typename TDescV1> struct DescToViewDimension : ConvertToDescV1NotSupported {
		static const UINT dimensionTexture2D = 0;
		static const UINT dimensionTexture2DArray = 0;
	};

	template <typename T> inline bool IsPow2(T num) {
		static_assert(static_cast<T>(-1) > 0, "Signed type passed to IsPow2");
		return !(num & (num - 1));
	}
};
