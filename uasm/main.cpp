#include <iostream>
#include "args.h"
#include "machine.h"
#include "assembler.h"

int main(int argc, char** argv) {
	// Decode the arguments
	Arguments::Argument args = Arguments::decodeArgV(argc, argv);
	// Figure out what machine should be compiled for
	std::string machine = Arguments::getArgument(args, 'm');
	// Get file to be compiled
	std::string path = Arguments::getArgument(args, 'i');
	if (machine == "") { machine = "simple"; } // if -m is not passed, then use the simple machine
	if (path == "") { std::cerr << "[UASM-Main] [ERR] No Input file!\n"; exit(-1); }
	Machine::MachineFile machinefile = Machine::readMachine(machine);
	std::vector<unsigned char> code = Assembler::assembleMachine(machinefile, path);
	std::getchar();
}