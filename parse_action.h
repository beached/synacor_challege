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

#include <string>
#include <unordered_map>
#include <vector>


struct parse_action_t {
	// bool - tokenize parameters, std::string - help msg, std::function<...> action callback
//	using action_item_t = std::tuple<bool, std::string, std::function<void( std::vector<std::string> )>>;
	struct action_item_t {
		bool tokenize_parameters;
		std::string help_message;
		std::function<void( std::vector<std::string> )> action;
		action_item_t( ) = default;
		action_item_t( bool TokenizeParameters, std::string HelpMessage, std::function<void( std::vector<std::string> )> Action );
	};
	
	std::unordered_map<std::string, action_item_t> actions;
	std::string separators;

 	parse_action_t( std::initializer_list<std::pair<std::string, action_item_t>> Actions );
// 	parse_action_t( std::string Separators, std::initializer_list<std::pair<std::string, action_item_t>> Actions );


	void parse( std::string str ) const;
};	// struct parse_action_t

std::pair<std::string, parse_action_t::action_item_t> make_action( std::string key, bool tokenize_parameters, std::string help_message, std::function<void( std::vector<std::string> )> action ); 
