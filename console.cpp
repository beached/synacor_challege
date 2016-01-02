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
#include <string>
#include "console.h"
#include <boost/algorithm/string.hpp>
#include "vm_control.h"
#include "file_helper.h"
#include "parse_action.h"
#include <tuple>

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
		std::cout << "tick [num lines]-> run next instruction in vm.  Optionally, show previous/next [num lines] lines and registers\n";
		std::cout << "getbps -> display all breakpoints\n";
		std::cout << "clearbps -> clear all breakpoints\n";
		std::cout << "setbp <address> -> set breakpoint at <address>\n";
		std::cout << "clearbp <address> -> clear breakpoint at <address>\n";
		std::cout << "getmtraps -> display all memory traps\n";
		std::cout << "clearmtraps -> clear all memory traps\n";
		std::cout << "setmtrap <address> -> set memory trap at <address>\n";
		std::cout << "clearmtrap <address> -> clear memory trap at <address>\n";
		std::cout << "savestate [filename] -> save the state of program to [filename] or sc_<time since epoch>_state.bin if not specified\n";
		std::cout << "loadtate <filename> -> load the state of program from <filename>\n";
		std::cout << "starttrace -> start a full instruction/memory modification trace\n";
		std::cout << "stoptrace -> stop the full instruction/memory modification trace\n";
		std::cout << "savetrace [filename] -> save previous trace to [filename]/[filenaem].state or sc_<time since epoch>_trace.json/sc_<time since epoch>_trace.json.state if not specified\n";
		std::cout << "\n";
	}


}


void console( virtual_machine_t & vm ) {
	parse_action_t parse_action( {
		make_action( "saveasm", false, "[filename] -> save assembly of memory to to [filename] or sc_<time since epoch>_asm.txt if not specified\n", [&]( auto tokens ) {
		if( tokens.size( ) > 1 ) {
			vm_control::save_asm( vm, tokens[0] );
		} else {
			vm_control::save_asm( vm, generate_unique_file_name( "sc_", "_asm", "txt" ) );
		}
	} ),
	make_action( "showasm", true, "[from_address][to_address]->print all memory to screen", [&]( auto tokens ) {
		vm_control::show_asm( vm, tokens );
	} )
	} );

	std::cin.clear( );

	print_help( );

	std::string current_line;
	std::string last_line;
	std::vector<std::string> tokens;

	std::cout << "READY\n";
	while( std::getline( std::cin, current_line ) ) {
		if( current_line == "r" ) {
			current_line = last_line;
		}
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
			if( tokens.size( ) > 1 ) {
				auto num_lines = convert<uint16_t>( tokens[1] );
				uint16_t  first = 0;
				auto second = std::numeric_limits<uint16_t>::max( );
				if( vm.instruction_ptr > num_lines ) {
					first = vm.instruction_ptr - num_lines;
				}
				if( (vm.memory.size( ) - 1) - vm.instruction_ptr > num_lines ) {
					second = vm.instruction_ptr + num_lines;
				}
				vm_control::show_asm( vm, std::vector<uint16_t>{ first, second } );
			}
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
		} else if( tokens[0] == "getmtraps" ) {
			vm_control::get_memory_traps( vm );
		} else if( tokens[0] == "clearmtraps" ) {
			vm_control::clear_memory_traps( vm );
		} else if( tokens[0] == "setmtrap" ) {
			vm_control::set_memory_trap( vm, tokens );
		} else if( tokens[0] == "clearmtrap" ) {
			vm_control::clear_memory_traps( vm );
		} else if( tokens[0] == "savestate" ) {
			if( tokens.size( ) > 1 ) {
				vm_control::save_state( vm, current_line.substr( 10 ) );
			} else {
				vm_control::save_state( vm, generate_unique_file_name( "sc_", "_state", "bin" ) );
			}
		} else if( tokens[0] == "starttrace" ) {
			vm_control::start_tracing( vm );
		} else if( tokens[0] == "stoptrace" ) {
			vm_control::stop_tracing( vm );
		} else if( tokens[0] == "loadstate" ) {
			vm_control::load_state( vm, current_line.substr( 10 ) );
		} else if( tokens[0] == "showargstack" ) {
			vm_control::show_argument_stack( vm );
		} else if( tokens[0] == "showprogstack" ) {
			vm_control::show_program_stack( vm );
		} else if( tokens[0] == "go" ) {
			std::cout << "Resuming program\n\n";
			vm.debugging.should_break = false;
			return;
		} else if( tokens[0] == "quit" ) {
			std::cout << "Exiting Program\n\n\n";
			exit( EXIT_SUCCESS );
		} else if( tokens[0] == "savetrace" ) {
			if( tokens.size( ) > 1 ) {
				vm_control::save_trace( vm, current_line.substr( 10 ) );
			} else {
				vm_control::save_trace( vm, generate_unique_file_name( "sc_", "_trace", "json" ) );
			}

		} else {
			std::cout << "ERROR\n\n";
			print_help( );
		}
		std::cout << "READY\n";
		last_line = current_line;
	}
}
