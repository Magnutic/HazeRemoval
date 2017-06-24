#pragma once

#include "image.h"
#include "util.h"

namespace ImgProc {

/** Image processing filters. */
namespace filters {

/** O(n) implementation of box filter. */
template <typename PixelT>
BaseImage<PixelT> boxFilter(const BaseImage<PixelT>& image, size_t r);

/** Single-channel guided filter. */
ImageGrey guidedFilter(const ImageGrey& input, const ImageRgb& guide, size_t r, float eps);

/** RGB guided filter. */
ImageRgb guidedFilter(const ImageRgb& input, const ImageRgb& guide, size_t r, float eps);

/** Normalises greyscale image such that lowest value becomes 0.0f and highest becomes 1.0f */
inline void normalise(ImageGrey& img) {
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();

	for (auto& p : img.data()) {
		min = std::min(min, p);
		max = std::max(max, p);
	}

	for (auto& p : img.data()) { p = (p - min) / (max - min); }
}

}} // namespace ImgProc::filters

#include "filters.tpp"

