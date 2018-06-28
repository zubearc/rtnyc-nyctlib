# CMakeList.txt : CMake project for NYCTLib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

include_directories (includes/)

set(NYCTLIB_SOURCES
	# Protocol Buffers implementations
	sources/gtfs-realtime.pb.cc
	sources/nyct-subway.pb.cc
	#
	sources/GtfsFeedParser.cpp
	sources/NYCTFeedParser.cpp
)

if (EMSCRIPTEN)
	# todo: special code for emscripten build
else()
# Add source to this project's executable.
	add_executable (NYCTLib ${NYCTLIB_SOURCES} "NYCTLib.cpp" "NYCTLib.h")
endif()
# TODO: Add tests and install targets if needed.

find_package(protobuf CONFIG REQUIRED)
target_link_libraries(NYCTLib protobuf::libprotobuf)
include_directories(${protobuf_INCLUDE_DIR})