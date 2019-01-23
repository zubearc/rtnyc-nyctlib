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
	
	sources/subway/NYCTFeedTracker.cpp
	sources/busses/NYCBusTracker.cpp
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
	find_package(ZLIB REQUIRED)
	find_package(OpenSSL REQUIRED)
	find_package(flatbuffers REQUIRED)

	message("CURL install dir is ${CURL_INCLUDE_DIR}")

	set(NYCTLIB_EXECUTABLE_SOURCES
		sources/ReplayFeedService.cpp
		sources/DynamicNYCTFeedService.cpp
		sources/DynamicBusFeedService.cpp
		sources/interfaces/ConsoleInterface.cpp
		sources/json11.cpp
	)

	if (NOT NO_INTERFACE_BUILD)
		set(NYCTLIB_EXECUTABLE_SOURCES ${NYCTLIB_EXECUTABLE_SOURCES} "sources/interfaces/WSInterface.cpp")
		set(NYCTLIB_EXECUTABLE_SOURCES ${NYCTLIB_EXECUTABLE_SOURCES} "sources/interfaces/NYCTSubwayInterface.cpp")
		set(NYCTLIB_EXECUTABLE_SOURCES ${NYCTLIB_EXECUTABLE_SOURCES} "sources/interfaces/NYCBusInterface.cpp")
		set(NYCTLIB_EXECUTABLE_SOURCES ${NYCTLIB_EXECUTABLE_SOURCES} "sources/interfaces/SessInterface.cpp")
	endif()

	# Add source to this project's executable.
	add_executable(NYCTLib ${NYCTLIB_SOURCES} "NYCTLib.cpp" "NYCTLib.h"	${NYCTLIB_EXECUTABLE_SOURCES})
	target_link_libraries(NYCTLib PRIVATE ${CURL_LIBRARIES})
	include_directories(${CURL_INCLUDE_DIRS})
	if (NOT NO_INTERFACE_BUILD)
		find_library(uv REQUIRED)
		find_library(uwebsockets REQUIRED)

		# monolithic dependencies to get basic websockets ...
		target_link_libraries(NYCTLib PRIVATE ZLIB::ZLIB)
		target_link_libraries(NYCTLib PRIVATE OpenSSL::SSL OpenSSL::Crypto)
		target_link_libraries(NYCTLib PRIVATE ${uv_LIBRARIES})
		target_link_libraries(NYCTLib PRIVATE ${uwebsockets_LIBRARIES})
		target_link_libraries(NYCTLib PRIVATE ${OPENSSL_LIBRARIES})

		message("ZLIB install lib dir is ${ZLIB_LIBRARIES}")
		message("OPENSSL intall lib dir is ${OPENSSL_LIBRARIES}")
	else()
		add_definitions("-DNO_INTERFACES")
	endif()
endif()
# TODO: Add tests and install targets if needed.

find_package(protobuf CONFIG REQUIRED)
target_link_libraries(NYCTLib PRIVATE protobuf::libprotobuf)
include_directories(${protobuf_INCLUDE_DIR})
include_directories(${uwebsockets_INCLUDE_DIR})
include_directories(${flatbuffers_INCLUDE_DIR})

# below is debug code to print all CMake variables
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
