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
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <boost/utility/string_ref.hpp>
#include "memory_helper.h"


enum class ContainerFileStates { OPEN, ERROR, CLOSED };

template<typename T>
struct ReadOnlyFileAsContainer {
	using storage_type = container_conversion<T const>;	
	using value_type = typename T;
	using iterator = typename storage_type::iterator;
	using const_iterator = typename storage_type::const_iterator;
	using reference = typename storage_type::reference;
	using const_reference = typename storage_type::const_reference;

private:
	boost::iostreams::mapped_file_source m_mapped_file;
	std::unique_ptr<storage_type> m_file_data;
	ContainerFileStates m_state;

public:
	ReadOnlyFileAsContainer( boost::string_ref filename, size_t items = (boost::iostreams::mapped_file::max_length / sizeof( value_type )), boost::intmax_t offset = 0 ):
		m_mapped_file( filename.data( ), items*sizeof( value_type ), offset ),
		m_file_data( ), 
		m_state( ContainerFileStates::ERROR ) {
		if( !m_mapped_file.is_open( ) ) {
			return;
		}
		m_state = ContainerFileStates::OPEN;
		m_file_data = std::make_unique<storage_type>( m_mapped_file.data( ), m_mapped_file.data( ) + m_mapped_file.size( ) );
	}

	~ReadOnlyFileAsContainer( ) {
		close( );
	}

	bool is_open( ) const {
		return m_state == ContainerFileStates::OPEN;
	}

	size_t size( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->size( );
	}

	const_iterator begin( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->begin( );
	}

	const_iterator cbegin( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->begin( );
	}

	const_iterator end( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->end( );
	}

	const_iterator cend( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->end( );
	}

	const_reference operator[]( size_t pos ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}

	const_reference at( size_t pos ) const {
		assert( pos < size( ) );
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}

	ContainerFileStates state( ) const {
		return m_state;
	}

	void close( ) {
		if( m_mapped_file ) {
			m_mapped_file.close( );
		}
	}
	explicit operator bool( ) const {
		return m_state == ContainerFileStates::OPEN;	
	}

};	// struct ReadFileAsContainer

template<typename T>
struct FileAsContainer {
	using storage_type = container_conversion<T>;
	using value_type = typename T;
	using iterator = typename storage_type::iterator;
	using const_iterator = typename storage_type::const_iterator;
	using reference = typename storage_type::reference;
	using const_reference = typename storage_type::const_reference;

private:
	boost::iostreams::mapped_file m_mapped_file;
	std::unique_ptr<storage_type> m_file_data;
	ContainerFileStates m_state;

public:
	FileAsContainer( boost::string_ref filename, size_t items = (boost::iostreams::mapped_file::max_length/sizeof(value_type)), boost::intmax_t offset = 0 ):
		m_mapped_file( filename.data( ), items*sizeof(value_type), offset ),
		m_file_data( ),
		m_state( ContainerFileStates::OPEN ) {
		if( !m_mapped_file.is_open( ) ) {
			m_state = ContainerFileStates::ERROR;
			return;
		}
		m_file_data = std::make_unique<storage_type>( m_mapped_file.data( ), m_mapped_file.data( ) + m_mapped_file.size( ) );
	}

	~FileAsContainer( ) {
		close( );
	}

	bool is_open( ) const {
		return m_state == ContainerFileStates::OPEN;
	}

	size_t size( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->size( );
	}

	iterator begin( ) {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->begin( );
	}

	const_iterator begin( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->begin( );
	}

	const_iterator cbegin( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->begin( );
	}

	iterator end( ) {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->end( );
	}

	const_iterator end( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->end( );
	}

	const_iterator cend( ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return m_file_data->end( );
	}

	reference operator[]( size_t pos ) {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}


	const_reference operator[]( size_t pos ) const {
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}

	reference at( size_t pos ) {
		assert( pos < size( ) );
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}


	const_reference at( size_t pos ) const {
		assert( pos < size( ) );
		if( is_open( ) ) {
			throw std::runtime_error( "Attempt to access null data" );
		}
		return (*m_file_data)[pos];
	}

	ContainerFileStates state( ) const {
		return m_state;
	}
	
	void close( ) {
		m_state = ContainerFileStates::CLOSED;
		if( m_mapped_file ) {
			m_mapped_file.close( );
		}
	}
	explicit operator bool( ) const {
		return m_state == ContainerFileStates::OPEN;
	}

};	// struct WriteFileAsContainer

std::string generate_unique_file_name( boost::string_ref prefix = "", boost::string_ref suffix = "", boost::string_ref extension = "" );