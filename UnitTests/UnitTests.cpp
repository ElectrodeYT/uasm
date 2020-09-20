#include "pch.h"
#include "CppUnitTest.h"
#include <vector>

// UASM includes
#include "../uasm/helper.h"
#include "../uasm/assembler.h"
#include "../uasm/machine.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(AssemblerTests) {
	public:
		TEST_METHOD(AssemblerHandlesEmptyMachineFile) {
			Machine::MachineFile empty_machine;
			std::vector<std::string> code = { ".uasm 1.0" };
			Assembler::Assembled gen_code = Assembler::assembleMachine(empty_machine, code);
			Assert::IsTrue(gen_code.failed);
		}
		TEST_METHOD(Chip8) {
			std::ifstream machine_file("examples/machines/chip8.mach");
			std::ifstream assembler_file("examples/source/chip8.s");
			Machine::MachineFile chip8 = Machine::readMachine(Helper::readIntoVector(&machine_file));
			Assert::IsFalse(chip8.failed);
			Assembler::Assembled code = Assembler::assembleMachine(chip8, Helper::readIntoVector(&assembler_file));
			Assert::IsFalse(code.failed);
			Assert::IsFalse(code.harvard);
			Assert::AreEqual(code.origin, 0x200);
		}
		TEST_METHOD(AssemblerHarvard) {
			// Machine file
			std::vector<std::string> machine_lines = {
				"#name: Defines Test; Not a machine!",
				"#inst: tst1, 00000001, 0",
				"#inst: tst2, 00000010, 0",
				"#rule: cpu-is-harvard: 1"
			};
			// Code
			std::vector<std::string> code_lines = {
				".uasm 1.0",
				"tst1",
				"tst2",
				".segment data",
				".db 0x0"
			};
			Machine::MachineFile defines_machine = Machine::readMachine(machine_lines);
			Assert::IsFalse(defines_machine.failed);
			Assembler::Assembled code = Assembler::assembleMachine(defines_machine, code_lines);
			std::vector<unsigned char> expected_code = { 1, 2 };
			std::vector<unsigned char> expected_data = { 0 };
			Assert::IsFalse(code.failed);
			Assert::IsTrue(code.harvard);
			Assert::AreEqual(code.origin, 0);
			// Check that the currect number of entries are there
			Assert::IsTrue(expected_code.size() == code.prog.size());
			Assert::IsTrue(expected_data.size() == code.data.size());
			// Compare code
			for (int i = 0; i < expected_code.size(); i++) {
				Assert::IsTrue(expected_code[i] == code.prog[i]);
			}
			// Compare data
			for (int i = 0; i < expected_data.size(); i++) {
				Assert::IsTrue(expected_data[i] == code.data[i]);
			}
		}
		TEST_METHOD(Defines) {
			// Machine file
			std::vector<std::string> machine_lines = {
				"#name: Defines Test; Not a machine!",
				"#define: test1: 0x00;",
				"#define: test2: 0x01;",
				"#define: test3: 0x02;",
				"#define: test4: 0x03;"
			};
			// Code
			std::vector<std::string> code_lines = {
				".uasm 1.0",
				"test1",
				"test2",
				"test3",
				"test4"
			};
			Machine::MachineFile defines_machine = Machine::readMachine(machine_lines);
			Assert::IsFalse(defines_machine.failed);
			Assembler::Assembled code = Assembler::assembleMachine(defines_machine, code_lines);
			std::vector<unsigned char> expected = { 0, 1, 2, 3 };
			Assert::IsFalse(code.failed);
			Assert::IsFalse(code.harvard);
			Assert::AreEqual(code.origin, 0);
			// Check that the currect number of entries are there
			Assert::IsTrue(expected.size() == code.prog.size());
			for (int i = 0; i < expected.size(); i++) {
				Assert::IsTrue(expected[i] == code.prog[i]);
			}
		}
	};
}
