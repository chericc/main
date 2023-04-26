
#include "numericlimit.h"
#include <limits>
#include <iostream>

using std::cout;
using std::endl;

template <typename T>
void PrintNumeric()
{
	std::numeric_limits<T> l;
	cout << "is_specialized=" << l.is_specialized << endl;
	cout << "is_signed=" << l.is_signed << endl;
	cout << "is_exact=" << l.is_exact << endl;
	cout << "is_bounded=" << l.is_bounded << endl;
	cout << "is_modulo=" << l.is_modulo << endl;
	cout << "is_iec559=" << l.is_iec559 << endl;
	cout << "min()=" << l.min() << endl;
	cout << "max()=" << l.max() << endl;
	cout << "lowest()=" << l.lowest() << endl;
	cout << "digits=" << l.digits << endl;
	cout << "digits10=" << l.digits10 << endl;
	cout << "radix=" << l.radix << endl;
	cout << "min_exponent=" << l.min_exponent << endl;
	cout << "max_exponent=" << l.max_exponent << endl;
	cout << "min_exponent10=" << l.min_exponent10 << endl;
	cout << "max_exponent10=" << l.max_exponent10 << endl;
	cout << "epsilon()=" << l.epsilon() << endl;
	cout << "round_style=" << l.round_style << endl;
	cout << "round_error()=" << l.round_error() << endl;
	cout << "has_infinity=" << l.has_infinity << endl;
	cout << "quiet_NaN()=" << l.quiet_NaN() << endl;
	cout << "has_signaling_NaN" << l.has_signaling_NaN << endl;
	cout << "signaling_NaN()=" << l.signaling_NaN() << endl;
	cout << "has_denorm=" << l.has_denorm << endl;
	cout << "has_denorm_loss=" << l.has_denorm_loss << endl;
	cout << "denorm_min()=" << l.denorm_min() << endl;
	cout << "traps=" << l.traps << endl;
	cout << "tinyness_before=" << l.tinyness_before << endl;
 }

void TEST::test_numeric_limit()
{
	cout << "TEST test_numeric_limit" << endl;

	cout << "int" << endl;
	PrintNumeric<int>();
	cout << endl;

	cout << "char" << endl;
	PrintNumeric<char>();
	cout << endl;

	cout << "double" << endl;
	PrintNumeric<double>();
	cout << endl;

	cout << endl;
}

