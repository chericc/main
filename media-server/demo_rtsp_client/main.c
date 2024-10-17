#include <unistd.h>

#include "demo_rtsp.h"

int main()
{
	rtsp_client_test("10.0.0.103", "0/1");
	while (1) {
		usleep(1000);
	}
    return 0;
}
