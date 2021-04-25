#include "args.h"

using namespace Arguments;

Argument Arguments::decodeArgV(int argc, char** argv) {
	Argument args;
	for (int i = 1; i < argc; i += 2) { // first entry is the program name
		std::string s = std::string(argv[i]); // convert the c string to c++ string
		if (s.at(0) != '-') {
			// special case for this, entire string is the 'i' argument
			args.argumentName.push_back('i');
			args.argumentData.push_back(s);
		} else if (s.at(0) == '-' && i + 1 == argc) { // special case for a argument name string that is the last argument to execution
			args.argumentName.push_back(s[1]);
			args.argumentData.push_back("");
		} else if (s[0] == '-' && argv[i + 1][0] == '-') { // special case if the next argument is also a argument name
			args.argumentName.push_back(s[1]);
			args.argumentData.push_back("");
		} else { // normal case
			args.argumentName.push_back(s[1]);
			args.argumentData.push_back(std::string(argv[i + 1]));
		}
	}
	return args;
}

bool Arguments::getArgument(Argument args, char name, std::string* out) {
	for (int i = 0; i < args.argumentName.size(); i++) {
		if (args.argumentName.at(i) == name) {
			out = &args.argumentData.at(i);
			return true;
		}
	}
	return false;
}

bool Arguments::getArgument(Argument args, char name, bool* out) {
	for (int i = 0; i < args.argumentName.size(); i++) {
		if(args.argumentName.at(i) == name) {
			*out = true;
			return true;
		}
	}
	return false;
}