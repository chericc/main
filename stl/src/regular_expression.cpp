
#include "regular_expression.h"
#include <string>
#include <regex>
#include <iostream>

using std::cout;
using std::endl;

void TEST::test_regular_expression()
{
	/* regex_match, regex_search */
	{
		cout << "TEST: regex_match" << endl;

		std::string str(R"(<tag>value</tag>)");
		std::string reg_str(R"(<.*>.*</.*>)");
		std::regex reg1(reg_str);
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;
		cout << "regex_match found=" << std::regex_match(str, reg1) << endl;
		cout << endl;

		reg_str.assign(R"(<(.*)>.*</\1>)");
		std::regex reg2(reg_str);
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;
		cout << "regex_match found=" << std::regex_match(str, reg2) << endl;
		cout << endl;

		reg_str.assign(R"(<\(.*\)>.*</\1>)");
		std::regex reg3(reg_str, std::regex_constants::grep);
		cout << "str=" << reg_str << endl;
		cout << "reg_str=" << reg_str << endl;
		cout << "regex_match found=" << std::regex_match(str, reg3) << endl;
		cout << endl;

		str.assign(R"(XML tag: <tag>value</tag>)");
		reg_str.assign(R"(<(.*)>.*</\1>)");
		std::regex reg4(reg_str);
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;
		cout << "regex_match found=" << std::regex_match(str, reg4) << endl;
		cout << endl;

		std::regex reg5(reg_str);
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;
		cout << "regex_search found=" << std::regex_search(str, reg5) << endl;
		cout << endl << endl;
	}
	/* subexpression */
	{
		cout << "TEST: subexpression" << endl;

		std::string str = R"(XML tag: <tag-name>the value</tag-name>.)";
		std::string reg_str = R"(<(.*)>(.*)</(\1)>)";
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;

		cout << "bool found = regex_search(str, smatch, regex(reg_str))" << endl;
		std::smatch smatch;
		bool found = std::regex_search(str, smatch, std::regex(reg_str));
		cout << "found=" << found << endl;

		cout << "smatch.empty()=" << smatch.empty() << endl;
		cout << "smatch.size()=" << smatch.size() << endl;
		cout << "smatch.str()=" << smatch.str() << endl;
		cout << "smatch.length()=" << smatch.length() << endl;
		cout << "smatch.position()=" << smatch.position() << endl;
		cout << "smatch.prefix().str()=" << smatch.prefix().str() << endl;
		cout << "smatch.suffix().str()=" << smatch.suffix().str() << endl;
		cout << endl;
		
		for (unsigned int i=0; i<smatch.size(); ++i)
		{
			cout << "smatch[" << i << "].str()=" << smatch[i].str() << endl;
			cout << "smatch.str(" << i << ")=" << smatch.str(i) << endl;
			cout << "smatch.position(" << i << ")=" << smatch.position(i) << endl;
			cout << "smatch.length(" << i << ")=" << smatch.length(i) << endl;
			cout << endl;
		}
		cout << endl;
	}
	/* subexpression_2 */
	{
		cout << "TEST: subexpression_2" << endl;
		std::string str(
			"<person>\n"
			" <first>Nico</first>\n"
			" <last>Josuttis</last>\n"
			"</person>\n");
		std::string reg_str(R"(<(.*)>(.*)</(\1)>)");
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;

		auto pos = str.cbegin();
		auto end = str.cend();
		std::smatch smatch;
		std::regex reg(reg_str);
		for (; std::regex_search(pos, end, smatch, reg); pos = smatch.suffix().first)
		{
			cout << "match=" << smatch.str() << endl;
			cout << "tag=" << smatch.str(1) << endl;
			cout << "value=" << smatch.str(2) << endl;
			cout << endl;
		}
	}
	/* regex iterator */
	{
		cout << "TEST: regex iterator" << endl;
		std::string str(
			"<person>\n"
			" <first>Nico</first>\n"
			" <last>Josuttis</last>\n"
			"</person>\n");
		std::string reg_str(R"(<(.*)>(.*)</(\1)>)");

		cout << "str=" << str << endl;
		cout << "reg=" << reg_str << endl;

		std::regex reg(reg_str);
		std::sregex_iterator pos(str.cbegin(), str.cend(), reg);
		std::sregex_iterator end;
		for (; pos != end; ++pos)
		{
			cout << "match=" << pos->str() << endl;
			cout << "tag=" << pos->str(1) << endl;
			cout << "value=" << pos->str(2) << endl;
			cout << endl;
		}
		cout << endl;
	}
	/* regex token iterator */
	{
		cout << "TEST: regex token iterator" << endl;
		std::string str(
			"<person>\n"
			" <first>Nico</first>\n"
			" <last>Josuttis</last>\n"
			"</person>\n");
		std::string reg_str(R"(<(.*)>(.*)</(\1)>)");

		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;

		{
			cout << "sregex_token_iterator pos(str.cbegin(), str.cend(), reg, {-1,-1,0,0,1,1,2,2,3,3} )" << endl;
			std::regex reg(reg_str);
			std::sregex_token_iterator pos(str.cbegin(), str.cend(), reg, { -1,-1,0,0,1,1,2,2,3,3 });
			std::sregex_token_iterator end;
			for (; pos != end; ++pos)
			{
				cout << "match: " << pos->str() << endl;
			}
		}
		{
			cout << "sregex_token_iterator pos(str.cbegin(), str.cend(), reg, {-1} )" << endl;
			std::regex reg(reg_str);
			std::sregex_token_iterator pos(str.cbegin(), str.cend(), reg, { -1 });
			std::sregex_token_iterator end;
			for (; pos != end; ++pos)
			{
				cout << "match: " << pos->str() << endl;
			}
		}
		{
			cout << "sregex_token_iterator pos(str.cbegin(), str.cend(), reg, {0} )" << endl;
			std::regex reg(reg_str);
			std::sregex_token_iterator pos(str.cbegin(), str.cend(), reg, { 0 });
			std::sregex_token_iterator end;
			for (; pos != end; ++pos)
			{
				cout << "match=" << pos->str() << endl;
			}
		}
		cout << endl;

		str = "zhangwuji, zhangsanfeng, xiexun, zhouzhiruo, zhaomin";
		reg_str = "[ ]*[,][ ]*";
		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;
		{
			cout << "sregex_token_iterator pos(str.cbegin(), str.cend(), reg, {-1})" << endl;
			std::regex reg(reg_str);
			std::sregex_token_iterator pos(str.cbegin(), str.cend(), reg, { -1 });
			std::sregex_token_iterator end;
			for (; pos != end; ++pos)
			{
				cout << "match=" << pos->str() << endl;
			}
			cout << endl;
		}

		cout << endl;
	}
	/* regex_replace */
	{
		cout << "TEST: regex_replace" << endl;
		std::string str(
			"<person>\n"
			" <first>Nico</first>\n"
			" <last>Josuttis</last>\n"
			"</person>\n");
		const std::string reg_str(R"(<(.*)>(.*)</(\1)>)");

		cout << "str=" << str << endl;
		cout << "reg_str=" << reg_str << endl;

		cout << R"(regex_replace(str, regex, "<$1 value="$2">"))" << endl;
		std::string str_replace = std::regex_replace(str, std::regex(reg_str), R"(<$1 value="$2"/>)");
		cout << "str_replace=" << str_replace << endl;

		cout << R"(regex_replace(str, regex, "<\1 value="\2">", format_sed)" << endl;
		str_replace = std::regex_replace(str, std::regex(reg_str), R"(<\1 value="\2"/>)", std::regex_constants::format_sed);
		cout << "str_replace=" << str_replace << endl;

		cout << "regex_replace using iterator" << endl;
		str_replace.clear();
		std::regex_replace(std::back_inserter(str_replace),
			str.cbegin(), str.cend(), std::regex(reg_str),
			R"(<$1 value="$2"/>)",
			std::regex_constants::format_no_copy | std::regex_constants::format_first_only);
		cout << "str_replace=" << str_replace << endl;

		cout << endl;
	}
	/* regex flag */
	{
		std::regex_constants::syntax_option_type f;
		f = std::regex_constants::ECMAScript; // default
		f = std::regex_constants::basic; // POSIX basic regular expression(BRE)
		f = std::regex_constants::extended; // POSIX extened regular expression(ERE)
		f = std::regex_constants::awk; // UNIX awk
		f = std::regex_constants::grep; // UNIX grep
		f = std::regex_constants::egrep; // UNIX egrep
		f = std::regex_constants::icase; // ignore case
		f = std::regex_constants::nosubs;
		std::regex_constants::match_flag_type f1;
		f1 = std::regex_constants::match_not_null;
		f1 = std::regex_constants::match_not_bol;
	}
	/* regex exception */
	{
	}
}
