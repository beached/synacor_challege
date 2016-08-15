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

std::vector<uint16_t> calc_range( virtual_machine_t const & vm, std::string line ) { 
	auto num_lines = convert<uint16_t>( line );
	std::vector<uint16_t> result;
	result.resize( 2 );

	result[0] = vm.instruction_ptr > num_lines ? vm.instruction_ptr - num_lines : 0;
	result[1] = (vm.memory.size( ) - 1) - vm.instruction_ptr > num_lines ? vm.instruction_ptr + num_lines : std::numeric_limits<uint16_t>::max( );

	return result;
}

template<typename...T>


void console( virtual_machine_t & vm ) {

	parse_action_t const parse_action( { 
		make_action( 
			"saveasm", 
			false, 
			"[filename] -> save assembly of memory to to [filename] or sc_<time since epoch>_asm.txt if not specified\n", 
			[&vm]( auto tokens ) {
				if( tokens.size( ) > 1 ) {
					vm_control::save_asm( vm, tokens[0] );
				} else {
					vm_control::save_asm( vm, generate_unique_file_name( "sc_", "_asm", "txt" ) );
				} 
				return true;
			} ),
		make_action(
			"showasm",
			true,
			"[from_address][to_address] -> print all memory to screen",
			[&vm]( auto tokens ) { vm_control::show_asm( vm, tokens ); return true; } ),
		make_action(
			"getip",
			true,
			"-> print current instruction ptr value",
			[&vm]( auto ) { vm_control::get_ip( vm ); return true; } ),
		make_action(
			"setip",
			true,
			"<address> -> set the instruction ptr value to <address>",
			[&vm]( auto tokens ) { vm_control::set_ip( vm, tokens ); return true; } ),
		make_action(
			"get_mem",
			true,
			"<address> -> print current memory value at <address>",
			[&vm]( auto tokens ) { vm_control::get_mem( vm, tokens ); return true; } ),
		make_action(
			"set_mem",
			true,
			"<address> <value> -> set the memory at <address> to <value>",
			[&vm]( auto tokens ) { vm_control::set_mem( vm, tokens ); return true; } ),
		make_action(
			"tick",
			true,
			"[num lines]-> run next instruction in vm.  Optionally, show previous/next [num lines] lines and registers",
			[&vm]( auto tokens ) {
				vm_control::tick( vm );
				if( tokens.size( ) > 1 ) {
					vm_control::show_asm( vm, calc_range( vm, tokens[1] ) );
				} 
				return true;				
			} ),
		make_action(
			"getreg",
			true,
			"<0-7> print register <0-7>",
			[&vm]( auto tokens ) { vm_control::get_reg( vm, tokens ); return true; } ),
		make_action(
			"setreg",
			true,
			"<0-7> <value> -> set the register <0-7> to <value>",
			[&vm]( auto tokens ) { vm_control::set_reg( vm, tokens ); return true; } ),
		make_action(
			"getregs",
			true,
			"display value in all registers and instruction ptr",
			[&vm]( auto ) { vm_control::get_regs( vm ); return true; } ),
		make_action(
			"getbps",
			true,
			"display all breakpoints",
			[&vm]( auto ) { vm_control::get_bps( vm ); return true; } ),
		make_action(
			"clearbps",
			true,
			"clear all breakpoints",
			[&vm]( auto ) { vm_control::clear_bps( vm ); return true; } ),
		make_action(
			"setbp",
			true,
			"<address> -> set breakpoint at <address>",
			[&vm]( auto tokens ) { vm_control::set_bp( vm, tokens ); return true; } ),
		make_action(
			"clearbp",
			true,
			"<address> -> clear breakpoint at <address>",
			[&vm]( auto tokens ) { vm_control::clear_bp( vm, tokens ); return true; } ),
		make_action(
			"getmtraps",
			true,
			"display all memory traps",
			[&vm]( auto ) { vm_control::get_memory_traps( vm ); return true; } ),
		make_action(
			"clearmtraps",
			true,
			"clear all memory traps",
			[&vm]( auto ) { vm_control::clear_memory_traps( vm ); return true; } ),
		make_action(
			"setmtrap",
			true,
			"<address> -> set memory trap at <address>",
			[&vm]( auto tokens ) { vm_control::set_memory_trap( vm, tokens ); return true; } ),
		make_action(
			"clearmtrap",
			true,
			"<address> -> clear memory trap at <address>",
			[&vm]( auto tokens ) { vm_control::clear_memory_trap( vm, tokens ); return true; } ),
		make_action(
			"setitrap",
			true,
			"<INSTRUCTION> -> set a trap before the instruction is called",
			[&vm]( auto tokens ) { vm_control::set_instruction_trap( vm, tokens ); return true; } ),
		make_action(
			"savestate",
			false,
			"[filename] -> save the state of program to [filename] or sc_<time since epoch>_state.bin if not specified",
			[&vm]( auto tokens ) {
				if( tokens[0].empty( ) ) {
					vm_control::save_state( vm, generate_unique_file_name( "sc_", "_state", "bin" ) );
				} else {
					vm_control::save_state( vm, tokens[0] );					
				}
				return true;
			} ),
		make_action(
			"loadstate",
			false,
			"<filename> -> load the state of program from <filename>",
			[&vm]( auto tokens ) { vm_control::load_state( vm, tokens[0] ); return true; } ),
		make_action(
			"showargstack",
			true,
			"display the argument stack items",
			[&vm]( auto ) { vm_control::show_argument_stack( vm ); return true; } ),
		make_action(
			"showprogstack",
			true,
			"display the program stack items",
			[&vm]( auto ) { vm_control::show_program_stack( vm ); return true; } ),
		make_action(
			"starttrace",
			true,
			"start a full instruction/memory modification trace",
			[&vm]( auto ) { vm_control::start_tracing( vm ); return true; } ),			
		make_action(
			"stoptrace",
			true,
			"stop the full instruction/memory modification trace",
			[&vm]( auto ) { vm_control::stop_tracing( vm ); return true; } ),	
		make_action(
			"savetrace",
			true,
			"[filename] -> save previous trace to [filename]/[filenaem].state or sc_<time since epoch>_trace.json/sc_<time since epoch>_trace.json.state if not specified",
			[&vm]( auto tokens ) { 
				if( tokens.empty( ) ) {
					vm_control::save_trace( vm, generate_unique_file_name( "sc_", "_trace", "json" ) );
				} else {
					vm_control::save_trace( vm, tokens[0].substr( 10 ) );
				}			
				return true; 
			} ),			
		make_action(
			"go",
			true,
			"resume program",
			[&vm]( auto ) { std::cout << "resuming program\n\n"; vm.debugging.should_break = false; return false; } ),
		make_action(
			"quit",
			true,
			"exit program",
			[]( auto ) -> bool { std::cout << "exiting program\n\n"; exit( EXIT_SUCCESS ); } )			
	} );

	std::cin.clear( );
	std::string current_line;

	parse_action.help( );
	std::cout << "READY\n";
	while( std::getline( std::cin, current_line ) ) {
		if( !parse_action.parse( current_line ) ) {
			break;
		}
		std::cout << "READY\n";
	}
}
