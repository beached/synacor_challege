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
#include <cctype>

#include "memory_helper.h"
#include "helpers.h"

namespace {
	virtual_memory_t<8> registers;
	std::vector<uint16_t> argument_stack;
	std::vector<uint16_t> program_stack;
	term_buff_t term_buff;
	virtual_memory_t<32768u> memory;
	const uint16_t MODULO = 32768;

	uint16_t instruction_ptr;

	uint16_t const REGISTER0 = 32768;

	bool is_value( uint16_t i ) {
		return (i & 0b0111111111111111) == i;
	}

	bool is_register( uint16_t i ) {
		return i >= REGISTER0 && i < (REGISTER0 + 8);
	}

	void validate( uint16_t i ) {
		if( is_register( i ) || (i & 0b0111111111111111) == i ) {
			return;
		}
		std::cerr << "Invalid instruction in memory " << i << std::endl;
		exit( EXIT_FAILURE );
	}

	uint16_t & get_register( uint16_t i ) {
		if( !is_register( i ) ) {
			std::cerr << "FATAL ERROR: get_register called with invalid value " << i << std::endl;
			exit( EXIT_FAILURE );
		}
		return registers[i - REGISTER0];
	}

	uint16_t & get_value( uint16_t & i ) {
		validate( i );
		if( is_register( i ) ) {
			return get_register( i );
		}
		return i;
	}

	uint16_t & get_reg_or_mem( uint16_t i ) {
		validate( i );
		if( is_register( i ) ) {
			return get_register( i );
		}
		return memory[i];
	}

	uint16_t pop_argument_stack( ) {
		if( argument_stack.empty( ) ) {
			std::cerr << "INSTRUCTION STACK UNDERFLOW" << std::endl;
			exit( EXIT_FAILURE );
		}
		auto result = *argument_stack.rbegin( );
		argument_stack.pop_back( );
		return result;
	}

	uint16_t pop_program_stack( ) {
		if( program_stack.empty( ) ) {
			std::cerr << "STACK UNDERFLOW" << std::endl;
			exit( EXIT_FAILURE );
		}
		auto result = *program_stack.rbegin( );
		program_stack.pop_back( );
		return result;
	}

	namespace instructions {
		void inst_halt( ) {
			exit( EXIT_SUCCESS );
		}

		void inst_set( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_register( a ) = get_value( b );
		}

		void inst_push( ) {
			auto a = pop_argument_stack( );
			program_stack.push_back( get_value( a ) );
		}

		void inst_pop( ) {
			auto a = pop_argument_stack( );

			auto s = pop_program_stack( );
			get_reg_or_mem( a ) = get_value( s );
		}

		void inst_eq( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = get_value( b ) == get_value( c ) ? 1 : 0;
		}

		void inst_gt( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = get_value( b ) > get_value( c ) ? 1 : 0;
		}

		void inst_jmp( ) {
			auto a = pop_argument_stack( );
			instruction_ptr = get_value( a );
		}

		void inst_jt( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			if( get_value( a ) != 0 ) {
				instruction_ptr = get_value( b );
			}
		}

		void inst_jf( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			if( get_value( a ) == 0 ) {
				instruction_ptr = get_value( b );
			}
		}

		void inst_add( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );

			get_reg_or_mem( a ) = (get_value( b ) + get_value( c )) % MODULO;
		}

		void inst_mult( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			auto tmp = (static_cast<uint32_t>(get_value( b )) * static_cast<uint32_t>(get_value( c ))) % 32768u;
			get_reg_or_mem( a ) = static_cast<uint16_t>(tmp);
		}

		void inst_mod( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = get_value( b ) % get_value( c );
		}

		void inst_and( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = get_value( b ) & get_value( c );
		}

		void inst_or( ) {
			auto c = pop_argument_stack( );
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = get_value( b ) | get_value( c );
		}

		void inst_not( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			const uint16_t MASK = 0b1000000000000000;
			uint16_t val = get_value( b );
			uint16_t tmp = val & MASK;
			uint16_t tmp2 = ~val & ~MASK;
			get_reg_or_mem( a ) = tmp | tmp2;
		}

		void inst_rmem( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			get_reg_or_mem( a ) = memory[get_value( b )];
		}

		void inst_wmem( ) {
			auto b = pop_argument_stack( );
			auto a = pop_argument_stack( );
			auto val_a = get_value( a );
			auto val_b = get_value( b );
			if( !is_value( val_b ) ) {
				std::cerr << "INVALID VALUE " << val_b << std::endl;
				exit( EXIT_FAILURE );
			} else if( !is_value( val_a ) ) {
				std::cerr << "INVALID VALUE " << val_a << std::endl;
				exit( EXIT_FAILURE );
			}
			memory[val_a] = val_b;
		}

		void inst_call( ) {
			auto a = pop_argument_stack( );
			program_stack.push_back( instruction_ptr );
			instruction_ptr = get_value( a );
		}

		void inst_ret( ) {
			auto a = pop_program_stack( );
			instruction_ptr = a;
		}

		void inst_out( ) {
			auto a = pop_argument_stack( );
			std::cout << static_cast<char>(get_value( a ));
		}

		void inst_in( ) {
			auto a = pop_argument_stack( );

			auto tmp = term_buff.get( );

			get_reg_or_mem( a ) = static_cast<uint16_t>(tmp);
		}

		void inst_noop( ) {

		}

		struct decoded_inst_t {
			typedef void( *instruction_t )();
			size_t const arg_count;
			instruction_t instruction;
			std::string name;

			decoded_inst_t( size_t ac, instruction_t i, std::string n ):
				arg_count( ac ),
				instruction( i ),
				name( std::move( n ) ) { }
		};

		std::vector<decoded_inst_t> decoder = {
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
	}
}

bool is_instruction( uint16_t i ) {
	return i < instructions::decoder.size( );
}

bool is_alphanum( uint16_t i ) {
	auto const c = static_cast<char>(i);
	return std::isalnum( c ) || c == ' ' || c == '.';
}

int main( int argc, char** argv ) {
	zero_fill( registers );
	if( argc <= 1 ) {
		std::cerr << "Must supply a vm file" << std::endl;
		exit( EXIT_FAILURE );
	}
	memory.from_file( argv[1] );
	instruction_ptr = 0;

	for( instruction_ptr = 0; instruction_ptr < memory.size( ); ++instruction_ptr ) {
		auto const val = memory[instruction_ptr];
		std::cout << instruction_ptr << ": ";
		if( is_instruction( val ) ) {
			auto d = instructions::decoder[val];
			std::cout << d.name << "(" << d.arg_count << ")";
		} else if( is_register( val ) ) {
			std::cout << "REGISTER_" << (val - REGISTER0);
		} else if( is_value( val ) ) {
			if( is_alphanum( val ) ) {
				std::cout << "ASCII " << val << "'" << static_cast<char>(val) << "'";
			} else {
				std::cout << "VALUE " << val;
			}
		} else {
			std::cout << "INVALID";
		}
		std::cout << "\n";
	}
	return EXIT_SUCCESS;
}

