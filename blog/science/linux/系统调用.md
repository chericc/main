# 系统调用

## 原理

## 

```c
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```

是如何进行区分的？

```c
long syscall(long number, ...);
```