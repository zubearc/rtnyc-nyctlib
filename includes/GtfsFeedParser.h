#pragma once

#include <string>

namespace nyctlib {
	class GtfsFeedParser {
	public:
		GtfsFeedParser();
		
		virtual bool loadFile(std::string &filename) noexcept;
		virtual bool loadBuffer(const char *buffer, int length) noexcept;

		virtual ~GtfsFeedParser() = default;
	};
}