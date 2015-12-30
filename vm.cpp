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

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <vector>
#include "vm.h"

uint16_t & virtual_machine_t::get_register( uint16_t i, bool log7 ) {
	if( !is_register( i ) ) {
		std::cerr << "FATAL ERROR: get_register called with invalid value " << i << std::endl;
		exit( EXIT_FAILURE );
	}
	if( log7 && i - REGISTER0 == 7 ) {
		std::cout << "Someone is peaking" << std::endl;
	}
	return registers[i - REGISTER0];
}

bool virtual_machine_t::is_value( uint16_t i ) {
	return i < REGISTER0;
}

bool virtual_machine_t::is_register( uint16_t i ) {
	return i >= REGISTER0 && i < (REGISTER0 + 8);
}

void virtual_machine_t::validate( uint16_t i ) {
	if( i < (REGISTER0 + 8) ) {
		return;
	}
	std::cerr << "Invalid instruction in memory " << i << std::endl;
	exit( EXIT_FAILURE );
}

uint16_t & virtual_machine_t::get_value( uint16_t & i ) {
	validate( i );
	if( is_register( i ) ) {
		return get_register( i );
	}
	return i;
}

uint16_t & virtual_machine_t::get_reg_or_mem( uint16_t i ) {
	validate( i );
	if( is_register( i ) ) {
		return get_register( i );
	}
	return memory[i];
}

uint16_t virtual_machine_t::pop_argument_stack( ) {
	if( argument_stack.empty( ) ) {
		std::cerr << "INSTRUCTION STACK UNDERFLOW" << std::endl;
		exit( EXIT_FAILURE );
	}
	auto result = *argument_stack.rbegin( );
	argument_stack.pop_back( );
	return result;
}

uint16_t virtual_machine_t::pop_program_stack( ) {
	if( program_stack.empty( ) ) {
		std::cerr << "STACK UNDERFLOW" << std::endl;
		exit( EXIT_FAILURE );
	}
	auto result = *program_stack.rbegin( );
	program_stack.pop_back( );
	return result;
}

uint16_t virtual_machine_t::fetch_opcode( bool is_instruction ) {
	auto current_instruction = memory[instruction_ptr];
	if( is_instruction ) {
		if( current_instruction >= instructions::decoder( ).size( ) ) {
			std::cerr << "FATAL ERROR: INVALID INSTRUCTION " << current_instruction << " @ location " << instruction_ptr << std::endl;
			exit( EXIT_FAILURE );
		}
	} else {
		validate( current_instruction );
	}
	++instruction_ptr;
	return current_instruction;
}

void virtual_machine_t::step( ) {

}

namespace instructions {
	void inst_halt( virtual_machine_t & ) {
		exit( EXIT_SUCCESS );
	}

	void inst_set( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_register( a ) = vm.get_value( b );
	}

