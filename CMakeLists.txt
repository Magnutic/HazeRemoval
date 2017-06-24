cmake_minimum_required (VERSION 2.8.12.2)

project (ImgProc)

if (MSVC)
	if (MSVC_VERSION LESS 1900)
		message (FATAL_ERROR "Visual Studio 2015 or later is required")
	endif()
else()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

# Set up compiler options for GCC & Clang
if ((${CMAKE_COMPILER_IS_GNUCXX}) OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
	# Set high warning level
	list (APPEND IP_COMPILE_OPTS
		"-Wall"
		"-Wextra"
		"-Wdeprecated"
		"-Wshadow"
		"-Wnon-virtual-dtor"
		"-Wconversion"
		"-Wsign-conversion"
	)

	# Use faster math
	list (APPEND IP_COMPILE_OPTS "-ffast-math" "-march=native")

	# Set up address sanitation and UB checker on debug builds
	set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address,undefined")
	set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")

	if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
		set  (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -fsanitize=address,undefined")
	endif()
endif()

# Set up compiler options for MSVC
if (MSVC)
	# Enable multi-core compilation and high warning level with MSVC
	list (APPEND IP_COMPILE_OPTS "/MP" "/W4")

	# Ignore warnings about not using non-standard extensions
	list (APPEND IP_COMPILE_DEFS "_SCL_SECURE_NO_WARNINGS" "_CRT_SECURE_NO_WARNINGS")
endif (MSVC)

find_package (DevIL REQUIRED)

add_executable (dehaze
	src/main.cpp
	src/filters.cpp
	src/image.cpp
	src/haze_removal.cpp
)
set_target_properties (dehaze PROPERTIES COMPILE_DEFINITIONS "${IP_COMPILE_DEFS}")
set_target_properties (dehaze PROPERTIES COMPILE_OPTIONS "${IP_COMPILE_OPTS}")

target_include_directories (dehaze PUBLIC ${IL_INCLUDE_DIR})
target_link_libraries (dehaze ${IL_LIBRARIES} ${ILU_LIBRARIES})

