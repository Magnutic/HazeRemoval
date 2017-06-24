#include <algorithm>
#include <iostream>
#include <sstream>

#include "filters.h"
#include "image.h"

#include "haze_removal.h"

using namespace ImgProc;

void dehaze(const std::string& filename, size_t r, float beta, bool saveIntermediates) {
	auto dotPos = std::find(filename.rbegin(), filename.rend(), '.').base();

	std::string filenameNoExt{
		filename.begin(),
		dotPos == filename.begin() ? filename.end() : --dotPos
	};

	std::cout << "Dehazing " << filename << "; radius: " << r << ", beta: " << beta << std::endl;

	ImageRgb hazyImg = loadRgbImage(filename);

	ImageGrey depth = filters::getDepthFromHazyImage(hazyImg, r);
	ImageGrey depthFiltered = filters::guidedFilter(depth, hazyImg, r, 0.00001f);
	ImageRgb J = filters::removeHaze(hazyImg, depthFiltered, beta);

	if (saveIntermediates) {
		saveGreyImage(depth, filenameNoExt + "_unfiltered_depth.jpg");
		saveGreyImage(depthFiltered, filenameNoExt + "_depth.jpg");
	}

	saveRgbImage(J, filenameNoExt + "_dehazed.jpg");
}

int main(int argn, char* argv[]) {
	if (argn < 2) {
		std::cout << "Usage: dehaze file [-r radius] [-b beta]" << std::endl;
		return 1;
	}

	std::string filename = argv[1];

	// Default values for algorithm parametres
	size_t radius = 9;
	float beta = 1.0f;

	auto handleArg = [](const std::string& str, auto& out) {
		auto tmp = out;
		std::istringstream ss{ str };
		ss >> tmp;

		if (ss.fail()) {
			std::cerr << "Invalid argument '" << str << "'." << std::endl;
			std::terminate();
		}

		out = tmp;
	};

	for (int i = 1; i < argn; ++i) {
		if (std::string{argv[i]} == "-r") {
			handleArg(argv[++i], radius);
		}
		else if (std::string{argv[i]} == "-b") {
			handleArg(argv[++i], beta);
		}
	}

	dehaze(filename, radius, beta, true);
}

