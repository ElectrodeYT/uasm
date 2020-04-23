#include "args.h"

using namespace Arguments;

Argument Arguments::decodeArgV(int argc, char** argv) {
	Argument args = Argument();
	for (int i = 1; i < argc; i += 2) { // first entry is the program name
		std::string s = std::string(argv[i]); // convert the c string to c++ string
		if (s.at(0) != '-') {
			// special case for this, entire string is the 'i' argument
			args.argumentName.push_back('i');
			args.argumentData.push_back(s);
		} else if (s.at(0) == '-' && i + 1 == argc) { // special case for a argument name string that is the last argument to execution
			args.argumentName.push_back(s.at(1));
			args.argumentData.push_back("");
		} else { // normal case
			args.argumentName.push_back(s.at(1));
			args.argumentData.push_back(std::string(argv[i + 1]));
		}
	}
	return args;
}