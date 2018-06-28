// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "GtfsFeedParser.h"
#include "NYCTFeedParser.h"

#include <stdlib.h>

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	nyctlib::NYCTFeedParser gtfsFeedParser;
#ifdef _WIN32
	gtfsFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");
#else

#endif
	gtfsFeedParser.dumpOut();

	system("pause");
	return 0;
}
