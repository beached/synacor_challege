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
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <atomic>
#include "vm.h"

static std::atomic_flag should_break = ATOMIC_FLAG_INIT;
static boost::asio::io_service io;
static boost::asio::signal_set signals(io, SIGINT, SIGTERM);

void handler( boost::asio::signal_set& this_, boost::system::error_code error, int signal_number ) {
	if( !error ) {
		should_break.clear( );
		this_.async_wait(boost::bind(handler, boost::ref(this_), _1, _2));
	}
}

void start_handler( ) {
	should_break.test_and_set( );
	signals.async_wait(boost::bind(handler, boost::ref(signals), _1, _2));
	boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io))).detach( );
}

int main( int argc, char** argv ) {
	virtual_machine_t vm;
	zero_fill( vm.registers );
	zero_fill( vm.memory );
	vm.instruction_ptr = 0;

	if( argc <= 1 ) {
		std::cerr << "Must supply a vm file" << std::endl;
		exit( EXIT_FAILURE );
	}
	vm.memory.from_file( argv[1] );

	start_handler( );

	while( true ) {		
		auto const & decoded = instructions::decoder( )[vm.fetch_opcode( true )];
		for( size_t n = 0; n < decoded.arg_count; ++n ) {
			vm.argument_stack.push_back( vm.fetch_opcode( ) );
		}
		decoded.instruction( vm );
		if( !should_break.test_and_set( ) ) {
			vm.should_break = true;
		}
		
	}

	return EXIT_SUCCESS;
}

