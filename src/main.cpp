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

void printHelp(char* exec) {
	std::cout << "uasm, the univeral assembler\n";
	std::cout << "Usage: \n";
	std::cout << exec << " [-m MACHINE_FILE] [-i INPUT_SOURCE] [-o OUTPUT FILE]\n";
}

int main(int argc, char** argv) {
	if(argc == 1) {
		printHelp(argv[0]);
		return 0;
	}

	// std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();
	// Decode the arguments
	Arguments::the().decodeArgV(argc, argv);
	// Figure out what machine should be compiled for
	std::string machine_path = "";
	std::string path = "";
	std::string out = "";

	// Check if arguments were given
	if(!Arguments::the().checkArgumentExistence('m')) {
		LOG_ERR("UASM", "-m not given!"); exit(-1);
	}

	if(!Arguments::the().checkArgumentExistence('i')) {
		LOG_ERR("UASM", "-i not given!"); exit(-1);
	}

	if(!Arguments::the().checkArgumentExistence('o')) {
		LOG_ERR("UASM", "-o not given!"); exit(-1);
	}

	if(!Arguments::the().getArgument('m', machine_path) || machine_path == "") {
		LOG_ERR("UASM", "No machine input file!"); exit(-1); 
	}
	// Get file to be compiled
	if(!Arguments::the().getArgument('i', path) || path == "") {
		LOG_ERR("UASM", "No Input file!"); exit(-1);
	}
	// Output file
	if(!Arguments::the().getArgument('o', out) || out == "") {
		LOG_ERR("UASM", "No Output File!"); exit(-1);
	}

	DEBUG_MSG("UASM", "Reading quiet mode");

	// Set quiet mode
	quiet = Arguments::the().checkArgumentExistence('q');

	DEBUG_MSG("UASM", "Quiet mode: " << quiet);

	// Check if the machine path contains a extension
	if (machine_path.find('.') == std::string::npos) {
		machine_path = machine_path += ".mach"; // If it doesnt add the .mach extension to it
	}

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


	std::cout << "Code size: " << std::dec << code.prog.size() << "\n";
	std::cout << "Code: \n";
	size_t count = 0;
	for (size_t i = 0; i < code.prog.size(); i++) {
        std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (int)code.prog.at(i);
        if(i + 1 != code.prog.size()) { std::cout << ", "; }
        if(++count == 10) { std::cout << "\n"; count = 0; }
	}
	std::cout << "\n\n";

	if (code.harvard) {
		std::cout << "Data: \n";
		for (size_t i = 0; i < code.data.size(); i++) {
			std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex << (int)code.data.at(i);
            if(i + 1 != code.data.size()) { std::cout << ", "; }
            if(++count == 10) { std::cout << "\n"; count = 0; }
		}
	}
	// Save file
	std::fstream file(out, std::ios::out | std::ios::binary);
	file.write((const char*)code.data.data(), code.data.size());
	file.flush();
	file.close();
	// std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
	// std::cout << "\n\nTime: " << std::dec << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " milliseconds\n";
}