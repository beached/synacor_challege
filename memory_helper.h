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

#include <array>
#include <cassert>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <boost/utility/string_ref.hpp>
#include <sstream>
#include "helpers.h"

template<size_t SIZE, typename T = uint16_t>
struct virtual_memory_t {
	using array_t = std::array<T, SIZE>;
	using iterator = typename array_t::iterator;
	using const_iterator = typename array_t::const_iterator;
	using value_type = typename array_t::value_type;
	using reference = typename array_t::reference;
	using const_reference = typename array_t::const_reference;
private:
	array_t m_memory;		
public:
	virtual_memory_t( ): m_memory { } {
		zero_fill( m_memory );
	}
	~virtual_memory_t( ) = default;
	virtual_memory_t( virtual_memory_t const & ) = default;
	virtual_memory_t( virtual_memory_t && ) = default;
	virtual_memory_t & operator=( virtual_memory_t const & ) = default;
	virtual_memory_t & operator=( virtual_memory_t && ) = default;

	virtual_memory_t( boost::string_ref filename ): m_memory { } {
		zero_fill( m_memory );
		from_file( filename );
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
		return SIZE;
	}

	reference operator[]( size_t pos ) {
		if( pos >= m_memory.size( ) ) {
			std::cerr << "OUT OF RANGE MEMORY ATTEMP\n";
			exit( EXIT_FAILURE );
		}
		return m_memory[pos];
	}

	const_reference operator[]( size_t pos ) const {
		if( pos >= m_memory.size( ) ) {
			std::cerr << "OUT OF RANGE MEMORY ATTEMP\n";
			exit( EXIT_FAILURE );
		}
		return m_memory[pos];
	}
};	// struct virtual_memory_t

template<size_t SIZE, typename T>
std::string to_json( virtual_memory_t<SIZE, T> const & mem ) {
	std::stringstream ss;
	size_t pos = 0;
	ss << "[ ";
	for( auto it = mem.begin( ); it != mem.end( ); ++it, ++pos ) {
		if( pos > 0 ) {
			ss << ", ";
		}
		ss << *it;
	}
	ss << " ]";
	return ss.str( );
}


