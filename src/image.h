#pragma once

#include <algorithm>
#include <cassert>
#include <exception>
#include <vector>

#include "image_view.h"
#include "pixel.h"
#include "util.h"

/** Image processing library. */
namespace ImgProc {

/** Exception thrown on errors within Image. */
struct ImageError : public std::runtime_error { using std::runtime_error::runtime_error; };

/** Basic image type, can be parameterised with different Pixel types. */
template <typename PixelT>
class BaseImage {
public:
	using PixelType = PixelT;

	/** Construct uninitialised image with given dimensions. */
	explicit BaseImage(coord_int width, coord_int height)
		: m_width(width), m_height(height), m_data(size_t(width*height)) {}

	/** Construct from row-major pixel data sequence. */
	explicit BaseImage(coord_int width, coord_int height, const PixelT* data)
		: BaseImage(width, height)
	{
		std::copy(data, data + width * height, std::back_inserter(m_data));
	}

	/** Construct from row-major pixel data vector (moving pixel data, faster). */
	explicit BaseImage(coord_int width, coord_int height, std::vector<PixelType>&& data)
		: m_width(width), m_height(height)
	{
		assert(size_t(width * height) <= data.size());
		m_data = std::move(data);
	}

	/** Get image width */
	coord_int width() const { return m_width; }

	/** Get image height */
	coord_int height() const { return m_height; }

	/** Get a View covering whole Image. */
	ImageView getView() const { return getView({0, 0}, width(), height()); }

	/** Get a View covering a subset of the image. Safer than directly constructing ImageView, as it
	 * bounds the view to the image's size.
	 */
	ImageView getView(Coord offset, coord_int width, coord_int height) const {
		return ImageView{
			Coord{ std::max(0, offset.x), std::max(0, offset.y) },
			clamp(0, m_width  - offset.x, width),
			clamp(0, m_height - offset.y, height)
		};
	}

	/** Access pixel safely (coord clamped to image bounds). */
	PixelType& operator[](Coord coord) { return data()[getIndex(coord)]; }

	/** Access const pixel safely (coord clamped to image bounds). */
	const PixelType& operator[](Coord coord) const { return data()[getIndex(coord)]; }

	/** Access pixel. Caller is responsible for ensurind coord is within image bounds. */
	PixelType& getPixelUnsafe(Coord coord) { return data()[getIndexUnsafe(coord)]; }

	/** Access const pixel. Caller is responsible for ensurind coord is within image bounds. */
	const PixelType& getPixelUnsafe(Coord coord) const { return data()[getIndexUnsafe(coord)]; }

	/** Direct access to pixel data. */
	std::vector<PixelType>& data()  { return m_data; }

	/** Direct access to const pixel data. */
	const std::vector<PixelType>& data() const { return m_data; }

private:
	PixelType& getPixel(size_t index) { return m_data[index]; }
	const PixelType& getPixel(size_t index) const { return m_data[index]; }

	// Get storage index of pixel at coord. */
	size_t getIndexUnsafe(Coord coord) const {
		auto index = size_t(coord.y * m_width + coord.x);
		assert(index < m_data.size());
		return index;
	}

	size_t getIndex(Coord coord) const {
		// Clamp to image borders
		coord = { clamp(coord.x, 0, width() - 1), clamp(coord.y, 0, height() - 1) };
		return getIndexUnsafe(coord);
	}

private:
	coord_int m_width = 0, m_height = 0;

