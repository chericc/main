#include "numeric.h"

#include <random>
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>

#include "utility.h"

using std::cout;
using std::endl;

namespace 
{
	template <typename Distribution, typename Engine>
	void distr(Distribution dis, Engine eng, const std::string &name)
	{
		cout << "name=" << name << endl;
		cout << "-max=" << eng.min() << endl;
		cout << "-max=" << eng.max() << endl;
		cout << "-values=" << dis(eng) << ' ' << dis(eng) << ' ' << dis(eng) << ' ' << endl;

		std::map<int, int> counter;
		for (int i = 0; i < 50000; ++i)
		{
			++counter[dis(eng)];
		}

		cout << "===" << endl;
		for (auto elem : counter)
		{
			cout << elem.first << " " << elem.second << endl;
		}
		cout << "===" << endl;
		cout << endl;
	}
}

void TEST::test_random_number()
{
	/* generate random numbers */
	{
		cout << "TEST: generate random numbers examples" << endl;
        // use a default random seed
        std::random_device rd;

		// create default engine as source randomness
		std::default_random_engine dre(rd());

		// use engine to generate integral numbers between 10 and 20 (both included)
		std::uniform_int_distribution<int> di(10, 20);
		for (int i = 0; i < 20; ++i) 
		{
			cout << di(dre) << " ";
		}
		cout << endl;

		// use engine to generate floating-point numbers between 10.0 and 20.0
		std::uniform_real_distribution<double> dr(10, 20);
		for (int i = 0; i < 8; ++i)
		{
			cout << dr(dre) << " ";
		}

		cout << endl;
		cout << endl;
	}
	/* shuffle example */
	{
		cout << "TEST: shuffle using random numbers" << endl;
		std::default_random_engine dre;
		/* use engine to shuffle elements */
		std::vector<int> v = { 0,1,2,3,4,5,6,7,8,9 };
		PRINT_ELEMENTES(v, "v:");
		std::shuffle(v.begin(), v.end(), dre);
		PRINT_ELEMENTES(v, "v:");
		cout << endl;
	}
}

void TEST::test_ditributeion()
{
	std::knuth_b e;

	std::uniform_real_distribution<> ud(0, 10);
	distr(ud, e, "uniform_real_distribution");

	std::normal_distribution<> nd;
	distr(nd, e, "normal_distribution");

	std::exponential_distribution<> ed;
	distr(ed, e, "exponential_distribution");

	std::gamma_distribution<> gd;
	distr(gd, e, "gamma_distribution");
}
