# valgrind cmds

## 常用工具

--tool=toolname

### memcheck

a memory error detector

分析内存泄漏

### cachegrind

a cache and branch-prediction profiler

### callgrind

a call-graph generating cache and branch prediction profiler

### helgrind

a thread error detector

### drd

a thread error detector

### massif

a heap profiler

分析内存使用

### dhat

a dynamic heap analysis tool

## 其它

### 分析内存使用

valgrind --tool=massif app

valgrind --tool=massif --time-unit=B --max-snapshots=64  --detailed-freq=1

ms_print 

### 分析文件描述符泄露

valgrind --track-fds=yes