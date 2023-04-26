
#include "chrono.h"

#include <iostream>
#include <chrono>

using std::cout;
using std::endl;

template <intmax_t N, intmax_t D = 1>
std::ostream &operator<<(std::ostream &os, const std::ratio<N, D> &rratio)
{
	os << "[ratio]" << rratio.num << "/" << rratio.den;
	return os;
}

void TEST::test_chrono_duration()
{
	cout << "TEST test_chrono_duration" << endl;

	// def
	{
		std::chrono::duration<int> durTwentySecs(20);
		std::chrono::duration<double, std::ratio<60>> durHalfAMinute(0.5);
		std::chrono::duration<long, std::ratio<1, 1000>> durOneMillisecond(1);

		std::chrono::nanoseconds durANanoSecond(1);
		std::chrono::microseconds durAMicroSecond(1);
		std::chrono::milliseconds durAMillisecond(1);
		std::chrono::seconds durASecond(1);
		std::chrono::minutes durAMinute(1);
		std::chrono::hours durAHour(1);
	}
	// conversion
	{
		cout << "TEST conversion" << endl;
		// second
		std::chrono::seconds durASecond(1);
		cout << "A second = " << durASecond.count() << " s" << endl;
		// second -> millisecond
		std::chrono::milliseconds durInMillisecond = durASecond;
		cout << "A second = " << durInMillisecond.count() << " ms" << endl;
		// second -> microsecond
		std::chrono::microseconds durInMicrosecond = durASecond;
		cout << "A second = " << durInMicrosecond.count() << " us" << endl;
		// second -> nanosecond
		std::chrono::nanoseconds durInNanosecond = durASecond;
		cout << "A second = " << durInNanosecond.count() << " ns" << endl;

		// hour
		std::chrono::hours durAnHour(1);
		cout << "An hour = " << durAnHour.count() << " h" << endl;
		// hour -> minute
		std::chrono::minutes durInMinute = durAnHour;
		cout << "An hour = " << durInMinute.count() << " min" << endl;
		// hour -> second
		std::chrono::seconds durInSecond = durAnHour;
		cout << "An hour = " << durInSecond.count() << " second" << endl;

		cout << endl;
	}
	// operation
	{
		cout << "TEST operation" << endl;

		// 1h + 30min = 5400s
		std::chrono::seconds durResultInSecond = std::chrono::hours(1) + std::chrono::minutes(30);
		cout << "1hour + 30min = " << durResultInSecond.count() << " sec" << endl;

		// 1h - 30min = 1800s
		durResultInSecond = std::chrono::hours(1) - std::chrono::minutes(30);
		cout << "1hour - 30min = " << durResultInSecond.count() << " sec" << endl;

		// 1min * 3 = 180s
		durResultInSecond = std::chrono::minutes(1) * 3;
		cout << "1min * 3 = " << durResultInSecond.count() << " sec" << endl;

		// 4min / 3 = 60s
		durResultInSecond = std::chrono::minutes(4) / 3;
		cout << "4min / 3 = " << durResultInSecond.count() << " sec" << endl;

		// 1min / 3 = 0s
		durResultInSecond = std::chrono::minutes(1) / 3;
		cout << "1min / 3 = " << durResultInSecond.count() << " sec" << endl;

		// 4min / 1s = 240
		cout << "4min / 1s = " << std::chrono::minutes(4) / std::chrono::seconds(1) << endl;

		// 4min / 5min = 0
		cout << "4min / 5min = " << std::chrono::minutes(4) / std::chrono::minutes(5) << endl;

		// 4min % 3 = 1min
		durResultInSecond = std::chrono::minutes(4) / 3;
		cout << "4min % 3 = " << durResultInSecond.count() << " min" << endl;

		// ++4min = 5min
		std::chrono::minutes durFourMinutes(4);
		++durFourMinutes;
		cout << "++4min = " << durFourMinutes.count() << " min" << endl;

		// --100s = 99s
		std::chrono::seconds durOneHundredSecond(100);
		--durOneHundredSecond;
		cout << "--100s = " << durOneHundredSecond.count() << " s" << endl;

		cout << endl;
	}
	// cast
	{
		cout << "TEST std::chrono::duration_cast" << endl;

		std::chrono::seconds durS(26*3600 + 7 * 60 + 35);

		std::chrono::hours durHour = std::chrono::duration_cast<std::chrono::hours>(durS);
		std::chrono::minutes durMinute = std::chrono::duration_cast<std::chrono::minutes>(durS % std::chrono::hours(1));
		std::chrono::seconds durSecond = std::chrono::duration_cast<std::chrono::seconds> (durS % std::chrono::minutes(1));

		cout << durS.count() << "sec= " << durHour.count() << "hours " << durMinute.count() << "minutes " <<
			durSecond.count() << "seconds" << endl;

		cout << endl;
	}

	cout << endl;
}

template <typename _ClockT>
void PrintClock()
{
	//using _ClockT::duration::rep
}

void TEST::test_chrono_clock()
{
	cout << "TEST test_chrono_clock" << endl;

	PrintClock<std::chrono::system_clock>();

	cout << endl;
}

