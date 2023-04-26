
#include <iostream>
#include <string>
#include "string.h"

using std::string;
using std::cout;
using std::endl;

void TEST::test_string_basic()
{
	cout << "TEST: test string basic" << endl;
	/* constructors */
	{
		cout << "TEST: string constructor" << endl;
		cout << "string s0" << endl;
		string s0;
		cout << "s0:" << s0.size() << endl << endl;

		cout << "string s1(\'This is amazing!\")" << endl;
		string s1("This is amazing!");
		cout << "s1:" << s1 << endl << endl;

		cout << "string s2(s1)" << endl;
		string s2(s1);
		cout << "s2:" << s2 << endl << endl;

		cout << "string s3(s2, 5)" << endl;
		string s3(s2, 5);
		cout << "s3:" << s3 << endl << endl;

		cout << "string s4(s2, 5, 7)" << endl;
		string s4(s2, 5, 7);
		cout << "s4:" << s4 << endl << endl;

		cout << "string s5(\"This is amazing!\", 5)" << endl;
		string s5("This is amazing!", 5);
		cout << "s5:" << s5 << endl << endl;

		cout << "string s6(12, '*')" << endl;
		string s6(12, '*');
		cout << "s6:" << s6 << endl << endl;

		cout << "string s7(s6.cbegin(), s6.cend())" << endl;
		string s7(s6.cbegin(), s6.cend());
		cout << "s7:" << s7 << endl << endl;

		cout << "string s8({\"This is amazing!\"})" << endl;
		string s8({ "This is amazing!" });
		cout << "s8:" << s8 << endl << endl;
		cout << endl;
	}
	/* size and capacity */
	{
		cout << "TEST: size and capacity" << endl;
		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "s0:" << s0 << endl;
		cout << "s0.size()=" << s0.size() << endl;
		cout << "s0.length()=" << s0.length() << endl;
		cout << "s0.empty()=" << s0.empty() << endl;
		cout << "s0.max_size()=" << s0.max_size() << endl;
		cout << "s0.capacity()=" << s0.capacity() << endl;
		cout << endl;
	}
	/* element access */
	{
		cout << "TEST: element acccess" << endl;
		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "s0[0]=" << s0[0] << endl;
		cout << "s0.at(0)=" << s0.at(0) << endl;
		cout << "s0.front()=" << s0.front() << endl;
		cout << "s0.back()=" << s0.back() << endl;
		cout << endl;
	}
	/* comparison */
	{
		cout << "TEST: comparison" << endl;
		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "string s1(s0)" << endl;
		string s1(s0);
		cout << "s0:" << s0 << endl;
		cout << "s1:" << s1 << endl;
		cout << "(s0==s1)=" << (s0 == s1) << endl;
		cout << "s0.compare(s1)=" << s0.compare(s1) << endl;
		cout << "s1.pop_back()" << endl;
		s1.pop_back();
		cout << "s0:" << s0 << endl;
		cout << "s1:" << s1 << endl;
		cout << "(s0==s1)=" << (s0 == s1) << endl;
		cout << "s0.compare(s1)=" << s0.compare(s1) << endl;
		cout << "s1.compare(s0)=" << s1.compare(s0) << endl;
		cout << endl;
	}
	/* assignment */
	{
		cout << "TEST: assignment" << endl;
		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		string s1;
		cout << "s1.assign(s0)" << endl;
		s1.assign(s0);
		cout << "s1:" << s1 << endl;
		cout << "s1.assign(s0, 5, 2)" << endl;
		s1.assign(s0, 5, 2);
		cout << "s1:" << s1 << endl;
		cout << "s1.assign(s0, 5, string::npos)" << endl;
		s1.assign(s0, 5, string::npos);
		cout << "s1:" << s1 << endl;
		cout << "s1.assign(\"This is amazing!\", 4)" << endl;
		s1.assign("This is amazing", 4);
		cout << "s1:" << s1 << endl;
		cout << "s1.assign(5, '*')" << endl;
		s1.assign(5, '*');
		cout << "s1:" << s1 << endl;
		cout << endl;
	}
	/* clean */
	{
		cout << "TEST: clean" << endl;
		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "s0:" << s0 << endl;
		cout << "s0.clear()" << endl;
		s0.clear();
		cout << "s0:" << s0 << endl;
		cout << "s0.assign(\"This is amazing!\")" << endl;
		s0.assign("This is amazing!");
		cout << "s0:" << s0 << endl;
		cout << "s0.erase()" << endl;
		s0.erase();
		cout << "s0:" << s0 << endl;
		cout << endl;
	}
	/* inserting and removing */
	{
		cout << "TEST: inserting and removing" << endl;
		cout << "string s0(\"is\")" << endl;
		string s0("is");
		cout << "s0:" << s0 << endl;

		cout << "s0 += \" a\"" << endl;
		s0 += " a";
		cout << "s0:" << s0 << endl;

		cout << "s0.append(\"mazing!!!!!!\")" << endl;
		s0.append("mazing!!!!!!");
		cout << "s0:" << s0 << endl;

		cout << "s0.insert(0, \"This\")" << endl;
		s0.insert(0, "This");
		cout << "s0:" << s0 << endl;

		cout << "s0.insert(4,\" \")" << endl;
		s0.insert(4, " ");
		cout << "s0:" << s0 << endl;

		cout << "s0.replace(0, 4, \"Amazing\")" << endl;
		s0.replace(0, 4, "Amazing");
		cout << "s0:" << s0 << endl;

		cout << "s0.erase(s0.size()-1)" << endl;
		s0.erase(s0.size() - 1);
		cout << "s0:" << s0 << endl;

		cout << "s0.erase(s0.size()-3, 3)" << endl;
		s0.erase(s0.size() - 3, 3);
		cout << "s0:" << s0 << endl;

		cout << "s0.pop_back()" << endl;
		s0.pop_back();
		cout << "s0:" << s0 << endl;

		cout << endl;
	}
	/* substring */
	{
		cout << "TEST: substring" << endl;

		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "s0:" << s0 << endl;
		cout << "s0.substr():" << s0.substr() << endl;
		cout << "s0.substr(5):" << s0.substr(5) << endl;
		cout << "s0.substr(5,2):" << s0.substr(5, 2) << endl;
		cout << "s0.substr(s0.find('a')):" << s0.substr(s0.find('a')) << endl;

		cout << endl;
	}
	/* search */
	{
		cout << "TEST: search" << endl;

		cout << "string s0(\"This is amazing!\")" << endl;
		string s0("This is amazing!");
		cout << "s0.find('i')=" << s0.find('i') << endl;
		cout << "s0.find(\"is\")=" << s0.find("is") << endl;
		cout << "s0.rfind('i')=" << s0.rfind('i') << endl;
		cout << "s0.rfind(\"in\")=" << s0.rfind("in") << endl;

		cout << "(s0.find('b')==string::npos)=" << (s0.find('b')==string::npos) << endl;

		cout << "s0.find_first_of(\"is\")=" << s0.find_first_of("is") << endl;
		cout << "s0.find_first_of(\"si\")=" << s0.find_first_of("si") << endl;

		cout << "s0.find_last_of(\"ing\")=" << s0.find_last_of("ing") << endl;
		cout << "s0.find_last_of(\"gni\")=" << s0.find_last_of("gni") << endl;

		cout << "s0.find_first_not_of(\"This \")=" << s0.find_first_not_of("This ") << endl;
		cout << "s0.find_last_not_of(\"is amaing!\")=" << s0.find_last_not_of("is amazing!") << endl;

		cout << endl;
	}
	/* numeric conversion */
	{
		cout << "TEST: numeric conversion" << endl;

		std::size_t idx = 0;

		cout << R"(std::stoi(string("10ghi"), & idx, 16)=)" << std::stoi(string("10ghi"), &idx, 16)
			<< " idx=" << idx << endl;

		cout << R"(std::stoi(string("10ghi"), & idx, 10)=)" << std::stoi(string("10ghi"), &idx, 10) 
			<< " idx=" << idx << endl;

		cout << R"(std::stoi(string("10ghi"), & idx, 8)=)" << std::stoi(string("10ghi"), &idx, 8)
			<< " idx=" << idx << endl;

		cout << R"(std::stoi(string("10ghi"), & idx, 2)=)" << std::stoi(string("10ghi"), &idx, 2)
			<< " idx=" << idx << endl;

		cout << R"(std::stof(string("10.001ghi"), & idx)=)" << std::stof(string("10.001ghi"), &idx)
			<< " idx=" << idx << endl;

		cout << R"(std::stod(string("1.333e2ghi"), & idx)=)" << std::stod(string("1.333e2ghi"), &idx)
			<< " idx=" << idx << endl;

		const int i0 = 12;
		const double d0 = 13.34;
		const double d1 = i0 + d0;

		cout << "to_string: " << endl;
		cout << std::to_string(i0) << " + " << std::to_string(d0) << " = " << std::to_string(d1) << endl;

		cout << endl;
	}
}