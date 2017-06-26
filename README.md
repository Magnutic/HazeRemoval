# Haze Removal

This is an Implementation of _Fast Single-Image Haze Removal Algorithm Using Color Attenuation Prior_ [1], done as a term project for an image processing class.
It also includes an implementation of _Guided Image Filtering_ [2].

Implemented in C++14, using DevIL for loading and saving images.

## License
ZLib License, see LICENSE.txt

## Building

CMake is used to generate builds.

### Linux, Makefiles

Install DevIL from your package manager, e.g. for Debian-based distros:

    $ sudo apt-get install libdevil-dev

Then create a build directory, navigate to it, and invoke CMake:

    $ cmake <haze-removal-root-dir>

Build using make:

    $ make -j4

### Windows, Visual Studio

CMake's `find_package(DevIL)` seems to have some problems on Windows. A working approach is to manually specify include directory and library files:

Download the DevIL Windows SDK from e.g. [the DevIL sourceforge page](https://sourceforge.net/projects/openil/files/DevIL%20Windows%20SDK/).
Extract to some directory, e.g. `C:\lib\`.
Create a build directory, navigate to it, then invoke CMake:

    $ cmake <haze-removal-root-dir> -DIL_INCLUDE_DIR="C:\lib\DevIL Windows SDK\include" -DIL_LIBRARIES="C:\lib\DevIL Windows SDK\lib\x86\Release\DevIL.lib" -DILU_LIBRARIES="C:\lib\DevIL Windows SDK\lib\x86\Release\ILU.lib"

Replace `C:\lib\` with wherever you extracted the DevIL SDK, replace `x86` with `x64` if you are doing a 64-bit build.
CMake will then produce a Visual Studio solution.

The DLL files provided with the SDK need to be either copied into the same directory as the resulting .exe file, or into a directory in the PATH environment variable.

## References

[1] Q. Zhu, J. Mai and L. Shao, "A Fast Single Image Haze Removal Algorithm Using Color Attenuation Prior," in IEEE Transactions on Image Processing, vol. 24, no. 11, pp. 3522-3533, Nov. 2015.
doi: 10.1109/TIP.2015.2446191
URL: <http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7128396&isnumber=7131605>

[2] K. He, J. Sun, X. Tang, "Guided Image Filtering", in European Conference on Computer Vision, 2010.
URL: <http://kaiminghe.com/publications/eccv10guidedfilter.pdf>

