#include "filters.h"

namespace ImgProc { namespace filters {

// Set of intermediate results of guided filter that can be reused for filtering different images
// with the same guide image.
class GuidedFilterValues {
public:
	GuidedFilterValues(const ImageRgb& guide, size_t r, float eps)
		: radius(r * 2 + 1)
		, I(splitChannels(guide))
		, mean_I_r(boxFilter(I[0], radius))
		, mean_I_g(boxFilter(I[1], radius))
		, mean_I_b(boxFilter(I[2], radius))

		, var_I_rr((boxFilter(I[0] * I[0], radius) - mean_I_r * mean_I_r) + eps)
		, var_I_rg( boxFilter(I[0] * I[1], radius) - mean_I_r * mean_I_g)
		, var_I_rb( boxFilter(I[0] * I[2], radius) - mean_I_r * mean_I_b)
		, var_I_gg((boxFilter(I[1] * I[1], radius) - mean_I_g * mean_I_g) + eps)
		, var_I_gb( boxFilter(I[1] * I[2], radius) - mean_I_g * mean_I_b)
		, var_I_bb((boxFilter(I[2] * I[2], radius) - mean_I_b * mean_I_b) + eps)

		, invrr(var_I_gg * var_I_bb - var_I_gb * var_I_gb)
		, invrg(var_I_gb * var_I_rb - var_I_rg * var_I_bb)
		, invrb(var_I_rg * var_I_gb - var_I_gg * var_I_rb)
		, invgg(var_I_rr * var_I_bb - var_I_rb * var_I_rb)
		, invgb(var_I_rb * var_I_rg - var_I_rr * var_I_gb)
		, invbb(var_I_rr * var_I_gg - var_I_rg * var_I_rg)
	{
		const auto covDet = invrr * var_I_rr + invrg * var_I_rg + invrb * var_I_rb;

		invrr /= covDet;
		invrg /= covDet;
		invrb /= covDet;
		invgg /= covDet;
		invgb /= covDet;
		invbb /= covDet;
	}

	size_t radius;
	std::array<ImageGrey, 3> I;
	ImageGrey mean_I_r, mean_I_g, mean_I_b;
	ImageGrey var_I_rr, var_I_rg, var_I_rb, var_I_gg, var_I_gb, var_I_bb;
	ImageGrey invrr, invrg, invrb, invgg, invgb, invbb;
};

// Filter one colour channel using previously calculated GuidedFilterValues
static ImageGrey guidedFilterChannel(const ImageGrey& input, const GuidedFilterValues& v) {
	const auto mean_p = boxFilter(input, v.radius);

	const auto mean_Ip_r = boxFilter(v.I[0] * input, v.radius);
	const auto mean_Ip_g = boxFilter(v.I[1] * input, v.radius);
	const auto mean_Ip_b = boxFilter(v.I[2] * input, v.radius);

	const auto cov_Ip_r = mean_Ip_r - v.mean_I_r * mean_p;
	const auto cov_Ip_g = mean_Ip_g - v.mean_I_g * mean_p;
	const auto cov_Ip_b = mean_Ip_b - v.mean_I_b * mean_p;

	const auto a_r = v.invrr * cov_Ip_r + v.invrg * cov_Ip_g + v.invrb * cov_Ip_b;
	const auto a_g = v.invrg * cov_Ip_r + v.invgg * cov_Ip_g + v.invgb * cov_Ip_b;
	const auto a_b = v.invrb * cov_Ip_r + v.invgb * cov_Ip_g + v.invbb * cov_Ip_b;

	const auto b = mean_p - a_r * v.mean_I_r - a_g * v.mean_I_g - a_b * v.mean_I_b;

	return boxFilter(a_r, v.radius) * v.I[0]
		 + boxFilter(a_g, v.radius) * v.I[1]
		 + boxFilter(a_b, v.radius) * v.I[2]
		 + boxFilter(b,   v.radius);
}

// Filter a greyscale image
ImageGrey guidedFilter(const ImageGrey& input, const ImageRgb& guide, size_t r, float eps) {
	GuidedFilterValues v{ guide, r, eps };
	return guidedFilterChannel(input, v);
}

// Filter a colour image
ImageRgb guidedFilter(const ImageRgb& input, const ImageRgb& guide, size_t r, float eps) {
	// Calculate reusable values first
	GuidedFilterValues v{ guide, r, eps };

	auto channels = splitChannels(input);

	// Then filter per channel
	for (size_t i = 0; i < channels.size(); ++i) {
		channels[i] = guidedFilterChannel(channels[i], v);
	}

	return joinChannels(channels[0], channels[1], channels[2]);
}

}} // namespace ImgProc::filters

