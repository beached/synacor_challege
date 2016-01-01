// The MIT License (MIT)
//
// Copyright (c) 2014-2015 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "vm.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "console.h"
#include <fstream>
#include "file_helper.h"

namespace {
	void print_help( ) {
		std::cout << "Debugging console\nValid commmands are:\n";
		std::cout << "saveasm [filename] -> save assembly of memory to to [filename] or sc_<time since epoch>_asm.txt if not specified\n";
		std::cout << "showasm [from_address] [to_address] -> print all memory to screen\n";
		std::cout << "go -> Resume program\n";
		std::cout << "quit -> Exit program\n";
		std::cout << "getip -> print current instruction ptr value\n";
		std::cout << "setip <address> -> set the instruction ptr value to <address>\n";
		std::cout << "getmem <address> -> print current memory value at <address>\n";
		std::cout << "setmem <address> <value> -> set the memory at <address> to <value>\n";
		std::cout << "getreg <0-7> print register <0-7>\n";
		std::cout << "setreg <0-7> <value> -> set the register <0-7> to <value>\n";
		std::cout << "getregs -> display value in all registers and intruction ptr\n";
		std::cout << "showargstack -> display the argument stack items\n";
		std::cout << "showprogstack -> display the program stack items\n";
		std::cout << "tick -> run next instruction in vm\n";
		std::cout << "getbps -> display all breakpoints\n";
		std::cout << "clearbps -> clear all breakpoints\n";
		std::cout << "setbp <address> -> set brakpoint at <address>\n";
		std::cout << "clearbp <address> -> clear brakpoint at <address>\n";
		std::cout << "savestate [filename] -> save the state of program to [filename] or sc_<time since epoch>_state.bin if not specified\n";
		std::cout << "loadtate <filename> -> load the state of program from <filename>\n";
		std::cout << "\n";
	}

	template<typename T>
	T convert( boost::string_ref orig ) {
		std::stringstream ss;
		ss << orig;
		T result;
		ss >> result;
		return result;
	}

	std::string dump_regs( virtual_machine_t & vm ) {
		std::stringstream ss;
		for( size_t n = 0; n < vm.registers.size( ); ++n ) {
			ss << "REG" << n << ": " << vm.registers[n] << "\n";
		}
		ss << "Instruction ptr: " << vm.instruction_ptr << "\n";
		return ss.str( );
	}
}

struct vm_control final {

	template<typename Tokens>
	static void show_asm( virtual_machine_t & vm, Tokens const & tokens  ) {
		uint16_t from_address = 0;
		uint16_t to_address = std::numeric_limits<uint16_t>::max( );
		if( tokens.size( ) > 1 ) {
			from_address = convert<uint16_t>( tokens[1] );
		}
		if( tokens.size( ) > 2 ) {
			to_address = convert<uint16_t>( tokens[2] );
		}

		full_dump( vm, from_address, to_address );
		std::cout << "\n\n";
	}

	static void save_asm( virtual_machine_t & vm, boost::string_ref fname ) {
		std::ofstream fout;
		fout.open( fname.data( ) );
		if( !fout ) {
			std::cerr << "Error saving memory dump to " << fname << "\n";
		} else {
			fout << full_dump_string( vm );
			fout.close( );
			std::cout << "Saved file to " << fname << "\n";
		}
	}

	static void get_ip( virtual_machine_t & vm ) {
		std::cout << "Current instruction ptr is " << vm.instruction_ptr << "\n";
	}

	template<typename Tokens>
	static void set_ip( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto new_ip = convert<uint16_t>( tokens[1] );
		assert( new_ip < vm.memory.size( ) );
		std::cout << "Setting instruction ptr to " << new_ip << "\n";
		vm.instruction_ptr = new_ip;
	}

	template<typename Tokens>
	void static get_mem( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		assert( addr < vm.memory.size( ) );
		std::cout << "Memory at address " << addr << " has a value of " << vm.memory[addr] << "\n";
	}

