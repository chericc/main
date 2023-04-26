#include "Singleton.h"

EXAMPLE_BEGIN

Boss Boss::m_instance;

Boss* Boss::instance()
{
	return &m_instance;
}

std::string Boss::Sing()
{
	return m_name + ": " + 
		"dahe xiangdongliu ...";
}

EXAMPLE_END