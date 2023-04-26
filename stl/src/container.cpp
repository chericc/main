#include "container.h"
#include "../utility.h"

#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <map>
#include <array>
#include <string>
#include <stack>
#include <queue>
#include <bitset>

using namespace std;

namespace
{
}

void TEST::test_array()
{
	{
		array<int, 5> a;
		PRINT_ELEMENTES(a, "default array not initialized");

		cout << "a.size()=" << a.size() << endl;
		cout << "a.empty()=" << a.empty() << endl;
		cout << "a.max_size()=" << a.max_size() << endl;

		a.fill(6);
		PRINT_ELEMENTES(a, "fill(5)");
		array<int, a.size()> b(a);
		cout << "array<int,a.size()> b(a)" << endl;
		PRINT_ELEMENTES(b, "b:");

		array<int, a.size()> c = a;
		cout << "array<int,a.size()> c = a" << endl;
		PRINT_ELEMENTES(c, "c:");

		array<int, 5> d = { 1,2,3,4 };
		cout << "array<int, 5> d = {1,2,3,4}" << endl;
		PRINT_ELEMENTES(d, "d:");
	}
	{
		array<int, 5> a = { 1,2,3,4,5 };
		array<int, 5> b = { 1,2,3,4,5 };
		PRINT_ELEMENTES(a, "a:");
		PRINT_ELEMENTES(b, "b:");
		cout << "a==b ? " << endl;
		cout << (a==b) << endl;

		a.at(a.size() - 1) = 0;
		cout << "modify a" << endl;
		PRINT_ELEMENTES(a, "a:");
		cout << "a==b ?" << endl;
		cout << (a == b) << endl;
	}
	{
		array<int, 5> a = { 1,2,3,4,5 };
		PRINT_ELEMENTES(a, "a:");
		cout << "a.front()=" << a.front() << endl;
		cout << "a.back()=" << a.back() << endl;

		try 
		{
			a.at(a.size()) = 0;
		}
		catch (std::out_of_range ex)
		{
			cout << "catch exception, " << ex.what() << endl;
		}
	}
	{
		array<int, 5> a = { 1,2,3,4,5 };
		PRINT_ELEMENTES(a, "a: ");
		for (auto it = a.begin(); it != a.end(); ++it)
		{
			*it *= 2;
		}
		PRINT_ELEMENTES(a, "using iterator *=2, a:");
	}
}

