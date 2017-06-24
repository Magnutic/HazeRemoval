#include "haze_removal.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "filters.h"

namespace ImgProc { namespace filters {

ImageGrey getDepthFromHazyImage(const ImageRgb& in, size_t kernelSize) {
	constexpr float theta[] = { 0.121779f, 0.959710f, -0.780245f };

	ImageGrey result{ in.width(), in.height() };

	// Get estimated depth using colour attenuation prior
	for (auto coord : in.getView()) {
		const Pixel& p = in[coord];
		const float depth = theta[0] + p.getLuminance() * theta[1] + p.getSaturation() * theta[2];
		result[coord] = clamp(depth, 0.0f, 1.0f);
	}

	auto tmp = result;
	auto view = tmp.getView();

	// apply square min-filter
	for (auto coord : view) {
		float min = 1.0f;
		auto subView = view.centredSubView(coord, coord_int(kernelSize), coord_int(kernelSize));

		for (auto subCoord : subView) {
			min = std::min(min, tmp[subCoord]);
		}

		result[coord] = min;
	}

	normalise(result);

	return result;
}

ImageRgb removeHaze(const ImageRgb& in, const ImageGrey& depth, float beta) {
	assert(in.width() == depth.width() && in.height() == depth.height());

	// Find background light colour A
	std::vector<Coord> coords;
	for (auto coord : depth.getView()) { coords.push_back(coord); }

	// Partially sort coords list so that coords for 0.1% farthest pixels come first
	size_t nHighest = depth.data().size() / 1000u;
	std::nth_element(coords.begin(), coords.begin() + long(nHighest), coords.end(),
		[&](auto a, auto b) { return depth[a] > depth[b]; }
	);

	Pixel A;
	for (size_t i = 0; i < nHighest; ++i) {
		auto inPixel = in[coords[i]];
		A = (inPixel.getLuminance() > A.getLuminance()) ? inPixel : A;
	}

	// Generate output image by solving image formation model for scene radiance
	ImageRgb out{ in.width(), in.height() };

	for (size_t i = 0; i < out.data().size(); ++i) {
		const Pixel& inPixel = in.data()[i];
		Pixel& outPixel = out.data()[i];
		float d = depth.data()[i];

		// Transmission map
		float t = std::max(0.1f, std::min(0.9f, std::exp(-beta * d)));

		outPixel = A + (inPixel - A) / t;
	}

	return out;
}

}} // namespace ImgProc::filters

