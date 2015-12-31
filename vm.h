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
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <vector>
#include "helpers.h"
#include "memory_helper.h"

struct virtual_machine_t {
	virtual_memory_t<8> registers;
	virtual_memory_t<32768u> memory;
	std::vector<uint16_t> argument_stack;
	std::vector<uint16_t> program_stack;

	term_buff_t term_buff;
	uint16_t instruction_ptr;

	static uint16_t const MODULO = 32768;
	static uint16_t const REGISTER0 = 32768;

	uint16_t & get_register( uint16_t i, bool log7 = true );
	static bool is_value( uint16_t i );
	static bool is_register( uint16_t i );
	static void validate( uint16_t i ); 
	uint16_t & get_value( uint16_t & i );
	uint16_t & get_reg_or_mem( uint16_t i );
	uint16_t pop_argument_stack( );	
	uint16_t pop_program_stack( );
	uint16_t fetch_opcode( bool is_instruction = false );
	void step( );
};	// struct virtual_machine_t


namespace instructions {
	void inst_halt( virtual_machine_t & );
	void inst_set( virtual_machine_t & vm );
	void inst_push( virtual_machine_t & vm );
	void inst_pop( virtual_machine_t & vm );
	void inst_eq( virtual_machine_t & vm );
	void inst_gt( virtual_machine_t & vm );
	void inst_jmp( virtual_machine_t & vm );
	void inst_jt( virtual_machine_t & vm );
	void inst_jf( virtual_machine_t & vm );
	void inst_add( virtual_machine_t & vm );
	void inst_mult( virtual_machine_t & vm );
	void inst_mod( virtual_machine_t & vm );
	void inst_and( virtual_machine_t & vm );
	void inst_or( virtual_machine_t & vm );
	void inst_not( virtual_machine_t & vm );
	void inst_rmem( virtual_machine_t & vm );
	void inst_wmem( virtual_machine_t & vm );
	void inst_call( virtual_machine_t & vm );
	void inst_ret( virtual_machine_t & vm );
	void inst_out( virtual_machine_t & vm );
	void inst_in( virtual_machine_t & vm );
	void inst_noop( virtual_machine_t & );

	struct decoded_inst_t {
		typedef void( *instruction_t )( virtual_machine_t & );
		size_t const arg_count;
		instruction_t instruction;
		std::string name;

		decoded_inst_t( size_t ac, instruction_t i, std::string n );
	};	// struct decoded_inst

	std::vector<decoded_inst_t> const & decoder( );
	bool is_instruction( uint16_t i );
}	// namespace instructions

bool is_alphanum( uint16_t i );

template<typename Decoder>
std::string dump_memory( virtual_machine_t & vm, Decoder decoder ) {
	std::stringstream ss;
	bool is_outputting = false;

	auto get_mem = [&]( auto & addr, bool inc = true ) {
		if( vm.memory.size( ) == addr ) {
			std::cerr << ss.str( ) << std::endl << std::endl;
			std::cerr << "UNEXPECTED END OF MEMORY\n" << std::endl;
			exit( EXIT_FAILURE );
		}
		if( inc ) {
			++addr;
			return vm.memory[addr - 1];
		}
		return vm.memory[addr];
	};

	auto is_next_output = [&]( uint16_t i ) {
		auto val = get_mem( i, false );
		if( !instructions::is_instruction( val ) ) {
			return false;
		}
		return 0 == decoder( val ).name.compare( "OUT" );
	};

	auto mem_to_str = [&]( auto i ) {
		if( virtual_machine_t::is_register( i ) ) {
			ss << "R" << static_cast<int>(i - virtual_machine_t::REGISTER0) << "(" << vm.get_register( i, false ) << ")";
		} else if( i < virtual_machine_t::REGISTER0 ) {
			ss << static_cast<int>(i);
			if( is_alphanum( i ) ) {
				ss << "'" << static_cast<unsigned char>(i) << "'";
			}
		} else {
			ss << "INVALID(" << static_cast<int>(i) << ")";
		}
	};

	bool is_output = false;

	for( uint16_t addr = 0; addr < vm.memory.size( ); ++addr ) {
		ss << addr << ": ";
		auto val = get_mem( addr );
		if( instructions::is_instruction( val ) ) {
			auto d = decoder( val );
			ss << d.name;
			if( val == 19/*OUT*/ ) {
				do {
					ss << " ";
					mem_to_str( get_mem( addr ) );
					val = get_mem( addr, false );
					if( val == 19 ) {
						++addr;
					}
				} while( val == 19 );
			} else {
				for( size_t n = 0; n < d.arg_count; ++n ) {
					ss << "  ";
					mem_to_str( get_mem( addr ) );
				}
			}
		} else {
			mem_to_str( val );
		}
		--addr;
		if( !is_outputting ) {
			ss << "\n";
		}
	}
	return ss.str( );
}


