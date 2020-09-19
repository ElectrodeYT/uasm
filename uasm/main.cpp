#include <iostream>
#include <iomanip>
#include <fstream>
#include "args.h"
#include "machine.h"
#include "assembler.h"
#include "log.h"
#include <chrono>

int main(int argc, char** argv) {
	std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
	// Decode the arguments
	Arguments::Argument args = Arguments::decodeArgV(argc, argv);
	// Figure out what machine should be compiled for
	std::string machine = Arguments::getArgument(args, 'm');
	// Get file to be compiled
	std::string path = Arguments::getArgument(args, 'i');
	// Output file
	std::string out = Arguments::getArgument(args, 'o');
	if (machine == "") { machine = "simple"; } // if -m is not passed, then use the simple machine
	if (path == "") { std::cerr << "[UASM-Main] [ERR] No Input file!\n"; exit(-1); }
	if (out == "") { LOG_ERR("UASM-Main", "No Output File!"); }
	// Read machine file
	Machine::MachineFile machinefile = Machine::readMachine(machine);
	// Return if machine file was invalid
	if (machinefile.failed) { return -1; }
	// Assemble Code
	Assembler::Assembled code = Assembler::assembleMachine(machinefile, path);
	// Return if code assembly failed
	if (code.failed) { return -1; }
	std::cout << "Code: \n";
	for (int i = 0; i < code.prog.size(); i++) {
		std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)code.prog.at(i) << " ";
	}
	std::cout << "\n\n";
	if (code.harvard) {
		std::cout << "Data: \n";
		for (int i = 0; i < code.data.size(); i++) {
			std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)code.data.at(i) << " ";
		}
	}
	// Save file
	std::fstream file(out, std::ios::out | std::ios::binary);
	file.write((const char*)code.data.data(), code.data.size());
	file.flush();
	file.close();
	std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
	std::cout << "\n\nTime: " << std::dec << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds\n";
}