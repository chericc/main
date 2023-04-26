#include "concurrency.h"

#include <future>
#include <thread>
#include <chrono>
#include <random>
#include <iostream>
#include <exception>
#include <chrono>
#include <string>
#include <stdexcept>

using std::cout;
using std::endl;

namespace 
{
	int doSomething(char c)
	{
		std::default_random_engine dre(c);
		std::uniform_int_distribution<int> id(1, 50);

		for (int i = 0; i < 10; ++i)
		{
			int t = id(dre);
			std::this_thread::sleep_for(std::chrono::milliseconds(t));
			cout.put(c).flush();
		}
		return c;
	}
	int func1()
	{
		return doSomething('.');
	}
	int func2()
	{
		return doSomething('+');
	}
	int func3()
	{
		throw 3;
		return 0;
	}
	int func4(int nParam)
	{
		cout << "func4 nParam=" << nParam << endl;
 		return nParam * nParam;
	}
}

void TEST::test_async_future()
{
	cout << "TEST test_async_future" << endl;
	// normal
	{
		cout << "TEST default usage" << endl;
		std::future<int> result1(std::async(func1));
		std::future<int> result2(std::async(func2));
		int r1 = result1.get();
		int r2 = result2.get();
		cout << endl;
		cout << "result1=" << static_cast<char>(r1) << endl;
		cout << "result2=" << static_cast<char>(r2) << endl;
		cout << endl;
	}
	// async
	{
		cout << "TEST async usage" << endl;
		std::future<int> result1(std::async(std::launch::async, func1));
		std::future<int> result2(std::async(std::launch::async, func2));
		int r1 = result1.get();
		int r2 = result2.get();
		cout << endl;
		cout << "result1=" << static_cast<char>(r1) << endl;
		cout << "result2=" << static_cast<char>(r2) << endl;
		cout << endl;
	}
	// deferred
	{
		cout << "TEST deferred usage" << endl;
		std::future<int> result1(std::async(std::launch::deferred, func1));
		std::future<int> result2(std::async(std::launch::deferred, func2));
		int r1 = result1.get();
		int r2 = result2.get();
		cout << endl;
		cout << "result1=" << static_cast<char>(r1) << endl;
		cout << "result2=" << static_cast<char>(r2) << endl;
		cout << endl;
	}
	// exception
	{
		cout << "TEST exception handling" << endl;
		std::future<int> result(std::async(std::launch::deferred, func3));
		try 
		{
			int r = result.get();
			cout << "result=" << r << endl;
		}
		catch (int e)
		{
			cout << "catch exception=" << e << endl;
		}
		cout << endl;
	}
	// passing argument
	{
		cout << "TEST passing argument" << endl;
		int nParam = 5;
		std::future<int> result(std::async(std::launch::deferred, func4, 5));
		int nResult = result.get();
		cout << "result=" << nResult << endl;
		cout << endl;
	}
	cout << endl;
}

void TEST::test_thread()
{
	cout << "TEST test_thread" << endl;

	// passing argument
	{
		cout << "TEST passing argument. " << endl;
		std::thread t1(func4, 5);
		std::thread::id id = t1.get_id();
		t1.join();
		cout << "started thread " << id << endl; // here id is a class
	}

	// exception handling
	{
		// exception in thread must be caught, or the program will be terminated
		cout << "TEST exception handling. Not catching it will lead to termination" << endl;
		try
		{
			std::thread t1(func3);
			std::thread::id id = t1.get_id();
			t1.join();
			cout << "started thread" << id << endl;
		}
		catch (...)
		{
			cout << "catch an exception" << endl; // This will catch nothing.
		}
	}

	cout << endl;
}

namespace
{
	void func5(std::promise<std::string> &p)
	{
		try
		{
			throw std::runtime_error(std::string("This is an exception"));
		}
		catch (...)
		{
			p.set_exception(std::current_exception());
		}
	}
	void func6(std::promise<std::string> &p)
	{
		p.set_value("this is amazing!");
	}
}

void TEST::test_promise()
{
	cout << "TEST test_promise" << endl;

	{
		cout << "TEST pass exception" << endl;
		try
		{
			std::promise<std::string> p;
			std::thread t(func5, std::ref(p)); // thread --> &&
			t.join();

			std::future<std::string> f(p.get_future());
			cout << "future=" << f.get() << endl;
		}
		catch (...)
		{
			cout << "catch exception" << endl;
		}
		cout << endl;
	}

	{
		cout << "TEST pass value" << endl;

		std::promise<std::string> p;
		std::thread t(func6, std::ref(p));
		t.join();

		std::future<std::string> f(p.get_future());
		cout << f.get() << endl;
		cout << endl;
	}

	cout << endl;
}

namespace
{
	std::mutex g_mutexPrintMutex;
	std::recursive_mutex g_rmutexPrintMutex;
	std::mutex g_mutexMutex;

	void print(const std::string &text)
	{
		std::lock_guard<std::mutex> lguard(g_mutexPrintMutex);

		for (char c : text)
			cout.put(c);
		cout << endl;
	}

	void callPrint(const std::string &text)
	{
		std::lock_guard<std::mutex> lguard(g_mutexPrintMutex);
		print(text);
	}

	void printA(const std::string &text)
	{
		std::lock_guard<std::recursive_mutex> lguard(g_rmutexPrintMutex);

		for (char c : text)
			cout.put(c);
		cout << endl;
	}
	void printB(const std::string &text)
	{
		std::lock_guard<std::recursive_mutex> lguard(g_rmutexPrintMutex);

		printA(text);
	}

	void quickLocker()
	{
		std::lock_guard<std::mutex> l(g_mutexMutex);
		//std::this_thread::sleep_for(std::chrono::duration()
	}
	void slowLocker()
	{

	}
}

void TEST::test_mutex_lock()
{
	cout << "TEST test_mutex_lock" << endl;

	// mutex lock_guard
	{
		cout << "TEST mutex and lock_guard" << endl;
		std::future<void> future1 = std::async(std::launch::async, print, "First thread");
		std::future<void> future2 = std::async(std::launch::async, print, "Second thread");
		future1.get();
		future2.get();
		cout << endl;
	}
	// mutex lock_guard: lock more than once
	{
		cout << "TEST mutex and lock guard: lock twice" << endl;
		try
		{
			std::future<void> future = std::async(std::launch::async, callPrint, "First thread");
			future.get();
		}
		catch (std::system_error e)
		{
			cout << "catch exception: e.code().message()=" << e.code().message() << endl;
		}
		cout << endl;
	}
	// recursive_lock lock_guard
	{
		cout << "TEST recursive_lock and lock_guard" << endl;

		std::future<void> future1 = std::async(std::launch::async, printA, "First thread");
		std::future<void> future2 = std::async(std::launch::async, printB, "Second thread");
		future1.get();
		future2.get();
		cout << endl;
	}
	// try_lock and try_lock_for
	{
		cout << "TEST try_lock try_lock_for" << endl;
		

	}

	cout << endl;
}
