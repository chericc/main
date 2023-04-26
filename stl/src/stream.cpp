#include "stream.h"

#include <iostream>
#include <bitset>
#include <complex>
#include <iomanip>
#include <fstream>
#include <sstream>

using std::cout;
using std::endl;

void TEST::test_stream()
{
	cout << "TEST: test_stream" << endl;
	/* stream operator << */
	{
		cout << "TEST: <<" << endl;

		int i = 7;
		cout << "int i=" << i << endl;

		double d = 7.65;
		cout << "double d=" << d << endl;

		void *p = (void*)0x1234;
		cout << "void *p=" << p << endl;

		std::bitset<10> bitset(0x1 | 0x2 | 0x4 | 0x8 | 0x10);
		cout << "bitset<10>=" << bitset << endl;

		std::complex<float> c(-1, 3);
		cout << "complex<float>=" << c << endl;

		cout << endl;
	}
}

void TEST::test_stream_manipulator()
{
	cout << "TEST: test_stream_manipulator" << endl;

	cout << "TEST ends(\0)" << endl;
	cout << std::ends;
	cout << endl;

	cout << "TEST flush" << endl;
	cout << std::flush;
	cout << endl;

	cout << "TEST setw(5)" << endl;
	cout << std::setw(5);
	cout << "Jim";
	cout << endl;
	cout << "Jim" << endl;
	cout << endl;

	cout << "TEST setfill(*)" << endl;
	cout << std::setfill('*');
	cout << std::setw(5);
	cout << "Jim" << endl;
	cout << endl;

	cout << "TEST left" << endl;
	cout << std::setw(5) << std::left << 123 << endl;
	cout << endl;

	cout << "TEST right" << endl;
	cout << std::setw(5) << std::right << 123 << endl;
	cout << endl;

	cout << "TEST internal" << endl;
	cout << std::setw(5) << std::internal << -123 << endl;
	cout << std::setw(5) << std::internal << 123 << endl;
	cout << endl;

	cout << "TEST boolalpha" << endl;
	cout << std::boolalpha;
	cout << true << endl;
	cout << false << endl;
	cout << endl;

	cout << "TEST noboolalpha" << endl;
	cout << std::noboolalpha;
	cout << true << endl;
	cout << false << endl;
	cout << endl;

	cout << "TEST showpos" << endl;
	cout << std::showpos;
	cout << 5 << endl;
	cout << endl;

	cout << "TEST noshowpos" << endl;
	cout << std::noshowpos << endl;
	cout << 5 << endl;
	cout << endl;

	cout << "TEST uppercase" << endl;
	cout << std::uppercase;
	cout << 1e10 << endl;
	cout << endl;

	cout << "TEST nouppercase" << endl;
	cout << std::nouppercase;
	cout << 1e10 << endl;
	cout << endl;

	cout << "TEST oct dec hex" << endl;
	cout << std::oct;
	cout << "16=" << 16 << endl;
	cout << std::dec;
	cout << "16=" << 16 << endl;
	cout << std::hex;
	cout << "16=" << 16 << endl;
	cout << endl;

	cout << "TEST showbase" << endl;
	cout << std::showbase;
	cout << std::oct;
	cout << 16 << endl;
	cout << std::dec;
	cout << 16 << endl;
	cout << std::hex;
	cout << 16 << endl;
	cout << endl;

	cout << "TEST noshowbase" << endl;
	cout << std::noshowbase;
	cout << 16 << endl;
	cout << endl;

	cout << "TEST showpoint" << endl;
	cout << std::showpoint;
	cout << 16.2 << endl;
	cout << endl;

	cout << "TEST noshowpoint" << endl;
	cout << std::noshowpoint;
	cout << 16.2 << endl;
	cout << endl;

	cout << "TEST setprecision(4)" << endl;
	cout << std::showpoint;
	cout << std::setprecision(4);
	cout << 16.2 << endl;

	cout << "TEST fixed scientific hexfloat defaultfloat" << endl;
	cout << std::fixed;
	cout << 3.14 << endl;
	cout << std::scientific;
	cout << 3.14 << endl;
	cout << std::hexfloat;
	cout << 3.14 << endl;
	cout << std::defaultfloat;
	cout << 3.14 << endl;

	cout << endl;
}

