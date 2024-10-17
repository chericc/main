#include <unistd.h>

void rtsp_client_test(const char* host, const char* file);

int main()
{
	rtsp_client_test("10.0.0.3", "0/1");
	while (1) {
		usleep(1000);
	}
    return 0;
}
