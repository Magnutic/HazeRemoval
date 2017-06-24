#pragma once

#include "image.h"

namespace ImgProc { namespace filters {

/** Gets estimated depth from hazy image. */
ImageGrey getDepthFromHazyImage(const ImageRgb& image, size_t kernelSize);

ImageRgb removeHaze(const ImageRgb& in, const ImageGrey& depth, float beta = 1.0f);

}} // namespace ImgProc::filters

