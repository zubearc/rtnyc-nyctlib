import os, sys, re

## CONFIG OPTIONS -- to override these options, pass arguments to the script 
## such as `python emscripten_build.py EMSCRIPTEN_COMPILER=\"/path/to/emcc\"`

# path to emscripten compiler
EMSCRIPTEN_COMPILER = 'emcc'
EXPORT_FUNCTIONS = [
	'nyctlib_init'
]
INCLUDE_DIRECTORIES = [
	"includes/",
	"/Development/libs/protobuf-3.5.1.1/src"
]
LIBRARY_DIRECTORIES = [
	"/Development/libs/protobuf-3.5.1.1/cmake/buildw/"
]
LIBRARIES = [
	"protobuf"
]
Sources = [
	"sources/WasmExports.cpp",
	"sources/WASMSubway.cpp",
	"sources/EmscriptenNYCTFeedService.cpp"
]
EMSCRIPTEN_GENERATE_WEBASSEMBLY = True
EMSCRIPTEN_OUTPUTFILE = "nyctlib.html"
CMAKE_FILE = 'CMakeLists.txt'
CXX_STD = 'c++14'
READFILE_PREFIX = ""


if ".." in sys.argv[0]:
	CMAKE_FILE = '../' + CMAKE_FILE
	READFILE_PREFIX = '../'


print sys.argv[1:]
for arg in sys.argv[1:]:
	exec(arg)

## ----------------------------------------- ##


def read_file(file_name):
	with open(file_name) as file:
		return file.readlines()

linker_flags = []

if len(EXPORT_FUNCTIONS) == 0:
	linker_flags.append("-s EXPORT_ALL")
else:
	linkeri = '-s EXPORTED_FUNCTIONS=['
	for f in EXPORT_FUNCTIONS:
		linkeri += "'_%s'," % f
	linkeri = linkeri[:-1]
	linkeri += ']'
	linker_flags.append(linkeri)

if EMSCRIPTEN_GENERATE_WEBASSEMBLY:
	linker_flags.append('-s WASM=1')

linker_flags.append('-std=' + CXX_STD)

cmakefiledata = read_file(CMAKE_FILE)
reading_sources = False
for line in cmakefiledata:
	if "#" in line:
		continue
	if 'set(NYCTLIB_SOURCES' in line:
		reading_sources = True
		continue
	if reading_sources and ')' in line:
		reading_sources = False
		break
	if reading_sources:
		Sources.append(line.strip())

if len(Sources) == 0:
	raise ValueError("No sources to build")

linker_flags_line = ""
for linker_flag in linker_flags:
	linker_flags_line += linker_flag + " "

sources_line = ""
for source in Sources:
	sources_line += READFILE_PREFIX + source + " "

includes_line = ""
for d in INCLUDE_DIRECTORIES:
	if d[0] == "/":
		includes_line += "-I" + d + " "
	else:		
		includes_line += "-I" + READFILE_PREFIX + d + " "

libdirs_line = ""
for d in LIBRARY_DIRECTORIES:
	if d[0] == "/":
		libdirs_line += "-L" + d + " "
	else:		
		libdirs_line += "-L" + READFILE_PREFIX + d + " "

if "~" in includes_line or "~" in libdirs_line:
	raise ValueError("You may not use ~ for input dirs. Use absolute path or normal relative path.")

libs_line = ""
for l in LIBRARIES:
	libs_line += "-l"+l+ " "

out = "{EMSCRIPTEN_COMPILER} {SOURCES} {INCLUDES} {LIBRARY_DIRECTORIES} {LIBRARIES} -o nyctlib.html -s EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]' -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -s EMTERPRETIFY_FILE=emf --emrun {LINKER_FLAGS}".format(
	EMSCRIPTEN_COMPILER = EMSCRIPTEN_COMPILER,
	SOURCES = sources_line,
	INCLUDES = includes_line,
	LIBRARY_DIRECTORIES = libdirs_line,
	LIBRARIES = libs_line,
	LINKER_FLAGS = linker_flags_line)
print out
os.system(out)