#include "pch.h"
#include "CppUnitTest.h"
#include <vector>

// UASM includes
#include "../uasm/assembler.h"
#include "../uasm/machine.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(AssemblerTests) {
	public:
		TEST_METHOD(AssemblerHandlesEmptyMachineFile) {
			Machine::MachineFile empty_machine;
			Assembler::Assembled code = Assembler::assembleMachine(empty_machine, "examples/source/chip8.s");
			Assert::IsTrue(code.failed);
		}
		TEST_METHOD(Chip8) {
			Machine::MachineFile chip8 = Machine::readMachine("examples/machines/chip8.mach");
			Assert::IsFalse(chip8.failed);
			Assembler::Assembled code = Assembler::assembleMachine(chip8, "examples/source/chip8.s");
			Assert::IsFalse(code.failed);
			Assert::IsFalse(code.harvard);
			Assert::AreEqual(code.origin, 0x200);
		}
		TEST_METHOD(Defines) {
			Machine::MachineFile defines_machine = Machine::readMachine("examples/machines/defines_test.mach");
			Assert::IsFalse(defines_machine.failed);
			Assembler::Assembled code = Assembler::assembleMachine(defines_machine, "examples/source/defines_test.s");
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
