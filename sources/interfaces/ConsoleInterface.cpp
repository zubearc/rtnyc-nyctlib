#include "interfaces/ConsoleInterface.h"

namespace nyctlib {
	ConsoleInterface::ConsoleInterface() {
	}

	void ConsoleInterface::run(int argc, char *argv[]) {
		if (argc == 1)
			return;
		
		for (int i = 0; i < argc; i++) {
			auto arg = std::string(argv[i]);
			printf("argv[%d] = '%s'\n", i, arg.c_str());
		}
	}
}