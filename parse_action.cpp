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

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_ref.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "parse_action.h"
#include <iostream>

parse_action_t::action_item_t::action_item_t( bool TokenizeParameters, std::string HelpMessage, action_t Action ):
	tokenize_parameters( TokenizeParameters ),
	help_message( std::move( HelpMessage ) ),
	action( Action ) { }

std::pair<std::string, parse_action_t::action_item_t> make_action( std::string key, bool tokenize_parameters, std::string help_message, parse_action_t::action_item_t::action_t action ) {
	return std::make_pair( key, parse_action_t::action_item_t{ tokenize_parameters, help_message, action } );
}

parse_action_t::parse_action_t( std::initializer_list<std::pair<std::string, parse_action_t::action_item_t>> Actions ): actions( ), separators( "\t " ), last_line( ) {
	for( auto const & item: Actions ) {
		actions.emplace( item.first, item.second );
	}

}

bool parse_action_t::help() const {
	std::cout << "Possible commands\n------------------\n\n";
	for( auto const & item : actions ) {
		std::cout << item.first << " " << item.second.help_message << "\n";
	}
	std::cout << std::endl;
	return true;
}

bool parse_action_t::parse( std::string str ) {
	if( "r" == str ) {
		str = last_line;
	} else {
		last_line = str;
	}
	std::vector<std::string> tokens;
	boost::split( tokens, str, boost::is_any_of( separators ) );
	auto action_it = actions.find( tokens[0] );
	if( actions.end( ) == action_it ) {
		return help( );		// TODO define error action
	}
	auto const & action = action_it->second;
	if( action.tokenize_parameters ) {
		tokens.erase( tokens.begin( ) );
	} else {
		tokens.resize( 1 );
		tokens[0] = str.substr( tokens[0].size( ) + 1 );
	}
	return action.action( tokens );
}

