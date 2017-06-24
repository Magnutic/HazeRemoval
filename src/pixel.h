#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <type_traits>

#include "util.h"

namespace ImgProc {

/** Pixel data type. Stores RGB colour as floats in the range [0.0f, 1.0f]. */
class Pixel {
public:
	Pixel(float r = 0.0f, float g = 0.0f, float b = 0.0f) : values{{ r, g, b }} {}
	Pixel(float v) : values{{v, v, v}} {}

	Pixel& operator+=(Pixel rhs);
	Pixel& operator-=(Pixel rhs);
	Pixel& operator*=(float factor);
	Pixel& operator/=(float denom);

	float getLuminance() const { return 0.2126f * r() + 0.7152f * g() + 0.0722f * b(); }

	void setLuminance(float value) {
		if (getLuminance() == 0) { r() = value; g() = value; b() = value; return; }
		auto factor = value / getLuminance();
		*this *= factor;
	}

	float getSaturation() const {
		float maxChannel = std::max(std::max(r(), g()), b());
		float minChannel = std::min(std::min(r(), g()), b());
		float value = getLuminance();

		if (value == 0.0f) { return 0; }
		return (maxChannel - minChannel) / value;
	}

	float& r() { return values[0]; }
	float& g() { return values[1]; }
	float& b() { return values[2]; }

	const float& r() const { return values[0]; }
	const float& g() const { return values[1]; }
	const float& b() const { return values[2]; }

	std::array<float, 3> values;
};

//--------------------------------------------------------------------------------------------------
// Arithmetic operators
//--------------------------------------------------------------------------------------------------

inline Pixel operator+(Pixel l, Pixel r) { return{ l.r() + r.r(), l.g() + r.g(), l.b() + r.b() }; }
inline Pixel operator-(Pixel l, Pixel r) { return{ l.r() - r.r(), l.g() - r.g(), l.b() - r.b() }; }
inline Pixel operator*(Pixel p, float f) { return{ p.r() * f, p.g() * f, p.b() * f }; }
inline Pixel operator*(float f, Pixel p) { return p * f; }
inline Pixel operator/(Pixel p, float d) { return{ p.r() / d, p.g() / d, p.b() / d }; }

inline Pixel& Pixel::operator+=(Pixel rhs) { *this = *this + rhs; return *this; }
inline Pixel& Pixel::operator-=(Pixel rhs) { *this = *this - rhs; return *this; }
inline Pixel& Pixel::operator*=(float f)   { *this = *this * f;   return *this; }
inline Pixel& Pixel::operator/=(float d)   { *this = *this / d;   return *this; }

//--------------------------------------------------------------------------------------------------

inline Pixel blend(Pixel lhs, Pixel rhs, float amount) {
	return rhs * amount + lhs * (1.0f - amount);
}

inline float linearToSrgb(float val) {
	if (val <= 0.0031308f) { return val * 12.92f; }
	return  1.055f * std::pow(val, 1.0f / 2.4f) - 0.055f;
}

inline float srgbToLinear(float val) {
	if (val <= 0.04045f) { return val / 12.92f; }
	return std::pow((val + 0.055f) / 1.055f, 2.4f);
}

inline Pixel linearToSrgb(Pixel p) {
	return { linearToSrgb(p.r()), linearToSrgb(p.g()), linearToSrgb(p.b()) };
}

inline Pixel srgbToLinear(Pixel p) {
	return { srgbToLinear(p.r()), srgbToLinear(p.g()), srgbToLinear(p.b()) };
}

// Pixel must be trivially copyable.
// (Otherwise it would invoke undefined behaviour when memsetting pixel data.)
static_assert(std::is_trivially_copyable<Pixel>::value,
	"Pixel must be trivially copyable to avoid UB.");

/** Allow printing Pixel for debugging purposes. */
inline std::ostream& operator<<(std::ostream& os, Pixel& p) {
	os << "{ r: " << p.r() << ", g: " << p.g() << ", b: " << p.b() << " }";
	return os;
}

} // namespace ImgProc

