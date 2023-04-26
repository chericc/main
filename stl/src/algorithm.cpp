#include "algorithm.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iterator>
#include <deque>
#include <random>

#include "../utility.h"

void TEST::test_algorithm_nonmodifying()
{
	/* for_each */
	{
		std::cout << "TEST: for_each" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "original v");

		std::cout << "for_each print" << std::endl;
		std::for_each(v.cbegin(), v.cend(), [](const int &ref) {std::cout << ref << " "; });
		std::cout << std::endl;

		std::cout << "for_each modify elements" << std::endl;
		std::for_each(v.begin(), v.end(), [](int &ref) {ref += 1; });
		PRINT_ELEMENTES(v, "after modifying");

		std::cout << std::endl;
	}
	/* count */
	{
		std::cout << "TEST: count" << std::endl;
		std::vector<int> v{ 1,3,3,4,5,6,6 };
		PRINT_ELEMENTES(v, "original vector");

		std::cout << "count 3" << std::endl;
		std::cout << std::count(v.cbegin(), v.cend(), 3) << std::endl;

		std::cout << "count_if over 3 using lambda" << std::endl;
		std::cout << std::count_if(v.cbegin(), v.cend(), [](const int &ref)->bool {return ref > 3; });

		std::cout << std::endl;
	}
	/* min_element & max_element & minmax_element */
	{
		std::cout << "TEST: min_element" << std::endl;

		std::vector<int> v{ 5,3,8,8,6 };
		PRINT_ELEMENTES(v, "original vector");
		std::cout << "*min_element(v)=" << *std::min_element(v.cbegin(), v.cend()) << std::endl;
		std::cout << "*min_element(v, comp)=" << *std::min_element(v.cbegin(), v.cend(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; }) << std::endl;
		std::cout << "*max_element(v)=" << *std::max_element(v.cbegin(), v.cend()) << std::endl;
		std::cout << "*max_element(v, comp)=" << *std::max_element(v.cbegin(), v.cend(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; }) << std::endl;
		auto it_minmax_1 = std::minmax_element(v.cbegin(), v.cend());
		auto it_minmax_2 = std::minmax_element(v.cbegin(), v.cend(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; });
		std::cout << "*minmax_element(v).first=" << *(it_minmax_1.first) << std::endl;
		std::cout << "*minmax_element(v).second=" << *(it_minmax_1.second) << std::endl;
		std::cout << "*minmax_element(v, comp).first=" << *(it_minmax_2.first) << std::endl;
		std::cout << "*minmax_element(v, comp).second=" << *(it_minmax_2.second) << std::endl;
		std::cout << std::endl;
	}
	/* find & find_if & find_if_not */
	{
		std::cout << "TEST: find" << std::endl;

		std::vector<int> v{ 5,3,8,8,6 };
		PRINT_ELEMENTES(v, "original vector");
		std::cout << "*find(v,3)=" << *std::find(v.cbegin(), v.cend(), 3) << std::endl;
		std::cout << "*find_if(v, >5)=" << *std::find_if(v.cbegin(), v.cend(), [](const int &ref)->bool {return ref > 5; }) << std::endl;
		std::cout << "*find_if_not(v, >5)=" << *std::find_if_not(v.cbegin(), v.cend(), [](const int &ref)->bool {return ref > 5; }) << std::endl;
		std::cout << std::endl;
	}
	/* search_n */
	{
		std::cout << "TEST: search_n" << std::endl;

		std::vector<int> v{ 4,5,7,7,7,5,2,2,3 };
		PRINT_ELEMENTES(v, "original vector");
		std::cout << "*search_n(v.cbegin(), v.cend(), 2, 7)=" << *std::search_n(v.cbegin(), v.cend(), 2, 7) << std::endl;
		std::cout << "*search_n(v.cbegin(), v.cend(), 2, 0, (elem+value)<5 = " <<
			*std::search_n(v.cbegin(), v.cend(), 2, 0, [](const int &ref_elem, const int &ref_value)->bool {return ref_elem + ref_value < 5; }) \
			<< std::endl;
		std::cout << std::endl;
	}
	/* search */
	{
		std::cout << "TEST: search" << std::endl;

		std::vector<int> v{ 1,6,7,8,2,3,4,9 };
		std::vector<int> v2{ 8,2,3 };
		PRINT_ELEMENTES(v, "v: ");
		PRINT_ELEMENTES(v2, "v2: ");
		std::cout << "*search(v.cbegin(), v.cend(), v2.cbegin(), v2.cend())=" <<
			*std::search(v.cbegin(), v.cend(), v2.cbegin(), v2.cend()) << std::endl;
		v2 = { 7,1,2 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "*search(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), op(ele-1==value))=" <<
			*std::search(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem - 1 == ref_value; })
			<< std::endl;
		std::cout << std::endl;
	}
	/* find_end */
	{
		std::cout << "TEST: find_end" << std::endl;

		std::cout << "find_end finds the last section while search finds the first" << std::endl;
		std::cout << std::endl;
	}
	/* find_first_of */
	{
		std::cout << "TEST: find_first_of" << std::endl;

		std::vector<int> v{ 23,67,53,88,1,78 };
		std::vector<int> v2{ 99,14,55,53 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");

		std::cout << "*find_first_of(v.cbegin(), v.cend(), v2.cbegin(), v2.cend())=" <<
			*std::find_first_of(v.cbegin(), v.cend(), v2.cbegin(), v2.cend()) << std::endl;
		std::cout << "*find_first_of(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), op(a + b == 100))=" <<
			*std::find_first_of(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem + ref_value == 100; }) << std::endl;
		std::cout << std::endl;
	}
	/* adjacent_find */
	{
		std::cout << "TEST: adjacent_find" << std::endl;

		std::vector<int> v{ 1,3,5,7,7,9,10 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "*adjacent_find(v.cbegin(), v.cend())=" <<
			*std::adjacent_find(v.cbegin(), v.cend()) << std::endl;
		std::cout << "*adjacent_find(v.cbegin(), v.cend(), op(a + 1 == b))=" <<
			*std::adjacent_find(v.cbegin(), v.cend(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem + 1 == ref_value; }) << std::endl;
		std::cout << std::endl;
	}
	/* equal */
	{
		std::cout << "TEST: equal" << std::endl;

		std::vector<int> v{ 0,1,2,3,4,5,6,7,8,9 };
		std::vector<int> v2{ 0,1,2,3,4,5,6,7,8,9,10,11 };
		std::vector<int> v3{ 1,2,3,4,5,6,7,8,9,10 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		PRINT_ELEMENTES(v3, "v3:");

		std::cout << "equal(v.cbegin(), v.cend(), v1.cbegin())=" <<
			std::equal(v.cbegin(), v.cend(), v2.cbegin()) << std::endl;
		std::cout << "equal(v.cbegin(), v.cend(), v3.cbegin(), op(a+1==b))=" <<
			std::equal(v.cbegin(), v.cend(), v3.cbegin(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem + 1 == ref_value; }) << std::endl;
		std::cout << std::endl;
	}
	/* is_permutation */
	{
		std::cout << "TEST: is_permutation" << std::endl;

		std::vector<int> v{ 1,3,5,7,9 };
		std::vector<int> v2{ 3,5,1,9,7 };
		std::vector<int> v3{ 10,8,6,4,2 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "is_permutation(v.cbegin(), v.cend(), v2.cbegin())=" <<
			std::is_permutation(v.cbegin(), v.cend(), v2.cbegin()) << std::endl;
		std::cout << "is_permutation(v.cbegin(), v.cend(), v3.cbegin(), op(a+b==11))=" <<
			std::is_permutation(v.cbegin(), v.cend(), v3.cbegin(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem + ref_value == 11; }) << std::endl;
		std::cout << std::endl;
	}
	/* mismatch */
	{
		std::cout << "TEST: mismatch" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		std::vector<int> v2{ 1,2,4,5,6 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "*mismatch(v.cbegin(), v.cend(), v2.cbegin()).first=" <<
			*std::mismatch(v.cbegin(), v.cend(), v2.cbegin()).first << std::endl;
		std::cout << "*mismatch(v.cbegin(), v.cend(), v2.cbegin()).second=" <<
			*std::mismatch(v.cbegin(), v.cend(), v2.cbegin()).second << std::endl;
		std::cout << "*mismatch(v.cbegin(), v.cend(), v2.cbegin(), op(a==b)).first=" <<
			*std::mismatch(v.cbegin(), v.cend(), v2.cbegin(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem == ref_value; }).first << std::endl;
		std::cout << "*mismatch(v.cbegin(), v.cend(), v2.cbegin(), op(a==b)).second=" <<
			*std::mismatch(v.cbegin(), v.cend(), v2.cbegin(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem == ref_value; }).second << std::endl;
		std::cout << std::endl;
	}
	/* lexicographical_compare */
	{
		std::cout << "TEST: lexicographical_compare" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		std::vector<int> v2{ 1,2,3,4,5,6 };
		std::vector<int> v3{ 1,2,3,4,6 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "lexicographical_compare(v.cbegin(), v.cend(), v2.cbegin(), v2.cend())=" <<
			std::lexicographical_compare(v.cbegin(), v.cend(), v2.cbegin(), v2.cend()) << std::endl;
		std::cout << "lexicographical_compare(v.cbegin(), v.cend(), v3.cbegin(), v3.cend())=" <<
			std::lexicographical_compare(v.cbegin(), v.cend(), v3.cbegin(), v3.cend()) << std::endl;
		std::cout << "lexicographical_compare(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), op(a<b))=" <<
			std::lexicographical_compare(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), 
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem < ref_value; }) << std::endl;
		std::cout << "lexicographical_compare(v.cbegin(), v.cend(), v3.cbegin(), v3.cend(), op(a<b))=" <<
			std::lexicographical_compare(v.cbegin(), v.cend(), v3.cbegin(), v3.cend(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem < ref_value; }) << std::endl;
		std::cout << std::endl;
	}
	/* is_sorted */
	{
		std::cout << "TEST: is_sorted" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "is_sorted(v.cbegin(), v.cend())=" <<
			std::is_sorted(v.cbegin(), v.cend()) << std::endl;

		std::vector<int> v2{ 5,4,3,2,1 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "is_sorted(v2.cbegin(), v2.cend())=" <<
			std::is_sorted(v2.cbegin(), v2.cend()) << std::endl;
		std::cout << "is_sorted(v2.cbegin(), v2.cend(), op(a>b))=" <<
			std::is_sorted(v2.cbegin(), v2.cend(),
				[](const int &ref_elem, const int &ref_value)->bool {return ref_elem > ref_value; }) << std::endl;

		std::cout << std::endl;
	}
	/* is_sorted_until */
	{
		std::cout << "TEST: is_sorted_until" << std::endl;

		std::vector<int> v{ 1,2,4,3,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "*is_sorted_until(v.cbegin(), v.cend())=" <<
			*std::is_sorted_until(v.cbegin(), v.cend()) << std::endl;
		std::vector<int> v2{ 5,3,4,2,1 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "*is_sorted_until(v2.cbegin(), v2.cend(), op(a>b))=" <<
			*std::is_sorted_until(v2.cbegin(), v2.cend(),
				[](const int &ref_a, const int ref_b)->bool {return ref_a > ref_b; }) << std::endl;
		std::cout << std::endl;
	}
	/* is_partitioned */
	{
		std::cout << "TEST: is_partitioned" << std::endl;

		std::vector<int> v{ 3,1,5,7,3,4,8,6,2 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "is_partitioned(v.cbegin(), v.cend(), op(a%2 != 0))=" <<
			std::is_partitioned(v.cbegin(), v.cend(),
				[](const int &ref)->bool {return ref % 2 != 0; }) << std::endl;
		std::cout << std::endl;
	}
	/* partition_point */
	{
		std::cout << "TEST: partition_point" << std::endl;

		std::vector<int> v{ 1,3,5,7,2,4,6,8 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "*partition_point(v.cbegin(), v.cend(), op)=" <<
			*std::partition_point(v.cbegin(), v.cend(),
				[](const int &ref)->bool {return ref % 2 != 0; }) << std::endl;
		std::cout << std::endl;
	}
	/* is_heap, is_heap_until */
	{
		std::cout << "TEST: is_heap, is_heap_until" << std::endl;

		std::vector<int> v{ 5,3,4,1,2,2,1 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "is_heap(v.cbegin(), v.cend())=" <<
			std::is_heap(v.cbegin(), v.cend()) << std::endl;
		std::cout << "is_heap(v.cbegin(), v.cend(), op(a<b))=" <<
			std::is_heap(v.cbegin(), v.cend(),
				[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; }) << std::endl;
		std::vector<int> v2{ 5,3,4,1,2,5,2 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "is_heap(v2.cbegin(), v2.cend())=" <<
			std::is_heap(v2.cbegin(), v2.cend()) << std::endl;
		std::cout << "*is_heap_until(v2.cbegin(), v2.cend())=" <<
			*std::is_heap_until(v2.cbegin(), v2.cend()) << std::endl;
		std::cout << "*is_heap_until(v2.cbegin(), v2.cend(), op(a<b))=" <<
			*std::is_heap_until(v2.cbegin(), v2.cend(),
				[](const int &ref_a, const int ref_b)->bool {return ref_a < ref_b; }) << std::endl;
		std::cout << std::endl;
	}
	/* all_of, any_of, none_of */
	{
		std::cout << "TEST: all_of, any_of, none_of" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "all_of(v.cbegin(), v.cend(), op(a<100))=" <<
			std::all_of(v.cbegin(), v.cend(),
				[](const int &ref)->bool {return ref < 100; }) << std::endl;
		std::cout << "any_of(v.cbegin(), v.cend(), op(a<2))=" <<
			std::any_of(v.cbegin(), v.cend(),
				[](const int &ref)->bool {return ref < 2; }) << std::endl;
		std::cout << "none_of(v.cbegin(), v.cend(), op(a<0))=" <<
			std::none_of(v.cbegin(), v.cend(),
				[](const int &ref)->bool {return ref < 0; }) << std::endl;
		std::cout << std::endl;
	}
} 

void TEST::test_algorithm_modifying()
{
	/* copy */
	{
		std::cout << "TEST: copy" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		
		PRINT_ELEMENTES(v, "v:");

		std::vector<int> v1;
		std::cout << "copy(v.cbegin(), v.cend(), back_inserter(v1))" << std::endl;
		std::copy(v.cbegin(), v.cend(), std::back_inserter(v1));
		PRINT_ELEMENTES(v1, "v1");
		std::cout << std::endl;
	}
	/* copy_if */
	{
		std::cout << "TEST: copy_if" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v1;
		std::cout << "copy(v.cbegin(), v.cend(), back_inserted(v1), op(v<4))" << std::endl;
		std::copy_if(v.cbegin(), v.cend(), std::back_inserter(v1),
			[](const int &ref)->bool {return ref < 4; });
		std::cout << std::endl;
	}
	/* copy_n */
	{
		std::cout << "TEST: copy_n" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2;
		std::cout << "copy_n(v.cbegin(), v.size()-1, back_inserter(v2))" << std::endl;
		std::copy_n(v.cbegin(), v.size() - 1, std::back_inserter(v2));
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* copy_backword */
	{
		std::cout << "TEST: copy_backward" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2(v.size());
		std::cout << "copy_backward(v.cbegin(), v.cend(), v2.end())" << std::endl;
		std::copy_backward(v.cbegin(), v.cend(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* move, move_backward */
	{
		std::cout << "TEST: move" << std::endl;
		std::vector<std::string> v{ "this", "is", "amazing!" };
		PRINT_ELEMENTES(v, "v:");
		std::vector<std::string> v2(v.size());
		std::cout << "move(v.cbegin(), v.cend(), v2.begin())" << std::endl;
		std::move(v.cbegin(), v.cend(), v2.begin());
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<std::string> v3{ "this", "is", "amazing", "too!" };
		PRINT_ELEMENTES(v3, "v3:");
		std::vector<std::string> v4(v3.size());
		std::cout << "move_backward(v3.cbegin(), v3.cend(), v4.back())" << std::endl;
		std::move_backward(v3.cbegin(), v3.cend(), v4.end());
		PRINT_ELEMENTES(v4, "v4:");
		std::cout << std::endl;
	}
	/* transform */
	{
		std::cout << "TEST: transform" << std::endl;

		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2;
		std::cout << "transform(v.cbegin(), v.cend(), back_inserter(v2), op(v+1))" << std::endl;
		std::transform(v.cbegin(), v.cend(), std::back_inserter(v2),
			[](const int &ref)->int {return ref + 1; });
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v3, "v3:");
		std::vector<int> v4{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5;
		std::cout << "transform(v3.cbegin(), v3.cend(), v4.cbegin(), back_inserter(v5), op(a+b))" << std::endl;
		std::transform(v3.cbegin(), v3.cend(), v4.cbegin(), std::back_inserter(v5),
			[](const int &ref_a, const int ref_b)->int {return ref_a + ref_b; });
		PRINT_ELEMENTES(v5, "v5:");

		std::cout << std::endl;
	}
	/* swap_ranges */
	{
		std::cout << "TEST: swap_ranges" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		std::vector<int> v2{ 2,3,4,5,6 };
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "swap_ranges(v.begin(), v.end(), v2.begin())" << std::endl;
		std::swap_ranges(v.begin(), v.end(), v2.begin());
		PRINT_ELEMENTES(v, "v:");
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* fill, fill_n */
	{
		std::cout << "TEST: fill, fill_n" << std::endl;
		std::vector<int> v(5);
		std::cout << "fill(v.begin(), v.end(), 12)" << std::endl;
		std::fill(v.begin(), v.end(), 12);
		PRINT_ELEMENTES(v, "v:");
		std::cout << "fill_n(v.begin(), v.size(), 13)" << std::endl;
		std::fill_n(v.begin(), v.size(), 13);
		PRINT_ELEMENTES(v, "v:");
		std::cout << std::endl;
	}
	/* generate, generate_n */
	{
		std::cout << "TEST: generate, generate_n" << std::endl;
		int count = 0;
		std::vector<int> v(5);
		std::cout << "generate(v.begin(), v.end(), op(++count%3))" << std::endl;
		std::generate(v.begin(), v.end(), [&count]()->int {return ++count % 3; });
		PRINT_ELEMENTES(v, "v:");

		std::vector<int> v2(5);
		std::cout << "generate_n(v.begin(), v.end(), op(++count%3))" << std::endl;
		std::generate_n(v2.begin(), v2.size(), [&count]()->int {return ++count % 3; });
		PRINT_ELEMENTES(v2, "v2:");

		std::cout << std::endl;
	}
	/* iota */
	{
		std::cout << "TEST: iota" << std::endl;
		std::vector<int> v(5);
		std::cout << "iota(v.begin(), v.end(), 12)" << std::endl;
		std::iota(v.begin(), v.end(), 12);
		PRINT_ELEMENTES(v, "v:");
		std::cout << std::endl;
	}
	/* replace, replace_if */
	{
		std::cout << "TEST: replace, replace_if" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		std::cout << "replace(v.begin(), v.end(), 3, 12)" << std::endl;
		std::replace(v.begin(), v.end(), 3, 12);
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5 };
		std::cout << "replace_if(v2.begin(), v2.end(), op(a>2), 12)" << std::endl;
		std::replace_if(v2.begin(), v2.end(),
			[](const int &ref)->bool {return ref > 2; }, 12);
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* replace_copy, replace_copy_if */
	{
		std::cout << "TEST: replace_copy, replace_copy_if" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		std::vector<int> v2;
		std::vector<int> v3;
		std::cout << "replace_copy(v.cbegin(), v.cend(), back_inserter(v2), 3, 12)" << std::endl;
		std::replace_copy(v.cbegin(), v.cend(), std::back_inserter(v2), 3, 12);
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "replace_copy(v.cbegin(), v.cend(), back_inserter(v3), op(a>2), 12)" << std::endl;
		std::replace_copy_if(v.cbegin(), v.cend(), std::back_inserter(v3),
			[](const int &ref)->bool {return ref > 2; }, 12);
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
}

void TEST::test_algorithm_removing()
{
	std::cout << "TEST algorithm removing" << std::endl;

	/* remove, remove_if */
	{
		std::cout << "TEST: remove, remove_if" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "remove(v.begin(), v.end(), 3)" << std::endl;
		std::remove(v.begin(), v.end(), 3);
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "remove_if(v.begin(), v.end(), op(a>2))" << std::endl;
		std::remove_if(v.begin(), v.end(),
			[](const int &ref)->bool {return ref > 2; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* remove_copy, remove_copy_if */
	{
		std::cout << "TEST: remove_copy, remove_copy_if" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2;
		std::vector<int> v3;
		std::cout << "remove_copy(v.cbegin(), v.cend(), insert_iterator(v2), 3)" << std::endl;
		std::remove_copy(v.cbegin(), v.cend(), std::back_inserter(v2), 3);
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "remove_copy_if(v.cbegin(), v.cend(), back_inserter(v3), op(>2))" << std::endl;
		std::remove_copy_if(v.cbegin(), v.cend(), std::back_inserter(v3),
			[](const int &ref)->bool {return ref > 2; });
		PRINT_ELEMENTES(v3, "v3;");
		std::cout << std::endl;
	}
	/* unique */
	{
		std::cout << "TEST: unique" << std::endl;
		std::vector<int> v{ 1,2,2,3,3,3,4,4,4,4,5,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "unique(v.begin(), v.end())" << std::endl;
		std::unique(v.begin(), v.end());
		PRINT_ELEMENTES(v, "v:");
		std::cout << "unique(v.begin(), v.end(), op(a==b))" << std::endl;
		std::vector<int> v2{ 1,2,2,3,3,3,4,4,4,4,5,5 };
		std::unique(v2.begin(), v2.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a == ref_b; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* unique_copy */
	{
		std::cout << "TEST: uinque_copy" << std::endl;
		std::vector<int> v{ 1,2,2,3,3,3,4,4,4,4,5,5 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2;
		std::vector<int> v3;
		std::cout << "unique(v.cbegin(), v.cend(), back_inserter(v2))" << std::endl;
		std::unique_copy(v.cbegin(), v.cend(), std::back_inserter(v2));
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "unique(v.cbegin(), v.cend(), back_inserter(v3), op(a==b))" << std::endl;
		std::unique_copy(v.cbegin(), v.cend(), std::back_inserter(v3),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a == ref_b; });
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
}

void TEST::test_algorithm_mutating()
{
	std::cout << "TEST: mutating algorithm" << std::endl;
	/* reverse, reverse_copy */
	{
		std::cout << "TEST: reverse, reverse_copy" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "reverse(v.begin(), v.end())" << std::endl;
		std::reverse(v.begin(), v.end());
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;
		std::cout << "reverse_copy(v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::reverse_copy(v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
	/* rotate, rotate_copy */
	{
		std::cout << "TEST: rotate, rotate_copy" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "rotate(v.begin(), v.begin()+2, v.end())" << std::endl;
		std::rotate(v.begin(), v.begin() + 2, v.end());
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;
		std::cout << "rotate_copy(v2.cbegin(), v2.cbegin()+2, v2.cend(), back_inserter(v3))" << std::endl;
		std::rotate_copy(v2.cbegin(), v2.cbegin() + 2, v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
	/* next_permutation */
	{
		std::cout << "TEST: next_premutation" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "next_permutation(v.begin(), v.end())" << std::endl;
		std::next_permutation(v.begin(), v.end());
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "next_permutation(v.begin(), v.end(), op(a<b))" << std::endl;
		std::next_permutation(v.begin(), v.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* prev_permutation */
	{		
		std::cout << "TEST: next_premutation" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "prev_permutation(v.begin(), v.end())" << std::endl;
		std::prev_permutation(v.begin(), v.end());
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "prev_permutation(v.begin(), v.end(), op(a<b))" << std::endl;
		std::prev_permutation(v.begin(), v.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a < ref_b; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* shuffle, random_shuffle */
	{
		std::cout << "TEST: shuffle" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "shuffle(v.begin(), v.end(), default_random_engine)" << std::endl;
		std::shuffle(v.begin(), v.end(), std::default_random_engine());
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "shuffle_random(v.begin(), v.end())" << std::endl;
		std::random_shuffle(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* partition, stable_partition */
	{
		std::cout << "TEST: partition, stable_partition" << std::endl;
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "partition(v.begin(), v.end(), op(a>3))" << std::endl;
		std::partition(v.begin(), v.end(), [](const int &ref)->bool {return ref > 3; });
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "stable_partition(v.begin(), v.end(), op(a>3))" << std::endl;
		std::stable_partition(v2.begin(), v2.end(), [](const int &ref)->bool {return ref > 3; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << std::endl;
	}
	/* partition_copy */
	{
		std::cout << "TEST: partition_copy" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2;
		std::vector<int> v3;
		std::cout << "partition_copy(v.cbegin(), v.cend(), back_inserter(v2), back_inserter(v3), op(a%2 == 0)" << std::endl;
		std::partition_copy(v.cbegin(), v.cend(), std::back_inserter(v2), std::back_inserter(v3),
			[](const int &ref)->bool {return ref % 2 == 0; });
		PRINT_ELEMENTES(v2, "v2:");
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
}

void TEST::test_algorithm_sorting()
{
	std::cout << "TEST: sorting algorithm" << std::endl;
	/* sort, stable_sort */
	{
		std::cout << "TEST: sort, stable_sort" << std::endl;
		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };

		std::vector<int> v2 = v;
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "sort(v2.begin(), v2.end())" << std::endl;
		std::sort(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3 = v;
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "sort(v3.begin(), v3.end(), op(a>b)" << std::endl;
		std::sort(v3.begin(), v3.end(), [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4 = v;
		PRINT_ELEMENTES(v4, "v4:");
		std::cout << "stable_sort(v4.begin(), v4.end())" << std::endl;
		std::stable_sort(v4.begin(), v4.end());
		PRINT_ELEMENTES(v4, "v4:");

		std::vector<int> v5 = v;
		PRINT_ELEMENTES(v5, "v5:");
		std::cout << "stable_sort(v5.begin(), v5.end(), op(a>b))" << std::endl;
		std::stable_sort(v5.begin(), v5.end(), [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v5, "v5:");

		std::cout << std::endl;
	}
	/* partial_sort, partial_sort_copy */
	{
		std::cout << "TEST: partial_sort" << std::endl;
		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };

		std::vector<int> v2 = v;
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "partial_sort(v2.begin(), v2.begin() + 5, v2.end())" << std::endl;
		std::partial_sort(v2.begin(), v2.begin() + 5, v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3 = v;
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "partial_sort(v3.begin(), v3.begin() + 5, v3.end(), op(a>b))" << std::endl;
		std::partial_sort(v3.begin(), v3.begin() + 5, v3.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;

		std::vector<int> v4 = v;
		std::vector<int> v5(v4.size());
		PRINT_ELEMENTES(v4, "v4:");
		std::cout << "partial_sort_copy(v4.cbegin(), v4.cend(), v5.begin(), v5.end())" << std::endl;
		std::partial_sort_copy(v4.cbegin(), v4.cend(), v5.begin(), v5.end());
		PRINT_ELEMENTES(v5, "v5:");

		std::vector<int> v6 = v;
		std::vector<int> v7(v6.size());
		PRINT_ELEMENTES(v6, "v4:");
		std::cout << "partial_sort_copy(v4.cbegin(), v4.cend(), v5.begin(), v5.end(), op(a>b))" << std::endl;
		std::partial_sort_copy(v6.cbegin(), v6.cend(), v7.begin(), v7.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v7, "v7:");
		std::cout << std::endl;
	}
	/* nth_element */
	{
		std::cout << "TEST: nth_element" << std::endl;
		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };

		std::vector<int> v2 = v;
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "nth_element(v2.begin(), v2.begin()+4, v2.end())" << std::endl;
		std::nth_element(v2.begin(), v2.begin() + 4, v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3 = v;
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "nth_element(v3.begin(), v3.begin()+4, v3.end(), op(a>b))" << std::endl;
		std::nth_element(v3.begin(), v3.begin() + 4, v3.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
	/* heap */
	{
		std::cout << "TEST: heap" << std::endl;
		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };
		
		std::vector<int> v2 = v;
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "make_heap(v2.begin(), v2.end())" << std::endl;
		std::make_heap(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3 = v;
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << "make_heap(v3.begin(), v3.end(), op(a>b))" << std::endl;
		std::make_heap(v3.begin(), v3.end(), [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });

		std::cout << "v2.push_back(13)" << std::endl;
		v2.push_back(13);
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "push_heap(v2.begin(), v2.end())" << std::endl;
		std::push_heap(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::cout << "pop_heap(v2.begin(), v2.end())" << std::endl;
		std::pop_heap(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");

		std::cout << "sort_heap(v3.begin(), v3.end())" << std::endl;
		std::sort_heap(v3.begin(), v3.end(),
			[](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v3, "v3:");

		std::cout << std::endl;
	}
}

void TEST::test_algorithm_sortedrange()
{
	std::cout << "TEST sorted range" << std::endl;
	/* binnary_search */
	{
		std::cout << "TEST: binary_search" << std::endl;

		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };

		std::vector<int> v2 = v;
		std::sort(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "binary_search(v.cbegin(), v.cend(), 5)=" << std::binary_search(v.cbegin(), v.cend(), 5) << std::endl;
		std::cout << "binary_search(v.cbegin(), v.cend(), 100)=" << std::binary_search(v.cbegin(), v.cend(), 100) << std::endl;

		std::sort(v2.begin(), v2.end(), [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; });
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "binary_search(v2.cbegin(), v2.cend(), 5, op(a>b))=" <<
			std::binary_search(v2.cbegin(), v2.cend(), 5, [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; }) << std::endl;
		std::cout << "binary_search(v2.cbegin(), v2.cend(), 100, op(a>b))=" <<
			std::binary_search(v2.cbegin(), v2.cend(), 100, [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; }) << std::endl;
		std::cout << std::endl;
	}
	/* includes */
	{
		std::cout << "TEST: includes" << std::endl;
		std::vector<int> v{ 4,8,1,2,5,6,3,9,7 };

		std::vector<int> v2 = v;
		sort(v2.begin(), v2.end());
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3{ 3,5,8 };
		PRINT_ELEMENTES(v3, "v3:");
		std::vector<int> v4{ 8,5,3 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 1,3,11 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6{ 12,3,1 };
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << "includes(v2.cbegin(), v2.cend(), v3.cbegin(), v3.cend())=" <<
			std::includes(v2.cbegin(), v2.cend(), v3.cbegin(), v3.cend()) << std::endl;

		std::cout << "includes(v2.cbegin(), v2.cend(), v5.cbegin(), v5.cend())=" <<
			std::includes(v2.cbegin(), v2.cend(), v5.cbegin(), v5.cend()) << std::endl;
		
		auto reverse_com = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };
		sort(v2.begin(), v2.end(), reverse_com);
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "includes(v2.cbegin(), v2.cend(), v4.cbegin(), v4.cend(), op(a>b))=" <<
			std::includes(v2.cbegin(), v2.cend(), v4.cbegin(), v4.cend(), reverse_com) << std::endl;

		std::cout << "includes(v2.cbegin(), v2.cend(), v6.cbegin(), v6.cend(), op(a>b))=" <<
			std::includes(v2.cbegin(), v2.cend(), v6.cbegin(), v6.cend(), reverse_com) << std::endl;

		std::cout << std::endl;
	}
	/* lower_bound, upper_bound */
	{
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");

		std::cout << "*lower_bound(v.cbegin(), v.cend(), 5)=" <<
			*std::lower_bound(v.cbegin(), v.cend(), 5) << std::endl;
		std::cout << "*upper_bound(v.cbegin(), v.cend(), 5)=" <<
			*std::upper_bound(v.cbegin(), v.cend(), 5) << std::endl;

		auto reverse_com = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };
		sort(v.begin(), v.end(), reverse_com);

		PRINT_ELEMENTES(v, "v:");
		std::cout << "*lower_bound(v.cbegin(), v.cend(), 5, reverse_com)=" <<
			*std::lower_bound(v.cbegin(), v.cend(), 5, reverse_com) << std::endl;
		std::cout << "*upper_bound(v.cbegin(), v.cend(), 5, reverse_com)=" <<
			*std::upper_bound(v.cbegin(), v.cend(), 5, reverse_com) << std::endl;

		std::cout << std::endl;
	}
	/* equal_range */
	{
		std::cout << "TEST: equal_range" << std::endl;
		std::vector<int> v{ 1,3,3,4,5,5,5,6,7,8 };
		PRINT_ELEMENTES(v, "v:");
		auto range = std::equal_range(v.cbegin(), v.cend(), 5);
		std::cout << "*equal_range(v.cbegin(), v.cend(), 5).first=" << *range.first << std::endl;
		std::cout << "*equal_range(v.cbegin(), v.cend(), 5).second=" << *range.second << std::endl;

		auto reverse_comp = [](const int &ref_a, const int ref_b)->bool {return ref_a > ref_b; };
		std::sort(v.begin(), v.end(), reverse_comp);
		PRINT_ELEMENTES(v, "v:");
		auto reverse_range = std::equal_range(v.cbegin(), v.cend(), 5, reverse_comp);
		std::cout << "*equal_range(v.cbegin(), v.cend(), 5, op(a>b)).first=" << *reverse_range.first << std::endl;
		std::cout << "*equal_range(v.cbegin(), v.cend(), 5, op(a>b)).second=" << *reverse_range.second << std::endl;

		std::cout << std::endl;
	}
	/* merge */
	{
		std::cout << "TEST: merge" << std::endl;
		std::vector<int> v{ 1,2,5,7,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,4,5,7,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;

		std::cout << "merge(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::merge(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4{ 8,7,5,3,1 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 6,4,3,2,1 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6;

		auto reverse_comp = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };

		std::cout << "merge(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), back_inserter(v6), op(a>b))" << std::endl;
		std::merge(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), std::back_inserter(v6), reverse_comp);
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << std::endl;
	}
	/* set_union */
	{
		std::cout << "TEST: set_union" << std::endl;
		std::vector<int> v{ 1,2,5,7,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,4,5,7,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;

		std::cout << "set_union(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::set_union(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4{ 8,7,5,3,1 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 6,4,3,2,1 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6;

		auto reverse_comp = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };

		std::cout << "set_union(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), back_inserter(v6), op(a>b))" << std::endl;
		std::set_union(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), std::back_inserter(v6), reverse_comp);
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << std::endl;
	}
	/* set_intersection */
	{
		std::cout << "TEST: set_intersection" << std::endl;
		std::vector<int> v{ 1,2,5,7,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,4,5,7,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;

		std::cout << "set_intersection(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::set_intersection(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4{ 8,7,5,3,1 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 6,4,3,2,1 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6;

		auto reverse_comp = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };

		std::cout << "set_intersection(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), back_inserter(v6), op(a>b))" << std::endl;
		std::set_intersection(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), std::back_inserter(v6), reverse_comp);
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << std::endl;
	}
	/* set_difference */
	{
		std::cout << "TEST: set_difference" << std::endl;
		std::vector<int> v{ 1,2,5,7,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,4,5,7,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;

		std::cout << "set_difference(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::set_difference(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4{ 8,7,5,3,1 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 6,4,3,2,1 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6;

		auto reverse_comp = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };

		std::cout << "set_difference(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), back_inserter(v6), op(a>b))" << std::endl;
		std::set_difference(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), std::back_inserter(v6), reverse_comp);
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << std::endl;
	}
	/* set_symmetric_difference */
	{
		std::cout << "TEST: set_symmetric_difference" << std::endl;
		std::vector<int> v{ 1,2,5,7,9 };
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 1,4,5,7,9 };
		PRINT_ELEMENTES(v2, "v2:");
		std::vector<int> v3;

		std::cout << "set_symmetric_difference(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), back_inserter(v3))" << std::endl;
		std::set_symmetric_difference(v.cbegin(), v.cend(), v2.cbegin(), v2.cend(), std::back_inserter(v3));
		PRINT_ELEMENTES(v3, "v3:");

		std::vector<int> v4{ 8,7,5,3,1 };
		PRINT_ELEMENTES(v4, "v4:");
		std::vector<int> v5{ 6,4,3,2,1 };
		PRINT_ELEMENTES(v5, "v5:");
		std::vector<int> v6;

		auto reverse_comp = [](const int &ref_a, const int &ref_b)->bool {return ref_a > ref_b; };

		std::cout << "set_symmetric_difference(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), back_inserter(v6), op(a>b))" << std::endl;
		std::set_symmetric_difference(v4.cbegin(), v4.cend(), v5.cbegin(), v5.cend(), std::back_inserter(v6), reverse_comp);
		PRINT_ELEMENTES(v6, "v6:");

		std::cout << std::endl;
	}
	/* inplace_merge */
	{
		std::cout << "TEST: inplace_merge" << std::endl;
		std::vector<int> v{ 1,2,4,5,7,2,4,5,6,8 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "inplace_merge(v.begin(), v.begin() + 5, v.end())" << std::endl;
		std::inplace_merge(v.begin(), v.begin() + 5, v.end());
		PRINT_ELEMENTES(v, "v:");
		std::cout << std::endl;
	}
}

void TEST::test_algorithm_numeric()
{
	std::cout << "TEST: numeric" << std::endl;
	/* accumulate */
	{
		std::vector<int> v{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		std::cout << "accumulate(v.cbegin(), v.cend(), 0)=" <<
			std::accumulate(v.cbegin(), v.cend(), 0) << std::endl;
		std::cout << "accumulate(v.cbegin(), v.cend(), 0, op(return a+b))=" <<
			std::accumulate(v.cbegin(), v.cend(), 0, 
				[](const int &ref_a, const int &ref_b)->int {return ref_a + ref_b; }) << std::endl;
		std::cout << std::endl;
	}
	/* inner_product */
	{
		std::cout << "TEST: inner_product" << std::endl;
		std::vector<int> v{ 1,3};
		PRINT_ELEMENTES(v, "v:");
		std::vector<int> v2{ 2,2 };
		PRINT_ELEMENTES(v2, "v2:");
		std::cout << "inner_product(v.cbegin(), v.cend(), v2.cbegin(), 10)=" <<
			std::inner_product(v.cbegin(), v.cend(), v2.cbegin(), 10) << std::endl;
		std::cout << "inner_product(v.cbegin(), v.cend(), v2.cbegin(), 10, op1(return a+b), op2(return a*b))=" <<
			std::inner_product(v.cbegin(), v.cend(), v2.cbegin(), 10,
				[](const int &ref_a, const int &ref_b)->int {return ref_a + ref_b; },
				[](const int &ref_a, const int &ref_b)->int {return ref_a * ref_b; });
		std::cout << std::endl;
	}
	/* partial_sum */
	{
		std::cout << "TEST: partial_sum" << std::endl;
		std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };
		std::vector<int> v2;
		PRINT_ELEMENTES(v, "v:");
		std::cout << "partial_sum(v.cbegin(), v.cend(), back_inserter(v2))" << std::endl;
		std::partial_sum(v.cbegin(), v.cend(), std::back_inserter(v2));
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3;
		std::cout << "partial_sum(v.cbegin(), v.cend(), back_inserter(v2), op(a+b))" << std::endl;
		std::partial_sum(v.cbegin(), v.cend(), std::back_inserter(v3),
			[](const int &ref_a, const int &ref_b)->int {return ref_a + ref_b; });
		PRINT_ELEMENTES(v3, "v3:");
		std::cout << std::endl;
	}
	/* adjacent_difference */
	{
		std::cout << "TEST: adjacent_difference" << std::endl;
		std::vector<int> v{ 1,2,4,7,11,16 };
		std::vector<int> v2;
		PRINT_ELEMENTES(v, "v:");
		std::cout << "adjacent_differenct(v.cbegin(), v.cend(), back_inserter(v2))" << std::endl;
		std::adjacent_difference(v.cbegin(), v.cend(), std::back_inserter(v2));
		PRINT_ELEMENTES(v2, "v2:");

		std::vector<int> v3;
		std::cout << "adjacent_difference(v.cbegin(), v.cend(), back_inserter(v2), op(a-b))" << std::endl;
		std::adjacent_difference(v.cbegin(), v.cend(), std::back_inserter(v2),
			[](const int &ref_a, const int &ref_b)->int {return ref_a - ref_b; });
		PRINT_ELEMENTES(v3);
		std::cout << std::endl;
	}
}