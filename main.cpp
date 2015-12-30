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

#include <array>
#include <vector>
#include <cstdlib>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/utility/string_ref.hpp>
#include <iostream>
#include <cstdint>
#include <string>
#include <iterator>

template<typename Iterator>
void zero_fill( Iterator begin, Iterator end ) {
	std::fill( begin, end, 0 );
}

template<typename Container>
void zero_fill( Container & c ) {
	zero_fill( std::begin( c ), std::end( c ) );
}

template<typename Iterator>
struct range_view {
	using iterator = Iterator;
	using const_iterator = const Iterator;
	using value_type = typename std::iterator_traits<Iterator>::value_type;
	using reference = typename std::iterator_traits<Iterator>::reference;
	using const_reference = typename std::iterator_traits<Iterator>::const_reference;

private:
	Iterator * m_begin;
	Iterator * m_end;
public:

	range_view( ) = delete;
	~range_view( ) = default;
	range_view( range_view const & ) = default;
	range_view( range_view && ) = default;
	range_view & operator=( range_view const & ) = default;
	range_view & operator=( range_view && ) = default;


	range_view( Iterator Begin, Iterator End ):
		m_begin( Begin ),
		m_end( End ) { }

	iterator begin( ) {
		return m_begin;
	}

	const_iterator begin( ) const {
		return m_begin;
	}

	iterator end( ) {
		return m_end;
	}

	const_iterator end( ) const {
		return m_end;
	}

	size_t size( ) const {
		return static_cast<size_t>(std::distance( m_begin, m_end ));
	}

	reference operator[]( size_t pos ) {
		assert( pos < size( ) );
		return *(m_begin + pos);
	}

	const_reference operator[]( size_t pos ) const {
		assert( pos < size( ) );
		return *(m_begin + pos);
	}
};	// struct range_view


template<typename As>
struct container_conversion {
	As * m_begin;
	As * m_end;

	container_conversion( ) = delete;
	~container_conversion( ) = default;
	container_conversion( container_conversion const & ) = default;
	container_conversion( container_conversion && ) = default;
	container_conversion & operator=( container_conversion const & ) = default;
	container_conversion & operator=( container_conversion && ) = default;


	template<typename From>
	container_conversion( From * Begin, From * End ):
		m_begin( reinterpret_cast<As*>( Begin ) ),
		m_end( reinterpret_cast<As*>( End ) ) { }

	As * begin( ) {
		return m_begin;
	}

	As const * begin( ) const {
		return m_begin;
	}

	As * end( ) {
		return m_end;
	}

	As const * end( ) const {
		return m_end;
	}

	size_t size( ) const {
		return static_cast<size_t>( std::distance( m_begin, m_end ) );
	}

	As & operator[]( size_t pos ) {
		assert( pos < size( ) );
		return *(m_begin + pos);
	}

	As const & operator[]( size_t pos ) const {
		assert( pos < size( ) );
		return *(m_begin + pos);
	}
};	// struct container_conversion

struct virtual_memory_t {
	using array_t = std::array<uint16_t, 32768>;
	using iterator = typename array_t::iterator;
	using const_iterator = typename array_t::const_iterator;
	using value_type = typename array_t::value_type;
	using reference = typename array_t::reference;
	using const_reference = typename array_t::const_reference;
private:
	array_t m_memory;	 
public:
	virtual_memory_t( ): m_memory{ } {
		zero_fill( m_memory );
	}
	~virtual_memory_t( ) = default;
	virtual_memory_t( virtual_memory_t const & ) = default;
	virtual_memory_t( virtual_memory_t && ) = default;
	virtual_memory_t & operator=( virtual_memory_t const & ) = default;
	virtual_memory_t & operator=( virtual_memory_t && ) = default;

	virtual_memory_t( boost::string_ref filename ): m_memory{ } {
		zero_fill( m_memory );
		from_file( filename );
	}

