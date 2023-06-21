# sprof

## compared to gprof

gprof 生成一个程序的执行概况；

sprof 显示一个动态库的分析概况；

## steps

```bash
# build your program and shared libraries with --pg option

# setup environment virables
export LD_PROFILE=libname.so
export LD_PROFILE_OUTPUT=$(pwd)/prof_data
mkdir -p $LD_PROFILE_OUTPUT

# run your program

# analyse output data with sprof

# 
sprof -p libxtools.so prof_data/libxtools.so.profile
Inconsistency detected by ld.so: dl-open.c: 929: _dl_open: Assertion `_dl_debug_update (args.nsid)->r_state == RT_CONSISTENT' failed!

--> 

it's said to be an gcc bug.

```