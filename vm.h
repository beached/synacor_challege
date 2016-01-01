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
#include <set>
#include "helpers.h"
#include "memory_helper.h"

struct vm_trace {
	struct op_t {
		uint16_t op_code;
		std::vector<uint16_t> params;
		op_t( uint16_t OpCode, std::vector<uint16_t> Params = { } );

		std::string to_json( ) const;
	};

	struct memory_change_t {
		int32_t address;
		int32_t old_value;
		int32_t new_value;

		memory_change_t( );
		memory_change_t( uint16_t Address, uint16_t Old );

		void clear( );
		std::string to_json( ) const;
	};	// struct memory_change

	std::vector<uint16_t> instruction_ptrs;
	std::vector<op_t> op_codes;
	std::vector<memory_change_t> memory_changes;

	void clear( ) {
		instruction_ptrs.clear( );
		op_codes.clear( );
		memory_changes.clear( );
	}

	std::string to_json( ) const;
};	// struct vm_trace

struct virtual_machine_t {
	virtual_memory_t<8> registers;
	virtual_memory_t<32768u> memory;
	std::vector<uint16_t> argument_stack;
	std::vector<uint16_t> program_stack;
	bool should_break;
	uint16_t instruction_ptr;
	std::set<uint16_t> breakpoints;
	vm_trace trace;
	bool enable_tracing;

	static uint16_t const MODULO = 32768;
	static uint16_t const REGISTER0 = 32768;

	virtual_machine_t( );
	virtual_machine_t( boost::string_ref filename );

	void tick( bool is_debugger = false );
	uint16_t & get_register( uint16_t i );
	static bool is_value( uint16_t i );
	static bool is_register( uint16_t i );
	static void validate( uint16_t i ); 
	uint16_t & get_value( uint16_t & i );
	uint16_t & get_reg_or_mem( uint16_t i );
	uint16_t pop_argument_stack( );	
	uint16_t pop_program_stack( );
	uint16_t fetch_opcode( bool is_instruction = false );
	void save_state( boost::string_ref filename );
	void load_state( boost::string_ref filename );
	void clear( );

};	// struct virtual_machine_t

std::string full_dump_string( virtual_machine_t & vm, uint16_t from_address = 0, uint16_t to_address = std::numeric_limits<uint16_t>::max( ) );
void full_dump( virtual_machine_t & vm, uint16_t from_address = 0, uint16_t to_address = std::numeric_limits<uint16_t>::max( ) );

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
		uint16_t op_code;
		size_t const arg_count;		
		instruction_t instruction;
		std::string name;
		bool do_trace;		
		decoded_inst_t( uint16_t opcode, size_t ac, instruction_t i, std::string n, bool dotrace = false );
	};	// struct decoded_inst

	std::vector<decoded_inst_t> const & decoder( );
	bool is_instruction( uint16_t i );
}	// namespace instructions

bool is_alphanum( uint16_t i );

std::string dump_memory( virtual_machine_t & vm, uint16_t from_address = 0, uint16_t to_address = std::numeric_limits<uint16_t>::max( ) );
