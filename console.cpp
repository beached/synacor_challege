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

#include "vm.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "console.h"
#include <fstream>

namespace {
	std::string epoch( ) {
		using boost::posix_time::ptime;
		using boost::posix_time::time_from_string;
		static ptime const epoch = time_from_string( "1970-01-01 00:00:00.000" );
		ptime other = time_from_string( "2011-08-09 17:27:00.000" );

		return std::to_string( (other - epoch).total_milliseconds( ) );
	}

	void print_help( ) {
		std::cout << "Debugging console\nValid commmands are:\n";
		std::cout << "savememory -> save memory to a file(sc_<time since epoch>_dump.txt)\n";
		std::cout << "printmemory -> print all memory to screen\n";
		std::cout << "exit -> exit debugger\n";
		std::cout << "quit -> exit program\n";
	}

	template<typename T>
	T convert( boost::string_ref orig ) {
		std::stringstream ss;
		ss << orig;
		T result;
		ss >> result;
		return result;
	}
}



void console( virtual_machine_t & vm ) {
	std::cin.clear( );

	print_help( );

	std::string current_line;
	while( std::getline( std::cin, current_line ) ) {
		if( current_line == "printmemory" ) {
			full_dump( vm );
			std::cout << "\n\n";
		} else if( current_line == "savememory" ) {
			std::ofstream fout;
			std::string const fname = "sc_" + epoch( ) + "_dump.txt";
			fout.open( fname.c_str( ) );
			if( !fout ) {
				std::cerr << "Error saving memory dump to " << fname << "\n";
			} else {
				fout << full_dump_string( vm );
				fout.close( );
				std::cout << "Saved file to " << fname << "\n\n";
			}
		} else if( current_line == "exit" ) {
			std::cout << "Exiting Debugger\n\n\n";
			return;
		} else if( current_line == "quit" ) {
			std::cout << "Exiting Program\n\n\n";
			exit( EXIT_SUCCESS );
		} else if( current_line.substr( 0, 6 ) == "setip " ) {
			auto new_ip = convert<uint16_t>( current_line.substr( 6 ) );
			assert( new_ip < 32768 );
			std::cout << "Setting instruction ptr to " << new_ip << "\n\n";
			vm.instruction_ptr = new_ip;
		} else {
			std::cout << "ERROR\n\n";
			print_help( );
		}
	}

}
