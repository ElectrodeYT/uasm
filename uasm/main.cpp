#include <iostream>
#include "args.h"
#include "machine.h"

int main(int argc, char** argv) {
	// Decode the arguments
	Arguments::Argument args = Arguments::decodeArgV(argc, argv);
	// Figure out what machine should be compiled for
	std::string machine = Arguments::getArgument(args, 'm');
	if (machine == "") { machine = "simple"; } // if -m is not passed, then use the simple machine
	Machine::MachineFile machinefile = Machine::readMachine(machine);

	std::getchar();
}