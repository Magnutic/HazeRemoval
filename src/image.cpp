#include "image.h"

#include <mutex>
#include <cstring> // std::memcpy
#include <iostream>
#include <sstream>
#include <string>

// DevIL
#include <IL/il.h>
#include <IL/ilu.h> // iluErrorString

// TODO: better logging than just dumping to stdout/stderr

namespace ImgProc {

// DevIL is not thread-safe, mutex to solve this.
static std::recursive_mutex ilMutex;

static void checkIlError() {
	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	bool any = false;
	auto error = ilGetError();
	std::ostringstream ss;

	while (error != IL_NO_ERROR) {
		auto str = iluErrorString(error);
		std::cerr << "DevIL error: " << str << "\n";
		error = ilGetError();

		if (error != IL_NO_ERROR) { ss << str << "; "; }
		any = true;
	}

	if (any) { throw ImageError{ ss.str() };}
}

// Ensures DevIL has been initialised. May be called multiple times.
static void initIl() {
	static bool ilInitialised = false;

	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	if (ilInitialised) { return; }
	std::cout << "Initialising DevIL..." << std::endl;

	ilInit();
	iluInit();
	checkIlError();

	// Even though we pass the image data back in the same order as DevIL provided it,
	// the image still ends up vertically flipped unless we explicitly set the origin.
	// N.B. EXIF orientation information will still be lost, TODO: fix
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	checkIlError();

	ilEnable(IL_FILE_OVERWRITE);

	std::cout << "Initialised DevIL.\n";
	ilInitialised = true;
}

// Wrapper for DevIL image type to provide RAII.
class IlImageGuard {
public:
	IlImageGuard() : m_image{ ilGenImage() } {}
	~IlImageGuard() { ilDeleteImage(m_image); }
	operator ILuint() const { return m_image; }

private:

	ILuint m_image = 0;
};

ImageRgb loadRgbImage(const std::string& filename) {
	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	initIl();

	IlImageGuard img;
	ilBindImage(img);
	checkIlError();

	std::cout << "Loading image '" << filename << "'.\n";

	if (ilLoadImage(filename.c_str()) == IL_FALSE) {
		throw ImageError{ "Failed to load image '" + filename + "'.\n" };
	}

	checkIlError();

	auto width = ILuint(ilGetInteger(IL_IMAGE_WIDTH));
	auto height = ILuint(ilGetInteger(IL_IMAGE_HEIGHT));

	std::cout << "Image dimensions: " << width << 'x' << height << ".\n";

	// Read image data into new pixel buffer
	std::vector<Pixel> buffer;
	buffer.resize(width * height);
	ilCopyPixels(0u, 0u, 0u, width, height, 1u, IL_RGB, IL_FLOAT, &(buffer[0]));
	checkIlError();

	// Use new pixel buffer
	return ImageRgb{ coord_int(width), coord_int(height), std::move(buffer) };
}

ImageGrey loadGreyImage(const std::string& filename) {
	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	initIl();

	IlImageGuard img;
	ilBindImage(img);
	checkIlError();

	std::cout << "Loading image '" << filename << "'.\n";

	if (ilLoadImage(filename.c_str()) == IL_FALSE) {
		throw ImageError{ "Failed to load image '" + filename + "'.\n" };
	}

	checkIlError();

	auto width = ILuint(ilGetInteger(IL_IMAGE_WIDTH));
	auto height = ILuint(ilGetInteger(IL_IMAGE_HEIGHT));

	std::cout << "Image dimensions: " << width << 'x' << height << ".\n";

	// Read image data into new pixel buffer
	std::vector<float> buffer;
	buffer.resize(width * height);
	ilCopyPixels(0u, 0u, 0u, width, height, 1u, IL_LUMINANCE, IL_FLOAT, &(buffer[0]));
	checkIlError();

	// Use new pixel buffer
	return ImageGrey{ coord_int(width), coord_int(height), std::move(buffer) };
}

void saveRgbImage(const ImageRgb& image, const std::string& filename) {
	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	initIl();

	std::cout << "Saving image to '" << filename << "'.\n";

	IlImageGuard img;
	ilBindImage(img);

	ilTexImage(ILuint(image.width()), ILuint(image.height()), 1u, 3u, IL_RGB, IL_FLOAT,
		(void*)(&(image.data()[0])) // Casting away const due to lack of const-correctness in DevIL API
	);

	checkIlError();

	ilSaveImage(filename.c_str());
	checkIlError();

	std::cout << "Wrote '" << filename << "'.\n";
}

void saveGreyImage(const ImageGrey& image, const std::string& filename) {
	std::lock_guard<std::recursive_mutex> lock{ ilMutex };

	initIl();

	std::cout << "Saving image to '" << filename << "'.\n";

	IlImageGuard img;
	ilBindImage(img);

	ilTexImage(ILuint(image.width()), ILuint(image.height()), 1u, 1u, IL_LUMINANCE, IL_FLOAT,
		(void*)(&(image.data()[0])) // Casting away const due to lack of const-correctness in DevIL API
	);

	checkIlError();

	ilSaveImage(filename.c_str());
	checkIlError();

	std::cout << "Wrote '" << filename << "'.\n";
}

std::array<ImageGrey, 3> splitChannels(const ImageRgb& image) {
	std::vector<float> channels[3];

	for (Coord coord : image.getView()) {
		for (size_t i = 0; i < 3; ++i) {
			channels[i].push_back(image[coord].values[i]);
		}
	}

	return {{
		ImageGrey{ image.width(), image.height(), std::move(channels[0]) },
		ImageGrey{ image.width(), image.height(), std::move(channels[1]) },
		ImageGrey{ image.width(), image.height(), std::move(channels[2]) }
	}};
}

ImageRgb joinChannels(const ImageGrey& r, const ImageGrey& g, const ImageGrey& b) {
	assert(r.width() == g.width() && g.width() == b.width()
		&& r.height() == g.height() && g.height() == b.height()
	);

	ImageRgb out(r.width(), r.height());

	for (size_t i = 0; i < out.data().size(); ++i) {
		out.data()[i].r() = r.data()[i];
		out.data()[i].g() = g.data()[i];
		out.data()[i].b() = b.data()[i];
	}

	return out;
}

} // namespace ImgProc

