#include <vector>

namespace ImgProc { namespace filters {

// Horizontal or vertical pass of box filter.
// Templated on direction, allowing compiler to optimise out branches based on direction.
template <bool vertical, typename PixelT>
void boxFilterPass(BaseImage<PixelT>& image, coord_int windowSize) {
	const auto halfWindowSize = windowSize / 2;
	const auto outerSize = vertical ? coord_int(image.width())  : coord_int(image.height());
	const auto innerSize = vertical ? coord_int(image.height()) : coord_int(image.width());

	std::vector<PixelT> rowOrColumn;
	rowOrColumn.resize(size_t(innerSize));

	// For each row / column
	for (coord_int i = 0; i < outerSize; ++i) {
		auto accum = PixelT{};
		int weight = 0;

		// Slide window over column / row, calculating accumulated value in window  by subtracting
		// element that the window just left behind and adding element that the window just passed
		// over - two arithmetic operations per iteration. Also track 'weight', number of elements
		// accumulated, so mean can be found by dividing the accumulation by weight.

		for (coord_int o = 0; o < innerSize + halfWindowSize; ++o) {
			if (o < windowSize) { ++weight; }
			else {
				// Remove value that left window
				auto oldCoord = vertical ? Coord{i, o - windowSize} : Coord{o - windowSize, i};
				accum -= image[oldCoord];
			}

			if (o < innerSize) {
				// Add value that entered window
				auto newCoord = vertical ? Coord{i, o} : Coord{o, i};
				accum += image[newCoord];
			}
			else { --weight; }

			// Set output to mean value of window.
			if (o >= halfWindowSize) {
				rowOrColumn[size_t(o - halfWindowSize)] = accum / float(weight);
			}
		}
		
		// Copy row / column values to output image.
		for (coord_int o = 0; o < innerSize; ++o) {
			const auto c = vertical ? Coord{i, o} : Coord{o, i};
			image[c] = rowOrColumn[size_t(o)];
		}
	}
}

template <typename PixelT>
BaseImage<PixelT> boxFilter(const BaseImage<PixelT>& image, size_t r) {
	BaseImage<PixelT> out{ image  };

	const auto radius = coord_int(r);

	boxFilterPass<false>(out, radius); // horizontal pass
	boxFilterPass<true>(out, radius);  // vertical pass

	return out;
}

}} // namespace ImgProc::filters

