# CMakeList.txt : CMake project for NYCTLib, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.5)

include_directories (includes/)

set(NYCTLIB_SOURCES
	# Protocol Buffers implementations
	sources/gtfs-realtime.pb.cc
	sources/nyct-subway.pb.cc
	#
	sources/gtfs/GtfsFeedParser.cpp
	sources/gtfs/NYCTFeedParser.cpp
	sources/NYCTFeedService.cpp
	sources/NYCTFeedTracker.cpp
)

set(CMAKE_CXX_STANDARD 14)
message("Using CMAKE C++ standard ${CMAKE_CXX_STANDARD}")

if (UNIX)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++14")
endif()

if (EMSCRIPTEN)
    message("Building with EMSCRIPTEN")
    message(FATAL_ERROR "Please use emscripen_build.py to build using Emscripten")
#    add_library(NYCTLib ${NYCTLIB_SOURCES})
#    set_target_properties(NYCTLib PROPERTIES LINK_FLAGS "-s WASM=1 -s EXPORT_ALL=1  -o nyctlib.html")
else()
	find_package(CURL REQUIRED)
	message("CURL install dir is ${CURL_INCLUDE_DIR}")
	# Add source to this project's executable.
	add_executable(NYCTLib ${NYCTLIB_SOURCES} "NYCTLib.cpp" "NYCTLib.h" "sources/DynamicNYCTFeedService.cpp")
	target_link_libraries(NYCTLib PRIVATE ${CURL_LIBRARIES})
	include_directories(${CURL_INCLUDE_DIRS})
endif()
# TODO: Add tests and install targets if needed.

find_package(protobuf CONFIG REQUIRED)
target_link_libraries(NYCTLib PRIVATE protobuf::libprotobuf)
include_directories(${protobuf_INCLUDE_DIR})
