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

#include "file_helper.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility/string_ref.hpp>
#include <string>

namespace {
	std::string epoch( ) {
		using boost::posix_time::ptime;
		using boost::posix_time::time_from_string;
		static ptime const epoch = time_from_string( "1970-01-01 00:00:00.000" );
		ptime other = time_from_string( "2011-08-09 17:27:00.000" );

		return std::to_string( (other - epoch).total_milliseconds( ) );
	}
}

std::string generate_unique_file_name( boost::string_ref prefix, boost::string_ref suffix, boost::string_ref extension ) {
	std::string result = prefix.to_string() + epoch( ) + suffix.to_string() + "." + extension.to_string();
	return result;
}

