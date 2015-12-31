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
#include <cstdio>

#include <vector>
#include "vm.h"
#include "console.h"
#include "file_helper.h"

virtual_machine_t::virtual_machine_t( ):
	registers( ),
	memory( ),
	argument_stack( ),
	program_stack( ),
	should_break( false ),
	term_buff( ),
	instruction_ptr( 0 ),
	breakpoints( ),
	vm_file( ) {
	
	zero_fill( registers );
	zero_fill( memory );
}

virtual_machine_t::virtual_machine_t( boost::string_ref filename ):
	registers( ),
	memory( ),
	argument_stack( ),
	program_stack( ),
	should_break( false ),
	term_buff( ),
	instruction_ptr( 0 ),
	breakpoints( ),
	vm_file( filename.to_string( ) ) {
	
	load_state( vm_file );
}

void virtual_machine_t::clear( ) { 
	zero_fill( registers );
	zero_fill( memory );
	program_stack.clear( );
	argument_stack.clear( );
	instruction_ptr = 0;
}

void virtual_machine_t::save_state( boost::string_ref filename ) {
	// Format of file in uint16_t's.  So 2bytes per 
	// 0->32767 -> memory from 0->32767
	// 32768->32775 -> registers 0->7
	// 32776 -> instruction ptr
	// 32777 -> size of program stack
	// 32778->32778+[32777] -> program stack
	// 32778+[32777]+1 -> size of argument stack
	// 32778+[32777]+2->end -> argument stack

	// Should be compatible with contest file format as it just extends it.  Anything less than
	// or equal to 32767 items is only representing the memory and assumes zeros for unused 
	// space and all registers/stacks.  Otherwise it will be a full state dump as here
	auto const total_items = memory.size( ) + registers.size( ) + 1/*instruction_ptr*/
		+ 1/*program stack size*/ + program_stack.size( )
		+ 1/*argument stack size*/ + argument_stack.size( );
	FileAsContainer<uint16_t> f( filename, total_items, 0, true );
	if( !f ) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit( EXIT_FAILURE );
	}
	auto output_position = std::copy( memory.begin( ), memory.end( ), f.begin( ) );
	output_position = std::copy( registers.begin( ), registers.end( ), output_position );
	*output_position = instruction_ptr;
	++output_position;
	*output_position = static_cast<uint16_t>(program_stack.size( ));
	++output_position;
	output_position = std::copy( program_stack.begin( ), program_stack.end( ), output_position );

	*output_position = static_cast<uint16_t>(argument_stack.size( ));
	++output_position;
	std::copy( argument_stack.begin( ), argument_stack.end( ), output_position );
	f.close( );
}


void virtual_machine_t::load_state( boost::string_ref filename ) {
	// Format of file in uint16_t's.  So 2bytes per 
	// 0->32767 -> memory from 0->32767
	// 32768->32775 -> registers 0->7
	// 32776 -> instruction ptr
	// 32777 -> size of program stack
	// 32778->32778+[32777] -> program stack
	// 32778+[32777]+1 -> size of argument stack
	// 32778+[32777]+2->end -> argument stack

	// Should be compatible with contest file format as it just extends it.  Anything less than
	// or equal to 32767 items is only representing the memory and assumes zeros for unused 
	// space and all registers/stacks.  Otherwise it will be a full state dump as here
	clear( );
	ReadOnlyFileAsContainer<uint16_t> f( filename );
	if( !f ) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit( EXIT_FAILURE );
	}

	size_t offset = 0;

	auto get_it = [&f, &offset]( ) {
		return f.begin( ) + offset;
	};

	{
		auto memory_size = f.size( ) > 32768 ? 32768 : f.size( );
		std::copy( f.begin( ), f.begin( ) + memory_size, memory.begin( ) );
		offset += memory.size( );
	}
	if( f.size( ) > 32768 ) {	// Has more than memory
		{
			auto it = get_it( );
			std::copy( it, it + registers.size( ), registers.begin( ) );
			offset += registers.size( );
		}
		instruction_ptr = *get_it( );
		++offset;
		{
			size_t program_stack_size = *get_it( );
			++offset;
			auto it = get_it( );
			program_stack.reserve( program_stack_size );
			std::copy( it, it + program_stack_size, std::back_inserter( program_stack ) ); 
			offset += program_stack.size( );
		}
		{
			size_t argument_stack_size = *get_it( );
			++offset;
			auto it = get_it( );
			argument_stack.reserve( argument_stack_size );
			std::copy( it, it + argument_stack_size, std::back_inserter( argument_stack ) );
		}
	}
	f.close( );
}


