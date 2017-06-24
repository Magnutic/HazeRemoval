#pragma once

#include <algorithm>
#include <cstdint>
#include <ostream>

namespace ImgProc {

/** Clamp a value between two extremes. */
template <typename T>
constexpr T clamp(T x, T min, T max) { return std::min(std::max(x, min), max); }

//--------------------------------------------------------------------------------------------------

/** Integer type used to represent pixel coordinates. */
using coord_int = int32_t;

/** 2D pixel coordinate type. */
struct Coord {
	coord_int x = 0, y = 0;

	// Arithmetic operators
	friend Coord operator+(Coord lhs, Coord rhs) { return{ lhs.x + rhs.x, lhs.y + rhs.y }; }
	friend Coord operator-(Coord lhs, Coord rhs) { return{ lhs.x - rhs.x, lhs.y - rhs.y }; }

	Coord& operator+=(Coord rhs) { *this = *this + rhs; return *this; }
	Coord& operator-=(Coord rhs) { *this = *this - rhs; return *this; }

	// Comparison operators
	friend bool operator==(Coord lhs, Coord rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
	friend bool operator!=(Coord lhs, Coord rhs) { return !(lhs == rhs); }
};

/** Allow printing Coord to output streams for debugging. */
template <typename Char, typename Traits>
std::basic_ostream<Char, Traits>& operator<<(std::basic_ostream<Char, Traits>& os, Coord coord) {
	return os << '(' << coord.x << ", " << coord.y << ')';
}

} // namespace ImgProc

