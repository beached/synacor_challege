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

#pragma once

#include <boost/utility/string_ref.hpp>
#include <limits>
#include <cstdint>
#include "vm.h"

struct vm_control final {
	vm_control( ) = delete;

	template<typename Tokens>
	static void show_asm( virtual_machine_t & vm, Tokens const & tokens ) {
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
	static void get_mem( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		assert( addr < vm.memory.size( ) );
		std::cout << "Memory at address " << addr << " has a value of " << vm.memory[addr] << "\n";
	}

	template<typename Tokens>
	static void set_mem( virtual_machine_t & vm, Tokens const & tokens ) {
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
	static void get_reg( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		assert( addr < vm.registers.size( ) );
		std::cout << "Register " << addr << " has a value of " << vm.registers[addr] << "\n";

	}

	template<typename Tokens>
	static void set_reg( virtual_machine_t & vm, Tokens const & tokens ) {
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

	template<typename Tokens>
	static void set_bp( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Setting breakpoint at " << addr << "\n";
		assert( addr < vm.memory.size( ) );
		vm.debugging.breakpoints.insert( addr );
	}

	template<typename Tokens>
	static void clear_bp( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Clear breakpoint at " << addr << "\n";
		assert( addr < vm.memory.size( ) );
		vm.debugging.breakpoints.erase( addr );
	}

	template<typename Tokens>
	static void set_memory_trap( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Setting memory trap at " << addr << "\n";
		assert( addr < vm.memory.size( ) + 8 );
		vm.debugging.memory_traps.insert( addr );
	}

	template<typename Tokens>
	static void clear_memory_trap( virtual_machine_t & vm, Tokens const & tokens ) {
		if( tokens.size( ) != 2 ) {
			std::cout << "Error\n";
			return;
		}
		auto addr = convert<uint16_t>( tokens[1] );
		std::cout << "Clear memory trap at " << addr << "\n";
		assert( addr < vm.memory.size( ) );
		vm.debugging.memory_traps.erase( addr );
	}

	static void save_asm( virtual_machine_t & vm, boost::string_ref fname );
	static void get_ip( virtual_machine_t & vm );
	static void tick( virtual_machine_t & vm );
	static void get_regs( virtual_machine_t & vm );
	static void get_bps( virtual_machine_t & vm );
	static void clear_bps( virtual_machine_t & vm );
	static void get_memory_traps( virtual_machine_t & vm );
	static void clear_memory_traps( virtual_machine_t & vm );
	static void save_state( virtual_machine_t & vm, boost::string_ref fname );
	static void load_state( virtual_machine_t & vm, boost::string_ref fname );
	static void show_argument_stack( virtual_machine_t & vm );
	static void show_program_stack( virtual_machine_t & vm );
	static void save_trace( virtual_machine_t & vm, boost::string_ref fname );
	static void start_tracing( virtual_machine_t & vm );
	static void stop_tracing( virtual_machine_t & vm );
};	//struct vm_control