	template<typename Tokens>
	void static set_mem( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 3 ) {
			std::cout << "Error\n";
		}
		auto addr = convert<uint16_t>( tokens[1] );
		auto value = convert<uint16_t>( tokens[2] );
		assert( addr < vm.memory.size( ) );
		std::cout << "Setting memory at address " << addr << " has with a value of " << value << "\n";
		vm.memory[addr] = value;

	}

	template<typename Tokens>
	void static get_reg( virtual_machine_t & vm, Tokens const & tokens ) { 
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		assert( addr < vm.registers.size( ) );
		std::cout << "Register " << addr << " has a value of " << vm.registers[addr] << "\n";

	}

	template<typename Tokens>
	void static set_reg( virtual_machine_t & vm, Tokens const & tokens ) { 
		if( tokens.size( ) != 3 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		auto value = convert<uint16_t>( tokens[2] );
		assert( addr < vm.registers.size( ) );
		std::cout << "Setting register " << addr << " with a value of " << value << "\n";
		vm.registers[addr] = value;
	}

	void static tick( virtual_machine_t & vm ) { 
		vm.tick( true );
		get_ip( vm );
	}

	void static get_regs( virtual_machine_t & vm ) { 
		std::cout << "Current register values\n";
		std::cout << dump_regs( vm ) << "\n";
	}

	void static get_bps( virtual_machine_t & vm ) {
		std::cout << "Current breakpoints(" << vm.breakpoints.size( ) << ")\n";
		for( auto const & bp : vm.breakpoints ) {
			std::cout << bp << "\n";
		}
	}

	void static clear_bps( virtual_machine_t & vm ) { 
		std::cout << "Clearing " << vm.breakpoints.size( ) << " breakpoints\n";
		vm.breakpoints.clear( );
	}


	template<typename Tokens>
	void static set_bp( virtual_machine_t & vm, Tokens const & tokens ) { 
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Setting breakpoint at " << addr << "\n";
		assert( addr < vm.memory.size( ) );
		vm.breakpoints.insert( addr );
	}


	template<typename Tokens>
	void static clear_bp( virtual_machine_t & vm, Tokens const & tokens ) { 
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Clear breakpoint at " << addr << "\n";
		assert( addr < vm.memory.size( ) );
		vm.breakpoints.erase( addr );
	}

	void static save_state( virtual_machine_t & vm, boost::string_ref fname ) {
		vm.save_state( fname );
		std::cout << "State saved to file '" << fname << "'\n";
	}

	void static load_state( virtual_machine_t & vm, boost::string_ref fname ) {
		vm.load_state( fname );
		std::cout << "Loaded state from file '" << fname << "'\n";
	}

	void static show_argument_stack( virtual_machine_t & vm ) { 
		std::cout << "Current argument stack(" << vm.argument_stack.size( ) << ")\n";
		for( size_t n = 0; n < vm.argument_stack.size( ); ++n ) {
			std::cout << n << ": " << vm.argument_stack[n] << "\n";
		}
	}

	void static show_program_stack( virtual_machine_t & vm ) {
		std::cout << "Current program stack(" << vm.program_stack.size( ) << ")\n";
		for( size_t n = 0; n < vm.program_stack.size( ); ++n ) {
			std::cout << n << ": " << vm.program_stack[n] << "\n";
		}
	}
};	//struct vm_control

void console( virtual_machine_t & vm ) {
	std::cin.clear( );

	print_help( );

	std::string current_line;
	std::vector<std::string> tokens;

	std::cout << "READY\n";
	while( std::getline( std::cin, current_line ) ) {
		boost::split( tokens, current_line, boost::is_any_of( "\t " ) );
		if( tokens.size( ) == 0 ) {
			continue;
		}
		if( tokens[0] == "showasm" ) {
			vm_control::show_asm( vm, tokens );
		} else if( tokens[0] == "saveasm" ) {
			if( tokens.size( ) > 1 ) {
				vm_control::save_asm( vm, current_line.substr( 10 ) );
			} else {
				vm_control::save_asm( vm, generate_unique_file_name( "sc_", "_asm", "txt" ) );
			}
		} else if( tokens[0] == "getip" ) {
			vm_control::get_ip( vm );
		} else if( tokens[0] == "setip" ) {
			vm_control::set_ip( vm, tokens );
		} else if( tokens[0] == "getmem" ) {
			vm_control::get_mem( vm, tokens );
		} else if( tokens[0] == "setmem" ) {
			vm_control::set_mem( vm, tokens );
		} else if( tokens[0] == "getreg" ) {
			vm_control::get_reg( vm, tokens );
		} else if( tokens[0] == "setreg" ) {
			vm_control::set_reg( vm, tokens );	
		} else if( tokens[0] == "tick" ) {
			vm_control::tick( vm );
		} else if( tokens[0] == "getregs" ) {
			vm_control::get_regs( vm );
		} else if( tokens[0] == "getbps" ) {
			vm_control::get_bps( vm );
		} else if( tokens[0] == "clearbps" ) {
			vm_control::clear_bps( vm );
		} else if( tokens[0] == "setbp" ) {
			vm_control::set_bp( vm, tokens );
		} else if( tokens[0] == "clearbp" ) {
			vm_control::clear_bps( vm );
		} else if( tokens[0] == "savestate" ) {
			if( tokens.size( ) > 1 ) {
				vm_control::save_state( vm, current_line.substr( 10 ) );
			} else {
				vm_control::save_state( vm, generate_unique_file_name( "sc_", "_state", "bin" ) );
			}
		} else if( tokens[0] == "loadstate" ) {
			vm_control::load_state( vm, current_line.substr( 10 ) );
		} else if( tokens[0] == "showargstack" ) {
			vm_control::show_argument_stack( vm );
		} else if( tokens[0] == "showprogstack" ) {
			vm_control::show_program_stack( vm );
		} else if( tokens[0] == "go" ) {
			std::cout << "Resuming program\n\n";
			vm.should_break = false;
			return;
		} else if( tokens[0] == "quit" ) {
			std::cout << "Exiting Program\n\n\n";
			exit( EXIT_SUCCESS );
		} else {
			std::cout << "ERROR\n\n";
			print_help( );
		}
		std::cout << "READY\n";
	}
}
