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

int main( int argc, char** argv ) {
	if( argc <= 1 ) {
		std::cerr << "Must supply a vm file" << std::endl;
		exit( EXIT_FAILURE );
	}
	std::atomic_flag should_break = ATOMIC_FLAG_INIT;
	boost::asio::io_service io;
	boost::asio::signal_set signals(io, SIGINT);

	virtual_machine_t vm( argv[1] );

	std::function<void( boost::asio::signal_set&, boost::system::error_code, int )> handler;

	handler = [&]( boost::asio::signal_set& this_, boost::system::error_code error, int signal_number ) {
		if( !error ) {
			if( !should_break.test_and_set( ) ) {
				std::cout << "EXITING" << std::endl;
				exit( EXIT_SUCCESS );
			}
			if( !vm.should_break ) {
				should_break.clear( );
			}
			this_.async_wait( boost::bind( handler, boost::ref( this_ ), _1, _2 ) );
		}
	};
	{	// start SIGINT handler
		should_break.test_and_set( );
		signals.async_wait(boost::bind(handler, boost::ref(signals), _1, _2));
		boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io))).detach( );
	}
	
	// Start main loop
	while( true ) {		
		vm.tick( );
		if( !should_break.test_and_set( ) ) {
			vm.should_break = true;
		}
		
	}

	return EXIT_SUCCESS;
}

