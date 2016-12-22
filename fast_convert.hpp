#ifndef FAST_CONVERT_HPP
#define FAST_CONVERT_HPP

#include "astring_view.hpp"

namespace xts
{
	//Assume that the input string have only numeral, no space, no '-', no '+', and doesn't overflow
	std::size_t fast_str_to_uint(astd::string_view str)
	{
		std::size_t result = 0;
		for (char c : str)
		{
			result = result * 10 + (c - '0');
		}
		return result;
	}
}

#endif //FAST_CONVERT_HPP
