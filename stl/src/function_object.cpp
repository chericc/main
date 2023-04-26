#include "function_object.h"
#include "../utility.h"
#include <list>
#include <algorithm>
#include <functional>
#include <vector>

namespace
{
	class AddValue
	{
	private:
		int m_v;
	public:
		AddValue(int v)
		{
			m_v = v;
		}
		void operator()(int &elem) const
		{
			elem += m_v;
		}
	};
}

void TEST::test_simple_function_object()
{
	AddValue self_add5(5);
	AddValue self_add3(3);

	std::list<int> list{1,2,3,4,5};
	PRINT_ELEMENTES(list, "original list");

	std::for_each(list.begin(), list.end(), self_add3);
	PRINT_ELEMENTES(list, "add3");

	std::for_each(list.begin(), list.end(), self_add5);
	PRINT_ELEMENTES(list, "add5");
}

void TEST::test_predefined_function_object()
{
	std::vector<int> v{ 1,2,3,4,5 };
	std::vector<int> v2{ 1,1,3,3,5 };
	PRINT_ELEMENTES(v, "first list:");
	PRINT_ELEMENTES(v2, "second list");
	std::negate<int> negate;
	PRINT_ELEMENTES_OP(v, negate, "negate list");
	std::plus<int> plus;
	PRINT_ELEMENTES_OP2(v, v2, plus, "plus");
	PRINT_ELEMENTES_OP2(v, v2, std::minus<int>(), "minus");
	PRINT_ELEMENTES_OP2(v, v2, std::multiplies<int>(), "multiplies");
	PRINT_ELEMENTES_OP2(v, v2, std::divides<int>(), "divides");
	PRINT_ELEMENTES_OP2(v, v2, std::modulus<int>(), "modulus");
	PRINT_ELEMENTES_OP2(v, v2, std::equal_to<int>(), "equal_to");
	PRINT_ELEMENTES_OP2(v, v2, std::not_equal_to<int>(), "not_equal_to");
	PRINT_ELEMENTES_OP2(v, v2, std::less<int>(), "less");
	PRINT_ELEMENTES_OP2(v, v2, std::greater<int>(), "greater");
	PRINT_ELEMENTES_OP2(v, v2, std::less_equal<int>(), "less_equal");
	PRINT_ELEMENTES_OP2(v, v2, std::greater_equal<int>(), "greater_equal");
	PRINT_ELEMENTES_OP(v, std::logical_not<int>(), "logical_not");
	PRINT_ELEMENTES_OP2(v, v2, std::logical_and<int>(), "logical_and");
	PRINT_ELEMENTES_OP2(v, v2, std::logical_or<int>(), "logical_or");
	PRINT_ELEMENTES_OP2(v, v2, std::bit_and<int>(), "bit_and");
	PRINT_ELEMENTES_OP2(v, v2, std::bit_or<int>(), "bit_or");
	PRINT_ELEMENTES_OP2(v, v2, std::bit_xor<int>(), "bit_xor");
}