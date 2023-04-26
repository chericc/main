
#include "iterator.h"

#include <list>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <deque>
#include <string>

using namespace std;

void TEST::test_insert_iterator()
{
	list<int> l{ 1,3,5,7,6,4,2 };

	cout << "list: " << endl;
	for (auto const& r : l)
	{
		cout << r << " ";
	}
	cout << endl;

	cout << "Using back_inserter to copy l to an empty vector" << endl;
	vector<int> v;
	copy(l.cbegin(), l.cend(), back_inserter(v));

	for (auto const& r : v)
	{
		cout << r << " ";
	}
	cout << endl;
	
	deque<int> q;

	cout << "Using front_inserter to copy l to an empty deque" << endl;
	copy(l.cbegin(), l.cend(), front_inserter(q));

	for (auto const& r : q)
	{
		cout << r << " ";
	}
	cout << endl;
}

void TEST::test_stream_iterator()
{
	vector<string> strs;

	cout << "input, Ctrl Z to end" << endl;
	copy(istream_iterator<string>(cin),
		istream_iterator<string>(),
		back_inserter(strs));
	cout << "input end" << endl;

	sort(strs.begin(), strs.end());
	unique_copy(strs.cbegin(), strs.cend(),
		ostream_iterator<string>(cout, "\n"));
}

void TEST::test_reverse_iterator()
{
	vector<int> v{ 1,2,3,4,5 };
	cout << "original vector" << endl;
	for (auto const& r : v)
	{
		cout << r << " ";
	}
	cout << endl;

	cout << "reverse cout" << endl;
	copy(v.crbegin(), v.crend(),
		ostream_iterator<int>(cout, " "));
	cout << endl;
}