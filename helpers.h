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
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cassert>


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
		m_begin( reinterpret_cast<As*>(Begin) ),
		m_end( reinterpret_cast<As*>(End) ) { }

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
		return static_cast<size_t>(std::distance( m_begin, m_end ));
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
			if( has_read ) {
				has_read = false;
				return '\n';
			}
			if( !std::getline( std::cin, cur_line ) ) {
				std::cerr << "ERRROR ATTEMPTING TO READ INPUT" << std::endl;
				exit( EXIT_FAILURE );
			}
			has_read = true;
			pos = cur_line.begin( );
			if( pos == cur_line.end( ) ) {
				std::cerr << "ERRROR ATTEMPTING TO READ INPUT" << std::endl;
				exit( EXIT_FAILURE );
			}
		}
		auto result = *pos;
		++pos;
		return result;
	}
};	// class term_buff_t
