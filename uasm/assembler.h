#pragma once
#include "machine.h"

namespace Assembler {
	class Variable {
	public:
		std::string name;
		long long value;
	};

	std::vector<unsigned char> assembleMachine(Machine::MachineFile machine, std::string path);
	std::string getRule(Machine::MachineFile, std::string name);
}