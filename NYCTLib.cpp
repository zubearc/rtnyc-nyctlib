// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "GtfsFeedParser.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	nyctlib::GtfsFeedParser gtfsFeedParser;
	gtfsFeedParser.loadFile("H:/Users/Extreme/Development/Projects/NYCT/DataArchives/gtfs_nyct_06192018_0831PM.bin");

	system("pause");
	return 0;
}
