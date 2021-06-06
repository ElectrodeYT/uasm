#include "args.h"

void Arguments::decodeArgV(int argc, char** argv) {
	for (int i = 1; i < argc; i += 2) { // first entry is the program name
		std::string s = std::string(argv[i]); // convert the c string to c++ string
		if (s.at(0) != '-') {
			// special case for this, entire string is the 'i' argument
			argumentName.push_back('i');
			argumentData.push_back(s);
		} else if (s.at(0) == '-' && i + 1 == argc) { // special case for a argument name string that is the last argument to execution
			argumentName.push_back(s[1]);
			argumentData.push_back("");
		} else if (s[0] == '-' && argv[i + 1][0] == '-') { // special case if the next argument is also a argument name
			argumentName.push_back(s[1]);
			argumentData.push_back("");
		} else { // normal case
			argumentName.push_back(s[1]);
			argumentData.push_back(std::string(argv[i + 1]));
		}
	}
}
