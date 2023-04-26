/**/

#pragma once

#include <string>
#include <iostream>
#include <map>

namespace TEST
{
	template <typename T>
	void PRINT_ELEMENTES(const T& container, const std::string & optstr = "")
	{
		std::cout << optstr << std::endl;
		for (const auto& r : container)
		{
			std::cout << r << " ";
		}
		std::cout << std::endl;
	}

	template <typename KEY, typename VALUE>
	void PRINT_ELEMENTES(const std::map<KEY, VALUE> container, const std::string & optstr = "")
	{
		std::cout << optstr << std::endl;
		for (const auto& r : container)
		{
			std::cout << r.first << " " << r.second << "     ";
		}
		std::cout << std::endl;
	}
	
	template <typename T, typename _Op>
	void PRINT_ELEMENTES_OP(const T &container, _Op function, const std::string &optstr = "")
	{
		std::cout << optstr << std::endl;
		for (const auto &ref : container)
		{
			std::cout << "OP(" << ref << ")=" << function(ref) << std::endl;
		}
	}

	template <typename T1, typename T2, typename _Op>
	void PRINT_ELEMENTES_OP2(const T1 &c1, const T2 &c2, _Op op, const std::string &optstr = "")
	{
		if (c1.size() != c2.size())
			return;
		std::cout << optstr << std::endl;
		auto cit1 = c1.begin();
		auto cit2 = c2.begin();
		for (; cit1 != c1.end(); cit1++, cit2++)
		{
			std::cout << "OP2(" << *cit1 << "," << *cit2 << ")=" << op(*cit1,*cit2) << std::endl;
		}
	}
}