void TEST::test_vector()
{
	{
		cout << "vector: " << endl;
		vector<int> v;
		cout << "default constructor: " << endl;
		cout << "v.size()=" << v.size() << endl;
		cout << "v.max_size()=" << v.max_size() << endl;
		cout << "v.capacity()=" << v.capacity() << endl;
	}
	{
		vector<int> v = { 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v:");
		cout << "v.size()=" << v.size() << endl;
		cout << "v.max_size()=" << v.max_size() << endl;
		cout << "v.capacity()=" << v.capacity() << endl;
		v.reserve(128);
		cout << "v.reserve(128), v.capacity()=" << v.capacity() << endl;
	}
	{
		vector<int> v = { 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "vector<int> v = {1,2,3,4,5} ");
		vector<int> v1(v);
		PRINT_ELEMENTES(v1, "vector<int> v1(v): ");
		vector<int> v2 = v;
		PRINT_ELEMENTES(v2, "vector<int> v2 = v");
		vector<int> v3(5);
		PRINT_ELEMENTES(v3, "vector<int> v3(5)");
		vector<int> v4(5, 12);
		PRINT_ELEMENTES(v4, "vector<int> v4(5,12)");
		vector<int> v5(v.begin(), v.end());
		PRINT_ELEMENTES(v5, "vector<int> v5(v.begin(), v.end())");
		vector<int> v6({ 1,2,3,4,5 });
		PRINT_ELEMENTES(v6, "vector<int> v6({1,2,3,4,5))");
		vector<int> v7{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v7, "vector<int> v7{1,2,3,4,5}");
	}
	{
		vector<int> v;
		vector<int> v1{ 1,2,3,4,5 };
		PRINT_ELEMENTES(v1, "v1:");
		v = v1;
		PRINT_ELEMENTES(v, "v=v1:");
		v = { 1,2,3,4,5 };
		PRINT_ELEMENTES(v, "v={1,2,3,4,5}");
		v.assign({ 1,2,3 });
		PRINT_ELEMENTES(v, "v.assign({1,2,3})");
		v.assign(v1.begin(), v1.end());
		PRINT_ELEMENTES(v, "v.assign(v1.begin(),v1.end())");
		v.assign(4, 8);
		PRINT_ELEMENTES(v, "v.assign(4,8)");
		vector<int> v2{ 1,2,3 };
		PRINT_ELEMENTES(v2, "v2:");
		v.swap(v2);
		PRINT_ELEMENTES(v, "v.swap(v2)");
	}
	{
		vector<int> v;
		v.push_back(12);
		v.push_back(4);
		PRINT_ELEMENTES(v, "v.pushback(12),(5)");
		v.pop_back();
		PRINT_ELEMENTES(v, "v.pop_back()");
		v.insert(v.begin(), 3);
		PRINT_ELEMENTES(v, "v.insert(v.begin(), 3)");
		v.clear();
		cout << "v.clear()" << endl;
		cout << "v.size()=" << v.size() << endl;
		cout << "v.capacity()=" << v.capacity() << endl;
	}
}

void TEST::test_list()
{
	cout << "list test" << endl;
	{
		list<int> l{ 1,2,3,4,5 };
		PRINT_ELEMENTES(l, "list<int> l {1,2,3,4,5}");
		list<int> l1 = { 1,2,3,4,5 };
		PRINT_ELEMENTES(l1, "list<int> l1 = {1,2,3,4,5}");
		list<int> l2({ 1,2,3,4,5 });
		PRINT_ELEMENTES(l2, "list<int> l2 ({1,2,3,4,5})");
		list<int> l3(5, 12);
		PRINT_ELEMENTES(l1, "list<int> l1(5,12)");
	}
	{
		list<int> l = { 1,2,3,4,5 };
		PRINT_ELEMENTES(l, "list<int> l = {1,2,3,4,5}");
		l.assign({ 2,3,4,5,6 });
		PRINT_ELEMENTES(l, "l.assign({2,3,4,5,6})");
		l.assign(5, 12);
		PRINT_ELEMENTES(l, "l.assign(5,12)");
		list<int> l2;
		cout << "empty l2" << endl;
		l2.swap(l);
		PRINT_ELEMENTES(l2, "l2.swap(l)");
	}
	{
		list<int> l = { 1,2,3,4,5 };
		PRINT_ELEMENTES(l, "l");
		cout << "l.front()=" << l.front() << endl;
		cout << "l.back()=" << l.back() << endl;
	}
	{
		cout << "list special modifying operation" << endl;
		list<int> l = { 1,2,2,3,3,3,4,5,5 };
		PRINT_ELEMENTES(l, "l:");
		l.unique();
		PRINT_ELEMENTES(l, "l.unique()");
		l = { 4,2,5,1,3,2 };
		PRINT_ELEMENTES(l, "l:");
		l.sort();
		PRINT_ELEMENTES(l, "l.sort()");
		list<int> l2 = { 7,9,8 };
		PRINT_ELEMENTES(l2, "l2:");
		l2.sort();
		PRINT_ELEMENTES(l2, "l2.sort()");
		l.merge(l2);
		PRINT_ELEMENTES(l, "l.merge(l2)");
		l.reverse();
		PRINT_ELEMENTES(l, "l.reverse()");
	}
}

void TEST::test_set()
{
	cout << "test set" << endl;
	{
		set<int> s = { 1,2,3,4,5 };
		PRINT_ELEMENTES(s, "set<int> s = {1,2,3,4,5}");
		set<int, greater<int> > s2 = { 1,2,3,4,5 };
		PRINT_ELEMENTES(s2, "set<int, greater<int> > s2 = {1,2,3,4,5}");
	}
	{
		set<int> s = { 1,1,8,3,3,3,6,5 };
		PRINT_ELEMENTES(s, "set<int> s = { 1,1,8,3,3,3,6,5 }");
		cout << "s.size()=" << s.size() << endl;
		cout << "s.max_size()=" << s.max_size() << endl;
		cout << "s.count(0)=" << s.count(0) << endl;
		cout << "s.count(1)=" << s.count(1) << endl;
		auto it = s.find(3);
		cout << "*s.find(3)=" << *it << endl;
		cout << "*s.lower_bound(4)=" << *s.lower_bound(4) << endl;
		cout << "*s.upper_bound(4)=" << *s.upper_bound(4) << endl;
		cout << "*s.equal_bound(4).first=" << *s.equal_range(4).first << endl;
		cout << "*s.equal_bound(4).second=" << *s.equal_range(4).second << endl;
		cout << "*s.lower_bound(5)=" << *s.lower_bound(5) << endl;
		cout << "*s.upper_bound(5)=" << *s.upper_bound(5) << endl;
		cout << "*s.equal_bound(5).first=" << *s.equal_range(5).first << endl;
		cout << "*s.equal_bound(5).second=" << *s.equal_range(5).second << endl;
	}
}

void TEST::test_map()
{
	cout << "test_map()" << endl;
	{
		map<string, int> m = { {"Jerry",78},{"Tom",80}, {"Lily",81} };
		PRINT_ELEMENTES<string, int>(m, "map<string,int> m = { {\"Jerry\",78},{\"Tom\",80}, {\"Lily\",81} }:");
		cout << "m.count(\"Jerry\")=" << m.count("Jerry") << endl;
		const auto &ref_find = m.find("Jerry");
		cout << "m.find(\"Jerry\")->first=" << ref_find->first << endl;
		cout << "m.find(\"Jerry\")->second=" << ref_find->second << endl;
		const auto &ref_lower_bound = m.lower_bound("Kitty");
		cout << "m.lower_bound(\"Kitty\")->first=" << ref_lower_bound->first << endl;
		cout << "m.lower_bound(\"Kitty\")->second=" << ref_lower_bound->second << endl;
		const auto &ref_upper_bound = m.upper_bound("Kitty");
		cout << "m.upper_bound(\"Kitty\")->first=" << ref_upper_bound->first << endl;
		cout << "m.upper_bound(\"Kitty\")->second=" << ref_upper_bound->second << endl;
		const auto &ref_equal_range = m.equal_range("Kitty");
		cout << "m.equal_range(\"Kitty\").first->first=" << ref_equal_range.first->first << endl;
		cout << "m.equal_range(\"Kitty\").second->first=" << ref_equal_range.second->first << endl;
	}
	{
		map<string, int> m;
		m.insert(pair<string,int>("Jerry", 78));
		m.insert(make_pair("Tom", 80));
		m.insert({ "Lily",81 });
		m.insert(map<string, int>::value_type("Jack", 93));
		PRINT_ELEMENTES<string, int>(m, "map.insert():");
		m.erase("Jack");
		PRINT_ELEMENTES<string, int>(m, "map.erase(\"Jack\")");
	}
}

void TEST::test_adapter()
{
	/* stack */
	{
		std::cout << "TEST: stack" << std::endl;
		std::stack<int> s;
		s.push(1);
		s.push(2);
		s.emplace(3);
		s.emplace(4);
		std::cout << "pop elements" << std::endl;
		while (! s.empty())
		{
			std::cout << s.top() << " ";
			s.pop();
		}
		std::cout << std::endl << std::endl;
	}
	/* queue */
	{
		std::cout << "TEST: queue" << std::endl;
		std::queue<int> q;
		q.push(1);
		q.push(2);
		q.emplace(3);
		q.emplace(4);
		std::cout << "pop elements" << std::endl;
		while (!q.empty())
		{
			std::cout << q.front() << " ";
			q.pop();
		}
		std::cout << std::endl << std::endl;
	}
	/* priority_queue */
	{
		std::cout << "TEST: priority_queue" << std::endl;
		std::priority_queue<double, std::vector<double>, std::greater<double>> q;
		q.push(87.5);
		q.push(64.0);
		q.emplace(90.0);
		q.emplace(98.0);
		std::cout << "pop elementes" << std::endl;
		while (!q.empty())
		{
			std::cout << q.top() << " ";
			q.pop();
		}
		std::cout << std::endl << std::endl;
	}
	/* bit_set */
	{
		std::cout << "TEST: bit_set" << std::endl;
		std::bitset<12> bs;
		bs.set(1);
		bs.set(3);
		bs.set(5);
		std::cout << "bitset.to_string()=" << bs.to_string() << std::endl;
		std::cout << std::endl;
	}
}