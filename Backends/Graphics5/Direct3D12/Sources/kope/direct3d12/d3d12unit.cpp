#include "d3d12unit.h"

#include <kope/graphics5/device.h>

#include <assert.h>

static D3D12_COMPARISON_FUNC convert_compare_function(kope_g5_compare_function fun) {
	switch (fun) {
	case KOPE_G5_COMPARE_FUNCTION_NEVER:
		return D3D12_COMPARISON_FUNC_NEVER;
	case KOPE_G5_COMPARE_FUNCTION_LESS:
		return D3D12_COMPARISON_FUNC_LESS;
	case KOPE_G5_COMPARE_FUNCTION_EQUAL:
		return D3D12_COMPARISON_FUNC_EQUAL;
	case KOPE_G5_COMPARE_FUNCTION_LESS_EQUAL:
		return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case KOPE_G5_COMPARE_FUNCTION_GREATER:
		return D3D12_COMPARISON_FUNC_GREATER;
	case KOPE_G5_COMPARE_FUNCTION_NOT_EQUAL:
		return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case KOPE_G5_COMPARE_FUNCTION_GREATER_EQUAL:
		return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case KOPE_G5_COMPARE_FUNCTION_ALWAYS:
		return D3D12_COMPARISON_FUNC_ALWAYS;
	default:
		assert(false);
		return D3D12_COMPARISON_FUNC_ALWAYS;
	}
}

static D3D12_FILTER convert_filter(kope_g5_filter_mode minification, kope_g5_filter_mode magnification, kope_g5_mipmap_filter_mode mipmap) {
	switch (minification) {
	case KOPE_G5_FILTER_MODE_NEAREST:
		switch (magnification) {
		case KOPE_G5_FILTER_MODE_NEAREST:
			switch (mipmap) {
			case KOPE_G5_MIPMAP_FILTER_MODE_NEAREST:
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			case KOPE_G5_MIPMAP_FILTER_MODE_LINEAR:
				return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			}
		case KOPE_G5_FILTER_MODE_LINEAR:
			switch (mipmap) {
			case KOPE_G5_MIPMAP_FILTER_MODE_NEAREST:
				return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			case KOPE_G5_MIPMAP_FILTER_MODE_LINEAR:
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			}
		}
	case KOPE_G5_FILTER_MODE_LINEAR:
		switch (magnification) {
		case KOPE_G5_FILTER_MODE_NEAREST:
			switch (mipmap) {
			case KOPE_G5_MIPMAP_FILTER_MODE_NEAREST:
				return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			case KOPE_G5_MIPMAP_FILTER_MODE_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		case KOPE_G5_FILTER_MODE_LINEAR:
			switch (mipmap) {
			case KOPE_G5_MIPMAP_FILTER_MODE_NEAREST:
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			case KOPE_G5_MIPMAP_FILTER_MODE_LINEAR:
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
		}
	}

	assert(false);
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

#include "buffer.cpp"
#include "commandlist.cpp"
#include "descriptorset.cpp"
#include "device.cpp"
#include "pipeline.cpp"
#include "sampler.cpp"
#include "texture.cpp"
