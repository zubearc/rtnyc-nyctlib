// NYCTLib.cpp : Defines the entry point for the application.
//

#include "NYCTLib.h"

#include "GtfsFeedParser.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	nyctlib::GtfsFeedParser gtfsFeedParser = nyctlib::GtfsFeedParser();

	system("pause");
	return 0;
}
