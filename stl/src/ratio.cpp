
#include "ratio.h"

#include <ratio>
#include <iostream>

using std::cout;
using std::endl;

template <intmax_t N, intmax_t D=1>
std::ostream &operator<<(std::ostream &os, const std::ratio<N,D> &rratio)
{
	os << "[ratio]" << rratio.num << "/" << rratio.den;
	return os;
}

void TEST::test_ratio()
{
	// ·ÖÄ¸£ºdenominator
	cout << "TEST test_ratio" << endl;

	// def
	{
		cout << "TSET def" << endl;

		typedef std::ratio<3, 5> r; // 3/5
		cout << "ratio<3,5>::num=" << r::num << endl;
		cout << "ratio<3,5>::den=" << r::den << endl;

		std::ratio<50, 100> r2;
		cout << "ratio<50,100>::num=" << r2.num << endl;
		cout << "ratio<50,100>::den=" << r2.den << endl;
		cout << "override <<: r2 = " << r2 << endl;

		cout << endl;
	}
	// operation
	{
		cout << "TEST operation" << endl;

		typedef std::ratio<3, 5> val1;
		typedef std::ratio<1, 10> val2;

		std::ratio_add<val1, val2>::type result1;
		cout << "3/5 + 1/10=" << result1 << endl;

		std::ratio_subtract<val1, val2>::type result2;
		cout << "3/5 - 1/10=" << result2 << endl;

		std::ratio_multiply<val1, val2>::type result3;
		cout << "3/5 * 1/10=" << result3 << endl;

		std::ratio_divide<val1, val2>::type result4;
		cout << "3/5 / 1/10=" << result4 << endl;

		std::ratio_equal<val1, val2>::type result5;
		cout << "3/5 == 1/10 = " << result5 << endl;

		std::ratio_not_equal<val1, val2>::type result6;
		cout << "3/5 != 1/10 = " << result6 << endl;

		std::ratio_less<val1, val2>::type result7;
		cout << "3/5 < 1/10 = " << result7 << endl;

		std::ratio_less_equal<val1, val2>::type result8;
		cout << "3/5 <= 1/10 = " << result8 << endl;

		std::ratio_greater<val1, val2>::type result9;
		cout << "3/5 > 1/10 = " << result9 << endl;
 
		std::ratio_greater_equal<val1, val2>::type result10;
		cout << "3/5 >= 1/10 = " << result10 << endl;

		cout << endl;
	}
	// predefined values
	{
		cout << "TEST predefined values" << endl;

		cout << "atto()=" << std::atto() << endl;
		cout << "femo()=" << std::femto() << endl;
		cout << "pico()=" << std::pico() << endl;
		cout << "nano()=" << std::nano() << endl;
		cout << "micro()=" << std::micro() << endl;
		cout << "milli()=" << std::milli() << endl;
		cout << "centi()=" << std::centi() << endl;
		cout << "deci()=" << std::deci() << endl;
		cout << "deca()=" << std::deca() << endl;
		cout << "hecto()=" << std::hecto() << endl;
		cout << "kilo()=" << std::kilo() << endl;
		cout << "mega()=" << std::mega() << endl;
		cout << "giga()=" << std::giga() << endl;
		cout << "tera()=" << std::tera() << endl;
		cout << "peta()=" << std::peta() << endl;
		cout << "exa()=" << std::exa() << endl;

		cout << endl;
	}

	cout << endl;
}

