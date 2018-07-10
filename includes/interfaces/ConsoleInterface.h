#pragma once

#include <string>

namespace nyctlib {
	class ConsoleInterface {
	public:
		ConsoleInterface();

		void run(int argc, char *argv[]);
	};
}