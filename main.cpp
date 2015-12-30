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
#include <cstdlib>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/utility/string_ref.hpp>
#include <iostream>
#include <cstdint>

template<typename Iterator>
void zero_fill( Iterator begin, Iterator end ) {
	std::fill( begin, end, 0 );
}

template<typename Container>
void zero_fill( Container & c ) {
	zero_fill( std::begin( c ), std::end( c ) );
}

template<typename As>
struct container_conversion {
	As * m_begin;
	As * m_end;

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
	using array_t = std::array<uint16_t, 65536*3>;
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
		
		container_conversion<char> memory_view( m_memory.begin( ), m_memory.end( ) );

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

	reference operator[]( size_t pos ) {
		return m_memory[pos];
	}

	const_reference operator[]( size_t pos ) const {
		return m_memory[pos];
	}
};	// struct virtual_memory

std::array<uint16_t, 8> registers;
virtual_memory_t memory;

int main( int argc, char** argv ) {
	zero_fill( registers );
	if( argc <= 1 ) {
		std::cerr << "Must supply a vm file" << std::endl;
		exit( EXIT_FAILURE );
	}
	memory.from_file( argv[1] );


	return EXIT_SUCCESS;
}

