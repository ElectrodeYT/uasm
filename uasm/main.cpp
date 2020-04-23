#include <iostream>
#include "args.h"

int main(int argc, char** argv) {
	// Decode the arguments
	Arguments::Argument args = Arguments::decodeArgV(argc, argv);

}