	void from_file( boost::string_ref filename ) {
		boost::iostreams::mapped_file_source file_memory;
		file_memory.open( filename.data( ) );
		if( !file_memory.is_open( ) ) {
			std::cerr << "Error opening file: " << filename << std::endl;
			exit( EXIT_FAILURE );
		}
		
		container_conversion<char> memory_view( m_memory.data( ), m_memory.data( ) + m_memory.size( ) );

		if( file_memory.size( ) > memory_view.size( ) ) {
			std::cerr << "VM File does not have the correct size.  It is " << file_memory.size( ) << " bytes, which is > " << memory_view.size( ) << "bytes" << std::endl; 
			exit( EXIT_FAILURE );
		}

		std::copy( file_memory.begin( ), file_memory.end( ), memory_view.begin( ) );

		file_memory.close( );
	}

	iterator begin( ) {
		return m_memory.begin( );
	}

	const_iterator begin( ) const {
		return m_memory.begin( );
	}

	iterator end( ) {
		return m_memory.end( );
	}

	const_iterator end( ) const {
		return m_memory.end( );
	}
	
	size_t size( ) const {
		return m_memory.size( );
	}

	reference operator[]( uint16_t pos ) {
		assert( pos < m_memory.size( ) );
		return m_memory[pos];
	}

	const_reference operator[]( size_t pos ) const {
		assert( pos < m_memory.size( ) );
		return m_memory[pos];
	}
};	// struct virtual_memory

class term_buff_t {
	std::string cur_line;
	std::string::iterator pos;
	bool has_read;
public:
	term_buff_t( ):
		cur_line( ),
		pos( cur_line.end( ) ),
		has_read( false ) { }

	~term_buff_t( ) = default;
	term_buff_t( term_buff_t const & ) = delete;
	term_buff_t( term_buff_t && ) = default;
	term_buff_t & operator=( term_buff_t const & ) = delete;
	term_buff_t & operator=( term_buff_t && ) = default;

	unsigned char get( ) {
		if( pos == cur_line.end( ) ) {
			if( !std::getline( std::cin, cur_line ) ) {
				std::cerr << "ERRROR ATTEMPTING TO READ INPUT" << std::endl;
				exit( EXIT_FAILURE );
			}
			pos = cur_line.begin( );
			if( has_read ) {				
				return '\n';
			} else if( pos == cur_line.end( ) ) {
				std::cerr << "ERRROR ATTEMPTING TO READ INPUT" << std::endl;
				exit( EXIT_FAILURE );
			}
		}
		auto result = *pos;
		++pos;
		return result;
	}
};	// class term_buff_t

namespace {
	std::array<uint16_t, 8> registers;
	std::vector<uint16_t> inst_stack;
	std::vector<uint16_t> stack;
	term_buff_t term_buff;
	virtual_memory_t memory;
	const uint16_t MODULO = 32768;

	uint16_t instruction_ptr;

	uint16_t const REG_POS[8] = { 32768, 32769, 32770, 32771, 32772, 32773, 32774, 32775 };
}
bool is_value( uint16_t i ) {
	return i < REG_POS[0];
}

bool is_register( uint16_t i ) {
	return i >= REG_POS[0] && i <= REG_POS[7];
}

