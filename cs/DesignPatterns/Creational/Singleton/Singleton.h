#include <string>
#include <Public/DesignPatterns.h>

EXAMPLE_BEGIN

class Singleton
{
};

/*
使用“饿汉”方式。具备多线程安全性。
*/

class Boss : public Singleton
{
public:
	static Boss *instance();
public:
	std::string Sing();
protected:
	Boss() :m_name("zhangsan") {}
	static Boss m_instance;
private:
	std::string m_name;
};

EXAMPLE_END