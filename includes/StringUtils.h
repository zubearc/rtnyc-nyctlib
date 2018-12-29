#pragma once

#include <string>

namespace Andromeda {
	std::string inline string_to_hex(const std::string& input)
	{
		static const char* const lut = "0123456789ABCDEF";
		size_t len = input.length();

		std::string output;
		output.reserve(2 * len);
		for (size_t i = 0; i < len; ++i)
		{
			const unsigned char c = input[i];
			output.push_back(lut[c >> 4]);
			output.push_back(lut[c & 15]);
		}
		return output;
	}
}