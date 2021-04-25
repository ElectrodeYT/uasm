#pragma once
#include <vector>
#include <string>


namespace Arguments {

	class Argument {
	public:
		std::vector<char> argumentName;
		std::vector<std::string> argumentData;
	};

	Argument decodeArgV(int argc, char** argv);
	bool getArgument(Argument args, char name, std::string* out);
	bool getArgument(Argument args, char name, bool* out);
}
