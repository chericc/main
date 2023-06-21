# comparison

## valgrind

## gperftools

## asan

## dmalloc

## compare

## advice

1. 对于日常开发和调试的环境，可以常态打开asan，这样能更早暴露内存问题，并且不会对性能产生明显影响（max 2x）；

2. 如果需要分析内存的占用情况，或者分析内存的占用变化情况，可以用gperftools，gperftools能够绘制直观的图；

3. 如果需要分析程序的性能，可以用gperftools；

4. 如果是一个独立的测试程序（可以在PC上编译和运行），则可以用valgrind进行快速的分析（使用方便）；