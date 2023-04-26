# arm-himix200-linux- 环境使用 dmalloc 调试

## 环境

### 编译

注意：对于存在 dlopen 形式的库，必须编译成动态库版本，否则 dlopen 时，主程序没有加载，就会找不到符号

../dmalloc-5.6.5/configure \
    --prefix=$(pwd)/output \
    --host=arm-linux-gnueabi \
    --target=arm-linux-gnueabi \
    --enable-shlib \
    CC=arm-himix200-linux-gcc \
    CXX=arm-himix200-linux-g++ \
    LD=arm-himix200-linux-ld
    

编译时有报错：
```C
./dmalloc.h:377:7: error: expected identifier or '(' before '__extension__'
 char *strdup(const char *string);
       ^
./dmalloc.h:396:7: error: expected identifier or '(' before '__extension__'
 char *strndup(const char *string, const DMALLOC_SIZE max_len);
       ^
```

应该是 configure 获取 strdup 和 strndup 是否是宏的实现的时候出了问题，这里先不深究具体原因，直接
在 build 目录里的 dmalloc.h 中添加 
```
#define DMALLOC_STRDUP_MACRO
#define DMALLOC_STRNDUP_MACRO
```
即可（注意，要添加到 #undef 的后面）

make threadscxx
make installthcxx

## 注意事项

对于存在动态库的环境，最好将 dmalloc 链接到动态库中（也即动态库不能依赖主程序），同时编译时增加 fPIC 选项
