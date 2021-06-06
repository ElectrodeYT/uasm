#pragma once
#include <vector>
#include <string>


class Arguments {
	private:
	Arguments() = default;

	public:

	std::vector<char> argumentName;
	std::vector<std::string> argumentData;

	static Arguments& the() {
		static Arguments instance;
		return instance;
	}

	void decodeArgV(int argc, char** argv);

	bool getArgument(char name, std::string& out) {
		for (size_t i = 0; i < argumentName.size(); i++) {
			if (argumentName.at(i) == name) {
				out = argumentData[i];
				return true;
			}
		}
		return false;
	}

	bool checkArgumentExistence(char name) {
		for(size_t i = 0; i < argumentName.size(); i++) {
			if(argumentName[i] == name) {
				return true;
			}
		}
		return false;
	}
};