void virtual_machine_t::tick( bool is_debugger ) {
		if( !is_debugger && (should_break || breakpoints.count( instruction_ptr ) > 0) ) {
			std::cout << "Breaking at address " << instruction_ptr << "\n";
			console( *this );
		}
		should_break = false;
		auto const & decoded = instructions::decoder( )[fetch_opcode( true )];
		for( size_t n = 0; n < decoded.arg_count; ++n ) {
			argument_stack.push_back( fetch_opcode( ) );
		}
		decoded.instruction( *this );
}

uint16_t & virtual_machine_t::get_register( uint16_t i ) {
	if( !is_register( i ) ) {
		std::cerr << "FATAL ERROR: get_register called with invalid value " << i << std::endl;
		exit( EXIT_FAILURE );
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

std::string dump_memory( virtual_machine_t & vm, uint16_t from_address, uint16_t to_address ) {
	if( to_address > vm.memory.size( ) ) {
		to_address = static_cast<uint16_t>(vm.memory.size( ));
	}
	if( from_address > to_address ) {
		from_address = to_address;
	}
	std::stringstream ss;

	auto get_mem = [&]( auto & addr, bool inc = true ) {
		if( addr >= vm.memory.size( ) ) {
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

	auto escape = []( int i ) {
		std::string str = std::to_string( i );
		while( str.size( ) < 3 ) {
			str = "0" + str;
		}
		str = "\\" + str;
		return str;
	};

	auto mem_to_str = [&]( auto i, bool raw_ascii = false ) {
		if( raw_ascii ) {
			if( is_alphanum( i ) ) {
				ss << static_cast<unsigned char>(i);
			} else {
				ss << escape( i );
			}
		} else if( virtual_machine_t::is_register( i ) ) {
			ss << "R" << static_cast<int>(i - virtual_machine_t::REGISTER0) << "(" << vm.get_register( i ) << ")";
		} else if( i < virtual_machine_t::REGISTER0 ) {
			ss << static_cast<int>(i);
		} else {
			ss << "INVALID(" << static_cast<int>(i) << ")";
		}
	};

	for( auto addr = from_address; addr < to_address; ++addr ) {
		ss << addr << ": ";
		auto val = get_mem( addr );
		auto const & decoder = instructions::decoder( );
		if( instructions::is_instruction( val ) ) {
			auto d = decoder[val];
			ss << d.name;
			if( val == 19/*OUT*/ ) {
				ss << " \"";
				do {
					mem_to_str( get_mem( addr ), true );
					val = get_mem( addr, false );
					if( val == 19 ) {
						++addr;
					}
				} while( val == 19 );
				ss << "\"";
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
		ss << "\n";
	}
	return ss.str( );
}


std::string full_dump_string( virtual_machine_t & vm, uint16_t from_address, uint16_t to_address ) {
	std::stringstream ss;
	ss << dump_memory( vm, from_address, to_address );
	ss << "\n\nInstruction Ptr: " << vm.instruction_ptr << "\n";
	ss << "Registers\n";
	for( uint16_t n=0; n<vm.registers.size( ); ++n ) {
		ss << "R" << n << ": " << static_cast<int>( vm.registers[n] ) << "\n";
	}
	return ss.str( );
}

void full_dump( virtual_machine_t & vm, uint16_t from_address, uint16_t to_address ) {
	std::cout << full_dump_string( vm, from_address, to_address );
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

		auto tmp = getchar( );
		if( tmp < 0 ) {
			vm.should_break = true;
		}
		if( vm.should_break ) {
			console( vm );	
			vm.should_break = false;
		}
		if( tmp < 0 ) {
			tmp = '\n';
		}
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

