#include <Public/DPSingleton.h>

#include <iostream>

#include "Singleton.h"

using std::cout;
using std::endl;

/*

*/

PRIVATE_BEGIN

void Example()
{
	cout << EXAMPLE::Boss::instance()->Sing() << endl;
}

PRIVATE_END

DP_BEGIN

void DPSingleton::Play()
{
	Example();
}

DP_END