	void inst_push( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );
		vm.program_stack.push_back( vm.get_value( a ) );
	}

	void inst_pop( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );

		auto s = vm.pop_program_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( s );
	}

	void inst_eq( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( b ) == vm.get_value( c ) ? 1 : 0;
	}

	void inst_gt( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( b ) > vm.get_value( c ) ? 1 : 0;
	}

	void inst_jmp( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );
		vm.instruction_ptr = vm.get_value( a );
	}

	void inst_jt( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		if( vm.get_value( a ) != 0 ) {
			vm.instruction_ptr = vm.get_value( b );
		}
	}

	void inst_jf( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		if( vm.get_value( a ) == 0 ) {
			vm.instruction_ptr = vm.get_value( b );
		}
	}

	void inst_add( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );

		vm.get_reg_or_mem( a ) = (vm.get_value( b ) + vm.get_value( c )) % vm.MODULO;
	}

	void inst_mult( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		auto tmp = (static_cast<uint32_t>(vm.get_value( b )) * static_cast<uint32_t>(vm.get_value( c ))) % vm.MODULO;
		vm.get_reg_or_mem( a ) = static_cast<uint16_t>(tmp);
	}

	void inst_mod( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( b ) % vm.get_value( c );
	}

	void inst_and( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( b ) & vm.get_value( c );
	}

	void inst_or( virtual_machine_t & vm ) {
		auto c = vm.pop_argument_stack( );
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.get_value( b ) | vm.get_value( c );
	}

	void inst_not( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		const uint16_t MASK = 0b1000000000000000;
		uint16_t val = vm.get_value( b );
		uint16_t tmp = val & MASK;
		uint16_t tmp2 = ~val & ~MASK;
		vm.get_reg_or_mem( a ) = tmp | tmp2;
	}

	void inst_rmem( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		vm.get_reg_or_mem( a ) = vm.memory[vm.get_value( b )];
	}

	void inst_wmem( virtual_machine_t & vm ) {
		auto b = vm.pop_argument_stack( );
		auto a = vm.pop_argument_stack( );
		auto val_a = vm.get_value( a );
		auto val_b = vm.get_value( b );
		if( !vm.is_value( val_b ) ) {
			std::cerr << "INVALID VALUE " << val_b << std::endl;
			exit( EXIT_FAILURE );
		} else if( !vm.is_value( val_a ) ) {
			std::cerr << "INVALID VALUE " << val_a << std::endl;
			exit( EXIT_FAILURE );
		}
		vm.memory[val_a] = val_b;
	}

	void inst_call( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );
		vm.program_stack.push_back( vm.instruction_ptr );
		vm.instruction_ptr = vm.get_value( a );
	}

	void inst_ret( virtual_machine_t & vm ) {
		auto a = vm.pop_program_stack( );
		vm.instruction_ptr = a;
	}

	void inst_out( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );
		std::cout << static_cast<char>(vm.get_value( a ));
	}

	void inst_in( virtual_machine_t & vm ) {
		auto a = vm.pop_argument_stack( );

		auto tmp = vm.term_buff.get( );

		vm.get_reg_or_mem( a ) = static_cast<uint16_t>(tmp);
	}

	void inst_noop( virtual_machine_t & ) {

	}

	decoded_inst_t::decoded_inst_t( size_t ac, decoded_inst_t::instruction_t i, std::string n ):
		arg_count( ac ),
		instruction( i ),
		name( std::move( n ) ) { }

	std::vector<decoded_inst_t> const & decoder( ) {
		static std::vector<decoded_inst_t> const decoder = {
			decoded_inst_t{ 0, inst_halt, "HALT" },	// 0
			decoded_inst_t{ 2, inst_set, "SET" },	// 1
			decoded_inst_t{ 1, inst_push, "PUSH" },	// 2
			decoded_inst_t{ 1, inst_pop, "POP" },	// 3
			decoded_inst_t{ 3, inst_eq, "EQ" },		// 4 
			decoded_inst_t{ 3, inst_gt, "GT" },		// 5
			decoded_inst_t{ 1, inst_jmp, "JMP" },	// 6
			decoded_inst_t{ 2, inst_jt, "JT" },		// 7
			decoded_inst_t{ 2, inst_jf, "JF" },		// 8
			decoded_inst_t{ 3, inst_add, "ADD" },	// 9
			decoded_inst_t{ 3, inst_mult, "MULT" },	// 10
			decoded_inst_t{ 3, inst_mod, "MOD" },	// 11
			decoded_inst_t{ 3, inst_and, "AND" },	// 12
			decoded_inst_t{ 3, inst_or, "OR" },		// 13
			decoded_inst_t{ 2, inst_not, "NOT" },	// 14
			decoded_inst_t{ 2, inst_rmem, "RMEM" },	// 15
			decoded_inst_t{ 2, inst_wmem, "WMEM" },	// 16
			decoded_inst_t{ 1, inst_call, "CALL" },	// 17
			decoded_inst_t{ 0, inst_ret, "RET" },	// 18
			decoded_inst_t{ 1, inst_out, "OUT" },	// 19
			decoded_inst_t{ 1, inst_in, "IN" },		// 20
			decoded_inst_t{ 0, inst_noop, "NOOP" }	// 21
		};
		return decoder;
	}

	bool is_instruction( uint16_t i ) {
		return i < instructions::decoder( ).size( );
	}
}	// namespace instructions

bool is_alphanum( uint16_t i ) {
	return 32 <= i && i < 127;
}

