#include "functional_algorithm_parameter.h"

#include <algorithm>
#include <vector>
#include <set>
#include <iostream>
#include <list>

namespace 
{
	void fun_print(int elem)
	{
		std::cout << elem << ' ';
	}

	int fun_square(int elem)
	{
		return elem * elem;
	}

	bool isGood(int score)
	{
		return score >= 80;
	}

	bool selfSmaller(int a, int b)
	{
		return (a < b);
	}

	bool selfBigger(int a, int b)
	{
		return (a > b);
	}
}

void TEST::test_foreach()
{
	std::vector<int> v{ 1,2,3,4,5 };

	std::cout << "Use for_each to print v" << std::endl;
	std::for_each(v.begin(), v.end(), fun_print);
	std::cout << std::endl;
}

void TEST::test_transform()
{
	std::set<int> set{ 1,2,3,4,5 };

	std::cout << "set:" << std::endl;
	for (auto const& r : set)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;

	std::cout << "transform set to vector using fun_square and back_inserter" << std::endl;
	std::vector<int> v;
	std::transform(set.cbegin(), set.cend(), std::back_inserter(v), fun_square);

	for (auto const& r : v)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;
}

void TEST::test_find_if()
{
	std::list<int> list{ 67,88,82,75,90,92,66 };

	std::cout << "list: " << std::endl;
	for (auto const& r : list)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;

	std::cout << "finding items bigger than 80" << std::endl;
	std::list<int>::const_iterator it = list.cbegin();
	while (1)
	{
		it = std::find_if(it, list.cend(), isGood);
		if (it != list.cend())
		{
			std::cout << *it << " ";
			++it;
		}
		else
			break;
	}
}



void TEST::test_sort_self_defined()
{
	std::vector<int> v{ 1,4,2,9,6,8,5 };

	std::cout << "v: " << std::endl;
	for (auto const& r : v)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;

	std::cout << "sort v using selfSmaller" << std::endl;
	std::sort(v.begin(), v.end(), selfSmaller);

	std::cout << "v: " << std::endl;
	for (auto const& r : v)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;

	std::cout << "sort v using selfBigger" << std::endl;
	std::sort(v.begin(), v.end(), selfBigger);

	std::cout << "v: " << std::endl;
	for (auto const& r : v)
	{
		std::cout << r << " ";
	}
	std::cout << std::endl;
}