## Chapter 7 - Linking

链接是将各种代码和数据片段收集并组合成一个单一文件的过程，这个文件可被加载到内存并执行。

链接可以执行与编译时，可以执行于加载时，也可以执行于运行时。在早期的计算机系统中，链接是手动执行的，在现代系统中，链接是由叫做链接器的程序自动执行的。

链接器使得分离编译成为可能。我们不用将一个大型应用程序组织为一个巨大的源文件，而是可以把它分解为更小、更好管理的模块，可以独立地修改和编译这些模块。

理解链接器的作用：

- 帮助你构造大型程序
- 帮助避免一些危险的编程错误
- 帮助你理解语言的作用域规则是如何实现的
- 帮助你理解其他重要的系统概念，比如加载和运行程序、虚拟内存、分页、内存映射。
- 使你能够利用共享库

### 7.1 Compiler driver

大多数编译系统提供编译器驱动程序，它代表用户在需要时调用语言预处理器、编译器、汇编器、链接器。

以 GCC 为例：  

源程序如下：

```C
// file: c7_1.c

#define ADD(a,b) ((a)+(b))

int main()
{
    int a = 1;
    int b = 2;
    int c = 0;

    c = ADD(a,b);

    return c;
}
```

进行预处理

gcc -E c7_1.c -o c7_1.i

```C
// file: c7_1.i

# 1 "c7_1.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "c7_1.c"



int main()
{
    int a = 1;
    int b = 2;
    int c = 0;

    c = ((a)+(b));

    return c;
}
```

编译：

gcc -S c7_1.i -o c7_1.s

c7_1.s
```C
	.file	"c7_1.c"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$1, -12(%rbp)
	movl	$2, -8(%rbp)
	movl	$0, -4(%rbp)
	movl	-12(%rbp), %edx
	movl	-8(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.12) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
```

汇编：

gcc -C c7_1.i -o c7_1.o

生成一个模块文件 c7_1.o

链接：

