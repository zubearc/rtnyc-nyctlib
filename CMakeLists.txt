# CMakeList.txt : CMake project for NYCTLib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories (includes/)

set(NYCTLIB_SOURCES
	sources/GtfsFeedParser.cpp
)

# Add source to this project's executable.
add_executable (NYCTLib "NYCTLib.cpp" "NYCTLib.h")

# TODO: Add tests and install targets if needed.
