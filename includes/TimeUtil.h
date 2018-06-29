#pragma once

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

inline std::string time_str_from_unixtime(long long timestamp) {
	std::time_t unixtime = timestamp;
	std::ostringstream s;
	s << std::put_time(std::localtime(&unixtime), "%c %Z");
	return s.str();
}