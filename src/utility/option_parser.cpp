/*
 * Copyright (c) 2014-2017, Andrzej Rybczak <electricityispower@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <iostream>

#include "utility/option_parser.h"

bool yes_no(const std::string &v)
{
	if (v == "yes")
		return true;
	else if (v == "no")
		return false;
	else
		invalid_value(v);
}

std::vector<int> parse_ratio(const std::string &s)
{
	std::vector<int> ret;
	std::vector<std::string> temp;

	boost::split(temp, s, boost::is_any_of(":"));

	for (auto i : temp)
		ret.push_back(std::stoi(i));

	return ret;
}

bool integers_in_range(const std::string &v)
{
	std::vector<std::string> temp;
	boost::split(temp, v, boost::is_any_of(":"));

	for (auto i : temp)
	{
		try
		{
			int x = std::stoi(i);
		}
		catch(const std::out_of_range &e)
		{
			return false;
		}
	}

	return true;
}

std::vector<int> check_ratio_two(const std::string &v)
{
	boost::regex expr{"^[1-9][0-9]*:[1-9][0-9]*$"};
	if (!boost::regex_match(v, expr))
		invalid_value(v);

	if (integers_in_range(v))
		return parse_ratio(v);
	else
		invalid_value(v);
}

std::vector<int> check_ratio_three(const std::string &v)
{
	boost::regex expr{"^[1-9][0-9]*:[1-9][0-9]*:[1-9][0-9]*$"};
	if (!boost::regex_match(v, expr))
		invalid_value(v);

	if (integers_in_range(v))
		return parse_ratio(v);
	else
		invalid_value(v);
}

////////////////////////////////////////////////////////////////////////////////

bool option_parser::run(std::istream &is, bool ignore_errors)
{
	// quoted value. leftmost and rightmost quotation marks are the delimiters.
	boost::regex quoted("(\\w+)\\h*=\\h*\"(.*)\"[^\"]*");
	// unquoted value. whitespaces get trimmed.
	boost::regex unquoted("(\\w+)\\h*=\\h*(.*?)\\h*");
	boost::smatch match;
	std::string line;
	while (std::getline(is, line))
	{
		if (boost::regex_match(line, match, quoted)
		||  boost::regex_match(line, match, unquoted))
		{
			std::string option = match[1];
			auto it = m_parsers.find(option);
			if (it != m_parsers.end())
			{
				try
				{
					it->second.parse(match[2]);
				}
				catch (std::exception &e)
				{
					std::cerr << "Error while processing option \"" << option
					          << "\": " << e.what() << "\n";
					if (!ignore_errors)
						return false;
				}
			}
			else
			{
				std::cerr << "Unknown option: " << option << "\n";
				if (!ignore_errors)
					return false;
			}
		}
	}
	return true;
}

bool option_parser::initialize_undefined(bool ignore_errors)
{
	for (auto &pp : m_parsers)
	{
		auto &p = pp.second;
		if (!p.used())
		{
			try
			{
				p.parse_default();
			}
			catch (std::exception &e)
			{
				std::cerr << "Error while initializing option \"" << pp.first
				          << "\": " << e.what() << "\n";
				if (ignore_errors)
					return false;
			}
		}
	}
	return true;
}
