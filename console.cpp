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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include "console.h"
#include <fstream>

namespace {
	std::string epoch( ) {
		using boost::posix_time::ptime;
		using boost::posix_time::time_from_string;
		static ptime const epoch = time_from_string( "1970-01-01 00:00:00.000" );
		ptime other = time_from_string( "2011-08-09 17:27:00.000" );

		return std::to_string( (other - epoch).total_milliseconds( ) );
	}

	void print_help( ) {
		std::cout << "Debugging console\nValid commmands are:\n";
		std::cout << "savememory -> save memory to a file(sc_<time since epoch>_dump.txt)\n";
		std::cout << "printmemory -> print all memory to screen\n";
		std::cout << "continue -> continue program\n";
		std::cout << "quit -> exit program\n";
		std::cout << "getip -> print current instruction ptr value\n";
		std::cout << "setip <address> -> set the instruction ptr value to <address>\n";
		std::cout << "getmem <address> -> print current memory value at <address>\n";
		std::cout << "setmem <address> <value> -> set the memory at <address> to <value>\n";
		std::cout << "getreg <0-7> print register <0-7>\n";
		std::cout << "setreg <0-7> <value> -> set the register <0-7> to <value>\n";
		std::cout << "getregisters -> display value in all registers and intruction ptr\n";
		std::cout << "tick -> run next instruction in vm\n";
		std::cout << "getbreakpoints -> display all breakpoints\n";
		std::cout << "clearbreakpoints -> clear all breakpoints\n";
		std::cout << "setbp <address> -> set brakpoint at <address>\n";
		std::cout << "clearbp <address> -> clear brakpoint at <address>\n";
		std::cout << "reloadvm -> reload existing vm\n";
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
}



void console( virtual_machine_t & vm ) {
	std::cin.clear( );

	print_help( );

	std::string current_line;
	std::vector<std::string> tokens;
	while( std::getline( std::cin, current_line ) ) {
		boost::split( tokens, current_line, boost::is_any_of("\t "));
		if( tokens.size( ) == 0 ) {
			continue;
		}
		if( tokens[0] == "printmemory" ) {
			full_dump( vm );
			std::cout << "\n\n";
		} else if( tokens[0] == "savememory" ) {
			std::ofstream fout;
			std::string const fname = "sc_" + epoch( ) + "_dump.txt";
			fout.open( fname.c_str( ) );
			if( !fout ) {
				std::cerr << "Error saving memory dump to " << fname << "\n";
			} else {
				fout << full_dump_string( vm );
				fout.close( );
				std::cout << "Saved file to " << fname << "\n";
			}
		} else if( tokens[0] == "continue" ) {
			std::cout << "Continuing program\n\n";
			return;
		} else if( tokens[0] == "quit" ) {
			std::cout << "Exiting Program\n\n\n";
			exit( EXIT_SUCCESS );
		} else if( tokens[0] == "getip" ) {
			std::cout << "Current instruction ptr is " << vm.instruction_ptr << "\n";
		} else if( tokens[0] == "setip" ) {
			if( tokens.size( ) != 2 ) {
				std::cout << "Error\n";
				continue;
			}
			auto new_ip = convert<uint16_t>( tokens[1] );
			assert( new_ip < vm.memory.size( ) );
			std::cout << "Setting instruction ptr to " << new_ip << "\n";
			vm.instruction_ptr = new_ip;
		} else if( tokens[0] == "getmem" ) {
			if( tokens.size( ) != 2 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			assert( addr < vm.memory.size( ) );
			std::cout << "Memory at address " << addr << " has a value of " << vm.memory[addr] << "\n"; 
		} else if( tokens[0] == "setmem" ) {
			if( tokens.size( ) != 3 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			auto value = convert<uint16_t>( tokens[2] );
			assert( addr < vm.memory.size( ) );
			std::cout << "Setting memory at address " << addr << " has with a value of " << value << "\n"; 
			vm.memory[addr] = value;
		} else if( tokens[0] == "getreg" ) {
			if( tokens.size( ) != 2 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			assert( addr < vm.registers.size( ) );
			std::cout << "Register " << addr << " has a value of " << vm.registers[addr] << "\n"; 
		} else if( tokens[0] == "setreg" ) {
			if( tokens.size( ) != 3 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			auto value = convert<uint16_t>( tokens[2] );
			assert( addr < vm.registers.size( ) );
			std::cout << "Setting register " << addr << " with a value of " << value << "\n"; 
			vm.registers[addr] = value;
		} else if( tokens[0] == "tick" ) {
			vm.tick( );
		} else if( tokens[0] == "getregisters" ) {
			std::cout << "Current register values\n";
			for( size_t n=0; n<vm.registers.size( ); ++n ) {
				std::cout << "REG" << n << ": " << vm.registers[n] << "\n";
			}
			std::cout << "Instruction ptr: " << vm.instruction_ptr << "\n";
		} else if( tokens[0] == "getbreakpoints" ) {
			std::cout << "Current breakpoints(" << vm.breakpoints.size( ) << ")\n";
			for( auto const & bp : vm.breakpoints ) {
				std::cout << bp << "\n";
			}
		} else if( tokens[0] == "clearbreakpoints" ) {
			std::cout << "Clearing " << vm.breakpoints.size( ) << " breakpoints\n";
			vm.breakpoints.clear( );
		} else if( tokens[0] == "setbp" ) {
			if( tokens.size( ) != 2 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			std::cout << "Setting breakpoint at " << addr << "\n";	
			assert( addr < vm.memory.size( ) );
			vm.breakpoints.insert( addr );
		} else if( tokens[0] == "clearbp" ) {
			if( tokens.size( ) != 2 ) {
				std::cout << "Error\n";
				continue;
			}
			auto addr = convert<uint16_t>( tokens[1] );
			std::cout << "Clear breakpoint at " << addr << "\n";	
			assert( addr < vm.memory.size( ) );
			vm.breakpoints.erase( addr );
		} else if( tokens[0] == "reloadvm" ) {
			std::cout << "Reloading vm from '" << vm.vm_file << "'\n";
			vm.load( vm.vm_file );			
		} else {
			std::cout << "ERROR\n\n";
			print_help( );
		}
	}

}
