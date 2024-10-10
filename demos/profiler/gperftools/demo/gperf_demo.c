
/*
/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc gperf_demo.c -I/home/test/code/work/env_gperf/output/include/gperftools -L/home/test/code/work/env_gperf/output/lib -ltcmalloc
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "heap-profiler.h"

int main() {
  if (1) {
    HeapProfilerStart("prefix");
    for (int i = 0; i < 100; ++i) {
      malloc(60 * 1024);
      HeapProfilerDump("11111");
      // char *profile = GetHeapProfile();
      // printf("profile: %s\n", profile);
      // free(profile);
    }
    char *profile = GetHeapProfile();
    printf("profile: %s\n", profile);
    free(profile);
    HeapProfilerDump("11111");
    HeapProfilerStop();
    printf("end\n");
    while (1) {
      sleep(1);
    }
  }

  return 0;
}