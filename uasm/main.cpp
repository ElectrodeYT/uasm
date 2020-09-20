#include <iostream>
#include <fstream>
#include <iomanip>
#include <fstream>
#include "args.h"
#include "machine.h"
#include "assembler.h"
#include "log.h"
#include "helper.h"
#include <chrono>

int main(int argc, char** argv) {
	std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
	// Decode the arguments
	Arguments::Argument args = Arguments::decodeArgV(argc, argv);
	// Figure out what machine should be compiled for
	std::string machine_path = Arguments::getArgument(args, 'm');
	// Get file to be compiled
	std::string path = Arguments::getArgument(args, 'i');
	// Output file
	std::string out = Arguments::getArgument(args, 'o');
	// Check things
	if (machine_path == "") { LOG_ERR("UASM", "No machine input file!"); exit(-1); }
	// Check if the machine path contains a extension
	if (machine_path.find('.') == std::string::npos) {
		machine_path = machine_path += ".mach"; // If it doesnt add the .mach extension to it
	}
	if (path == "") { LOG_ERR("UASM", "No Input file!"); exit(-1); }
	if (out == "") { LOG_ERR("UASM", "No Output File!"); exit(-1); }

	// Open files
	std::ifstream machine(machine_path);
	if (!machine.is_open()) { LOG_ERR("UASM", "Machine file could not be opened!"); exit(-1); }
	std::ifstream assembler(path);
	if (!assembler.is_open()) { LOG_ERR("UASM", "Assembler file could not be opened!"); exit(-1); }

	// Read files
	std::vector<std::string> machine_lines = Helper::readIntoVector(&machine);
	std::vector<std::string> assembler_lines = Helper::readIntoVector(&assembler);
	// Read machine file
	Machine::MachineFile machinefile = Machine::readMachine(machine_lines);
	// Return if machine file was invalid
	if (machinefile.failed) { return -1; }
	// Assemble Code
	Assembler::Assembled code = Assembler::assembleMachine(machinefile, assembler_lines);
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