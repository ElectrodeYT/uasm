#pragma once
#include "machine.h"

namespace Assembler {
	class Variable {
	public:
		std::string name;
		long long value;
		Variable(std::string n, long long v) : name(n), value(v) {};
	};

	class Instruction {
	public:
		Machine::Instruction machine_instruction;
		std::vector<std::string> arguments;
		long long address;
	};

	std::vector<unsigned char> assembleMachine(Machine::MachineFile machine, std::string path);
	std::string getRule(Machine::MachineFile, std::string name);
}