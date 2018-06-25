// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "GtfsFeedParser.h"

#include <stdlib.h>

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	nyctlib::GtfsFeedParser gtfsFeedParser;
#ifdef _WIN32
	gtfsFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
#else
	gtfsFeedParser.loadFile("../res/gtfs_nyct_06192018_0831PM.bin");
#endif

	system("pause");
	return 0;
}
