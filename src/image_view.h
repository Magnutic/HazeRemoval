#pragma once

#include "pixel.h"
#include "util.h"

namespace ImgProc {

/** View over (a rectangular subset) of an Image. */
class ImageView {
public:
	class iterator;

	explicit ImageView(Coord offset, coord_int width, coord_int height)
		: m_offset{std::max(0, offset.x), std::max(0, offset.y)}, m_width(width), m_height(height)
	{}

	/** Generate a new ImageView as a subset of this view. */
	ImageView subView(Coord offset, coord_int width, coord_int height) const {
		return ImageView{m_offset + offset, width, height };
	}

	/** Generate a new ImageView as a subset of this view, centred around given coordinate. */
	ImageView centredSubView(Coord centre, coord_int width, coord_int height) const {
		return subView(centre - Coord{ width/2, height/2 }, width, height);
	}

	iterator begin() const;
	iterator end() const;

	/** Get view's offset from Image origin. */
	Coord offset() const { return m_offset; }

	coord_int width() const { return m_width; }
	coord_int height() const { return m_height; }

	friend bool operator==(const ImageView& lhs, const ImageView& rhs) {
			return lhs.m_offset == rhs.m_offset
			&& lhs.m_width  == rhs.m_width
			&& lhs.m_height == rhs.m_height;
	}

	friend bool operator!=(const ImageView& lhs, const ImageView& rhs) {
		return !(lhs == rhs);
	}

private:
	Coord m_offset;
	coord_int m_width = 0, m_height = 0;
};

//--------------------------------------------------------------------------------------------------

/** Iterator over Coords in a ImageView. */
class ImageView::iterator {
public:
	iterator(const ImageView& view, Coord coord)
		: m_view(&view), m_coord(coord) {}

	Coord operator*() { return m_view->offset() + m_coord; }

	iterator& operator++() {
		// Advance to next coord in view
		auto newX = m_coord.x + 1;
		m_coord.x = newX % m_view->width();
		m_coord.y += newX / m_view->width();
		return *this;
	}

	iterator operator++(int) { auto tmp = *this; ++(*this); return tmp; }

	friend bool operator==(iterator lhs, iterator rhs) { return lhs.m_coord == rhs.m_coord; }
	friend bool operator!=(iterator lhs, iterator rhs) { return !(lhs == rhs); }

private:
	const ImageView* m_view;
	Coord m_coord;
};

//--------------------------------------------------------------------------------------------------

inline ImageView::iterator ImageView::begin() const {
	return iterator{ *this, Coord{} };
}

inline ImageView::iterator ImageView::end() const {
	return iterator{ *this, Coord{ 0, height() } };
}

} // namespace ImgProc