	// Internal pixel data storage
	std::vector<PixelType> m_data;
};

//--------------------------------------------------------------------------------------------------
// Arithmetic operators on Images
//--------------------------------------------------------------------------------------------------

namespace detail {

template <typename PixelT>
void checkSizes(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	if (l.width() != r.width() || l.height() != r.height()) {
		throw ImageError{ "Binary arithmetic operator on two images of different sizes." };
	}
};

// General implementation of binary operations on images
template <typename PixelT, typename BinaryOp>
BaseImage<PixelT> binaryOp(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r, BinaryOp op) {
	checkSizes(l, r);
	BaseImage<PixelT> out{ l.width(), l.height() };

	for (size_t i = 0; i < l.data().size(); ++i) {
		out.data()[i] = op(l.data()[i], r.data()[i]);
	}

	return out;
}

// General implementation of binary assignment operations on images
template <typename PixelT, typename BinaryOp>
BaseImage<PixelT>& binaryAssignmentOp(BaseImage<PixelT>& l, const BaseImage<PixelT>& r, BinaryOp op)
{
	checkSizes(l, r);

	for (size_t i = 0; i < l.data().size(); ++i) {
		l.data()[i] = op(l.data()[i], r.data()[i]);
	}

	return l;
}

static auto add = [](const auto& a, const auto& b) { return a + b; };
static auto sub = [](const auto& a, const auto& b) { return a - b; };
static auto mul = [](const auto& a, const auto& b) { return a * b; };
static auto div = [](const auto& a, const auto& b) { return a / b; };

} // namespace detail

template <typename PixelT>
BaseImage<PixelT> operator+(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryOp(l, r, detail::add);
}

template <typename PixelT>
BaseImage<PixelT> operator-(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryOp(l, r, detail::sub);
}

template <typename PixelT>
BaseImage<PixelT> operator*(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryOp(l, r, detail::mul);
}

template <typename PixelT>
BaseImage<PixelT> operator/(const BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryOp(l, r, detail::div);
}

template <typename PixelT>
BaseImage<PixelT>& operator+=(BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryAssignmentOp(l, r, detail::add);
}

template <typename PixelT>
BaseImage<PixelT> operator-=(BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryAssignmentOp(l, r, detail::sub);
}

template <typename PixelT>
BaseImage<PixelT>& operator*=(BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryAssignmentOp(l, r, detail::mul);
}

template <typename PixelT>
BaseImage<PixelT>& operator/=(BaseImage<PixelT>& l, const BaseImage<PixelT>& r) {
	return detail::binaryAssignmentOp(l, r, detail::div);
}

//--------------------------------------------------------------------------------------------------
// Arithmetic operators with reals
//--------------------------------------------------------------------------------------------------

namespace detail {

template <typename PixelT, typename BinaryOp>
BaseImage<PixelT> binaryOp(const BaseImage<PixelT>& l, float v, BinaryOp op) {
	auto out = l;
	for (auto& p : out.data()) { p = op(p, v); }
	return out;
}

template <typename PixelT, typename BinaryOp>
BaseImage<PixelT>& binaryAssignmentOp(BaseImage<PixelT>& l, float v, BinaryOp op) {
	for (auto& p : l.data()) { p = op(p, v); }
	return l;
}

} // namespace detail

template <typename PixelT>
BaseImage<PixelT> operator+(const BaseImage<PixelT>& l, float v) {
	return detail::binaryOp(l, v, detail::add);
}

template <typename PixelT>
BaseImage<PixelT> operator-(const BaseImage<PixelT>& l, float v) {
	return detail::binaryOp(l, v, detail::sub);
}

template <typename PixelT>
BaseImage<PixelT> operator*(const BaseImage<PixelT>& l, float v) {
	return detail::binaryOp(l, v, detail::mul);
}

template <typename PixelT>
BaseImage<PixelT> operator/(const BaseImage<PixelT>& l, float v) {
	return detail::binaryOp(l, v, detail::div);
}

template <typename PixelT>
BaseImage<PixelT>& operator+=(BaseImage<PixelT>& l, float v) {
	return detail::binaryAssignmentOp(l, v, detail::add);
}

template <typename PixelT>
BaseImage<PixelT>& operator-=(BaseImage<PixelT>& l, float v) {
	return detail::binaryAssignmentOp(l, v, detail::sub);
}

template <typename PixelT>
BaseImage<PixelT>& operator*=(BaseImage<PixelT>& l, float v) {
	return detail::binaryAssignmentOp(l, v, detail::mul);
}

template <typename PixelT>
BaseImage<PixelT>& operator/=(BaseImage<PixelT>& l, float v) {
	return detail::binaryAssignmentOp(l, v, detail::div);
}

//--------------------------------------------------------------------------------------------------

/** RGB Image type. */
using ImageRgb = BaseImage<Pixel>;

using ImageGrey = BaseImage<float>;

static_assert(std::is_move_constructible<ImageRgb>::value,
	"ImageRgb should be movable for good performance.");

static_assert(std::is_move_constructible<ImageGrey>::value,
	"ImageGrey should be movable for good performance.");

/** Load RGB image from file. */
ImageRgb loadRgbImage(const std::string& filename);

/** Load greyscale image from file. */
ImageGrey loadGreyImage(const std::string& filename);

/** Save RGB image to file. */
void saveRgbImage(const ImageRgb& image, const std::string& filename);

/** Save greyscale image to file. */
void saveGreyImage(const ImageGrey& image, const std::string& filename);

/** Split RGB image into three greyscale images, representing each colour channel. */
std::array<ImageGrey, 3> splitChannels(const ImageRgb& image);

/** Joing three greyscale images into one RGB image. */
ImageRgb joinChannels(const ImageGrey& r, const ImageGrey& g, const ImageGrey& b);

} // namespace ImgProc