void validate( uint16_t i ) {
	if( is_value( i ) || is_register( i ) ) {
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
	return registers[i - REG_POS[0]];
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

uint16_t pop_istack( ) {
	if( inst_stack.empty( ) ) {
		std::cerr << "INSTRUCTION STACK UNDERFLOW" << std::endl;
		exit( EXIT_FAILURE );
	}
	auto result = *inst_stack.rbegin( );
	inst_stack.pop_back( );
	return result;
}

uint16_t pop_stack( ) {
	if( stack.empty( ) ) {
		std::cerr << "STACK UNDERFLOW" << std::endl;
		exit( EXIT_FAILURE );
	}
	auto result = *stack.rbegin( );
	stack.pop_back( );
	return result;
}

namespace instructions {
	void inst_halt( ) {
		exit( EXIT_SUCCESS );
	}

	void inst_set( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );		
		get_register( a ) = get_value( b );
	}

	void inst_push( ) {
		auto a = pop_istack( );
		stack.push_back( get_value( a ) );
	}

	void inst_pop( ) {
		auto a = pop_istack( );

		auto s = pop_stack( );
		get_reg_or_mem( a ) = get_value( s );
	}

	void inst_eq( ) {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = get_value( b ) == get_value( c ) ? 1 : 0;
	}

	void inst_gt( ) {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = get_value( b ) > get_value( c ) ? 1 : 0;
	}

	void inst_jmp( ) {
		auto a = pop_istack( );
		instruction_ptr = get_value( a );
	}

	void inst_jt( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );
		if( get_value( a ) != 0 ) {
			instruction_ptr = get_value( b );
		}
	}

	void inst_jf( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );
		if( get_value( a ) == 0 ) {
			instruction_ptr = get_value( b );
		}
	}

	void inst_add( ) { 
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		
		get_reg_or_mem( a ) = (get_value( b ) + get_value( c )) % MODULO;
	}

	void inst_mult( ) {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		auto tmp = (static_cast<uint32_t>(get_value( b )) * static_cast<uint32_t>(get_value( c ))) % 32768u;
		get_reg_or_mem( a ) = static_cast<uint16_t>(tmp);
	}

	void inst_mod( ) {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = get_value( b ) % get_value( c );
	}

	void inst_and( ) {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = get_value( b ) & get_value( c );
	}

	void inst_or() {
		auto c = pop_istack( );
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = get_value( b ) | get_value( c );
	}

	void inst_not( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );
		const uint16_t MASK = 0b1000000000000000;
		uint16_t val = get_value( b );
		uint16_t tmp = val & MASK;
		uint16_t tmp2 = ~val & ~MASK; 
		get_reg_or_mem( a ) = tmp | tmp2; 
	}

	void inst_rmem( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );
		get_reg_or_mem( a ) = memory[get_value( b )];
	}

	void inst_wmem( ) {
		auto b = pop_istack( );
		auto a = pop_istack( );
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
		auto a = pop_istack( );
		stack.push_back( instruction_ptr );
		instruction_ptr = get_value( a );
	}

	void inst_ret( ) {
		auto a = pop_stack( );
		instruction_ptr = a;
	}

	void inst_out( ) { 
		auto a = pop_istack( );
		std::cout << static_cast<char>( get_value( a ) );
	}

	void inst_in( ) {
		auto a = pop_istack( );
		get_reg_or_mem( a ) = term_buff.get( );
	}

	void inst_noop( ) {

	}

	struct decoded_inst_t {
		typedef void( *instruction_t )( );
		size_t const arg_count;
		instruction_t instruction;

		decoded_inst_t( size_t ac, instruction_t i ): 
			arg_count( ac ),
			instruction( i ) { }
	};

	namespace {
		std::vector<decoded_inst_t> decoder = {
			{ 0, inst_halt },	// 0
			{ 2, inst_set },	// 1
			{ 1, inst_push },	// 2
			{ 1, inst_pop },	// 3
			{ 3, inst_eq },		// 4 
			{ 3, inst_gt },		// 5
			{ 1, inst_jmp },	// 6
			{ 2, inst_jt },		// 7
			{ 2, inst_jf },		// 8
			{ 3, inst_add },	// 9
			{ 3, inst_mult },	// 10
			{ 3, inst_mod },	// 11
			{ 3, inst_and },	// 12
			{ 3, inst_or },		// 13
			{ 2, inst_not },	// 14
			{ 2, inst_rmem },	// 15
			{ 2, inst_wmem },	// 16
			{ 1, inst_call },	// 17
			{ 0, inst_ret },	// 18
			{ 1, inst_out },	// 19
			{ 1, inst_in },		// 20
			{ 0, inst_noop }	// 21
		};
	}
}

int main( int argc, char** argv ) {
	zero_fill( registers );
	if( argc <= 1 ) {
		std::cerr << "Must supply a vm file" << std::endl;
		exit( EXIT_FAILURE );
	}
	memory.from_file( argv[1] );
	instruction_ptr = 0;
	
	do {
		auto current_instruction = memory[instruction_ptr];
		if( current_instruction >= instructions::decoder.size( ) ) {
			std::cerr << "FATAL ERROR: INVALID INSTRUCTION" << std::endl;
			exit( EXIT_FAILURE );
		}
		auto const & decoded = instructions::decoder[current_instruction];
		++instruction_ptr;
		for( size_t n = 0; n < decoded.arg_count; ++n ) {
			inst_stack.push_back( memory[instruction_ptr] );
			++instruction_ptr;
		}
		decoded.instruction( );
	} while( instruction_ptr < memory.size( ) );
	return EXIT_SUCCESS;
}