void TEST::test_formatting()
{
	cout << "TEST test_formatting" << endl;

	{
		cout << "TEST format control functions" << endl;

		cout << "cout.flags()=" << cout.flags() << endl;
		cout << "cout.flags(std::iso::showpos)=" << cout.flags(std::ios::showpos) << endl;
		cout << "cout.flags()=" << cout.flags() << endl;

		cout.unsetf(std::ios::showpos);

		cout << endl;
	}

	{
		cout << "TEST setf unsetf" << endl;

		cout << "16=" << 16 << endl;
		cout << "cout.setf(std::ios::oct)" << endl;
		cout.setf(std::ios::oct);
		cout << "16=" << 16 << endl;

		cout << "cout.setf(std::ios::boolalpha)" << endl;
		cout.setf(std::ios::boolalpha);
		cout << "true=" << true << endl;
		cout << "false=" << false << endl;
		cout.unsetf(std::ios::boolalpha);
		cout << "true=" << true << endl;
		cout << "false=" << false << endl;

		cout << endl;
	}

	{
		cout << "TEST width fill left right" << endl;
		cout << "cout.width()=" << cout.width() << endl;
		cout << 13 << endl;
		cout << "cout.width(8)" << endl;
		cout.width(8);
		cout << 13 << endl;

		cout << "cout.fill()=" << cout.fill() << endl;
		cout.width(8);
		cout << "cout.fill('*')" << endl;
		cout.fill('*');
		cout << 13 << endl;
		cout << "cout.fill()=" << cout.fill() << endl;

		cout << endl;
	}

	cout << endl;
}

void TEST::test_fstream()
{
	const std::string filename = "test.data";
	// output file
	{
		cout << "TEST output file" << endl;
		std::ofstream file(filename);
		if (file.fail())
		{
			std::cerr << "failed to open file=" << filename << endl;
			cout << "file.state=" << file.rdstate() << endl;
		}
		else
		{
			for (int i = 'A'; i < 'D'; ++i)
			{
				file << "value=" << std::setw(3) << i << " " << "char=" 
					<< static_cast<char>(i) << endl;
			}
		}
		cout << endl;
	}
	// input file
	{
		cout << "TEST input file" << endl;
		std::ifstream file(filename);
		if (file.fail())
		{
			std::cerr << "failed to open file=" << filename << endl;
		}
		else 
		{
			char c;
			while (file.get(c))
				cout.put(c);
		}
		cout << endl;
	}
	// open mode
	{
		cout << "TEST open mode" << endl;
		const std::string filename("test.data");
		std::fstream file(filename, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
		char c;
		while (file.get(c))
			cout.put(c);
		file.close();

		std::fstream file2(filename, std::ios::out | std::ios::trunc);
		while (file2.get(c))
			cout.put(c);
		cout << endl;
	}
	// explicit open 
	{
		cout << "TEST explicit open" << endl;
		const std::string filename("test.data");
		std::fstream file;
		file.open(filename, std::ios::out | std::ios::trunc);
		
		cout << "file.is_open()=" << file.is_open() << endl;
		file.close();
		cout << "file.is_open()=" << file.is_open() << endl;
		cout << endl;
	}
	// random access
	{
		cout << "TEST random access" << endl;

		const std::string filename("test.data");
		std::fstream file;
		file.open(filename, std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			std::cerr << "open file failed" << endl;
		}
		else
		{
			for (char c = '1'; c < '9'; ++c)
			{
				file.put(c);
			}
		}
		file.close();

		file.open(filename, std::ios::in);
		cout << "file.tellg()=" << file.tellg() << endl;
		file.seekg(5);
		cout << "file.tellg()=" << file.tellg() << endl;
		file.seekg(-5, std::ios::end);
		cout << "file.tellg()=" << file.tellg() << endl;
		file.seekg(2, std::ios::beg);
		file.seekg(2, std::ios::cur);
		cout << "file.tellg()=" << file.tellg() << endl;
		file.close();

		file.open(filename, std::ios::out);
		cout << "file.tellp()=" << file.tellp() << endl;
		file.seekp(2, std::ios::beg);

		cout << endl;
	}
}

void TEST::test_stringstream()
{
	cout << "TEST test_stream" << endl;

	{
		cout << "TEST ostringstream" << endl;
		std::ostringstream oss;
		oss << "623";
		oss.seekp(0);
		oss << "1";
		cout << oss.str() << endl;
		cout << endl;
	}
	{
		cout << "TEST istringstream" << endl;
		std::istringstream iss("3.7");
		int i;
		double f;
		iss >> i >> f;
		cout << i << endl;
		cout << f << endl;
		cout << endl;
	}

	cout << endl;
}

void TEST::test_coupling()
{
	// TEST loose coupling
	/** a.tie(b):
	*   Buffer of b will be emptied before an input or output operation.
	*/

	// TEST tight coupling
	/** a and b share same buffer.
		// T a(b.rdbuf());
	*/
		
}