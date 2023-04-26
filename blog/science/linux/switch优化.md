# switch优化

```C
#include <stdio.h>

#define P(a) printf(#a#a#a)

int test_switch(int c)
{
	switch(c)
	{
		case 0:P(0);break;
		case 1:P(1);break;
		case 2:P(2);break;
		case 3:P(3);break;
		case 4:P(4);break;
		case 5:P(5);break;
		default:P(6);break;
	}

	return 0;
}
```

```bash
gcc test.c -c
objdump -d test.o
```

```bash
0000000000000000 <test_switch>:
   0:   f3 0f 1e fa             endbr64
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   48 83 ec 10             sub    $0x10,%rsp
   c:   89 7d fc                mov    %edi,-0x4(%rbp)
   f:   83 7d fc 05             cmpl   $0x5,-0x4(%rbp)
  13:   0f 87 96 00 00 00       ja     af <test_switch+0xaf>
  19:   8b 45 fc                mov    -0x4(%rbp),%eax
  1c:   48 8d 14 85 00 00 00    lea    0x0(,%rax,4),%rdx
  23:   00
  24:   48 8d 05 00 00 00 00    lea    0x0(%rip),%rax        # 2b <test_switch+0x2b>
  2b:   8b 04 02                mov    (%rdx,%rax,1),%eax
  2e:   48 98                   cltq
  30:   48 8d 15 00 00 00 00    lea    0x0(%rip),%rdx        # 37 <test_switch+0x37>
  37:   48 01 d0                add    %rdx,%rax
  3a:   3e ff e0                notrack jmpq *%rax
  3d:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 44 <test_switch+0x44>
  44:   b8 00 00 00 00          mov    $0x0,%eax
  49:   e8 00 00 00 00          callq  4e <test_switch+0x4e>
  4e:   eb 71                   jmp    c1 <test_switch+0xc1>
  50:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 57 <test_switch+0x57>
  57:   b8 00 00 00 00          mov    $0x0,%eax
  5c:   e8 00 00 00 00          callq  61 <test_switch+0x61>
  61:   eb 5e                   jmp    c1 <test_switch+0xc1>
  63:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 6a <test_switch+0x6a>
  6a:   b8 00 00 00 00          mov    $0x0,%eax
  6f:   e8 00 00 00 00          callq  74 <test_switch+0x74>
  74:   eb 4b                   jmp    c1 <test_switch+0xc1>
  76:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 7d <test_switch+0x7d>
  7d:   b8 00 00 00 00          mov    $0x0,%eax
  82:   e8 00 00 00 00          callq  87 <test_switch+0x87>
  87:   eb 38                   jmp    c1 <test_switch+0xc1>
  89:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 90 <test_switch+0x90>
  90:   b8 00 00 00 00          mov    $0x0,%eax
  95:   e8 00 00 00 00          callq  9a <test_switch+0x9a>
  9a:   eb 25                   jmp    c1 <test_switch+0xc1>
  9c:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # a3 <test_switch+0xa3>
  a3:   b8 00 00 00 00          mov    $0x0,%eax
  a8:   e8 00 00 00 00          callq  ad <test_switch+0xad>
  ad:   eb 12                   jmp    c1 <test_switch+0xc1>
  af:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # b6 <test_switch+0xb6>
  b6:   b8 00 00 00 00          mov    $0x0,%eax
  bb:   e8 00 00 00 00          callq  c0 <test_switch+0xc0>
  c0:   90                      nop
  c1:   b8 00 00 00 00          mov    $0x0,%eax
  c6:   c9                      leaveq
  c7:   c3                      retq
```

注意这几行：

```bash
  30:   48 8d 15 00 00 00 00    lea    0x0(%rip),%rdx        # 37 <test_switch+0x37>
  37:   48 01 d0                add    %rdx,%rax
  3a:   3e ff e0                notrack jmpq *%rax
```

即利用传入的值进行计算并跳转到指定case（rdx是当前指令地址，rax是传入的值）。

如果把程序中case的值进行调整，使得间隔大一些，如下：

```C
#include <stdio.h>

#define P(a) printf(#a#a#a)

int test_switch(int c)
{
	switch(c)
	{
		case 0:P(0);break;
		case 10:P(1);break;
		case 20:P(2);break;
		case 30:P(3);break;
		case 40:P(4);break;
		case 50:P(5);break;
		default:P(6);break;
	}

	return 0;
}
```

得到的汇编如下：

```bash
0000000000000000 <test_switch>:
   0:   f3 0f 1e fa             endbr64
   4:   55                      push   %rbp
   5:   48 89 e5                mov    %rsp,%rbp
   8:   48 83 ec 10             sub    $0x10,%rsp
   c:   89 7d fc                mov    %edi,-0x4(%rbp)
   f:   83 7d fc 32             cmpl   $0x32,-0x4(%rbp)
  13:   0f 84 ab 00 00 00       je     c4 <test_switch+0xc4>
  19:   83 7d fc 32             cmpl   $0x32,-0x4(%rbp)
  1d:   0f 8f b4 00 00 00       jg     d7 <test_switch+0xd7>
  23:   83 7d fc 28             cmpl   $0x28,-0x4(%rbp)
  27:   0f 84 84 00 00 00       je     b1 <test_switch+0xb1>
  2d:   83 7d fc 28             cmpl   $0x28,-0x4(%rbp)
  31:   0f 8f a0 00 00 00       jg     d7 <test_switch+0xd7>
  37:   83 7d fc 1e             cmpl   $0x1e,-0x4(%rbp)
  3b:   74 61                   je     9e <test_switch+0x9e>
  3d:   83 7d fc 1e             cmpl   $0x1e,-0x4(%rbp)
  41:   0f 8f 90 00 00 00       jg     d7 <test_switch+0xd7>
  47:   83 7d fc 14             cmpl   $0x14,-0x4(%rbp)
  4b:   74 3e                   je     8b <test_switch+0x8b>
  4d:   83 7d fc 14             cmpl   $0x14,-0x4(%rbp)
  51:   0f 8f 80 00 00 00       jg     d7 <test_switch+0xd7>
  57:   83 7d fc 00             cmpl   $0x0,-0x4(%rbp)
  5b:   74 08                   je     65 <test_switch+0x65>
  5d:   83 7d fc 0a             cmpl   $0xa,-0x4(%rbp)
  61:   74 15                   je     78 <test_switch+0x78>
  63:   eb 72                   jmp    d7 <test_switch+0xd7>
  65:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 6c <test_switch+0x6c>
  6c:   b8 00 00 00 00          mov    $0x0,%eax
  71:   e8 00 00 00 00          callq  76 <test_switch+0x76>
  76:   eb 71                   jmp    e9 <test_switch+0xe9>
  78:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 7f <test_switch+0x7f>
  7f:   b8 00 00 00 00          mov    $0x0,%eax
  84:   e8 00 00 00 00          callq  89 <test_switch+0x89>
  89:   eb 5e                   jmp    e9 <test_switch+0xe9>
  8b:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # 92 <test_switch+0x92>
  92:   b8 00 00 00 00          mov    $0x0,%eax
  97:   e8 00 00 00 00          callq  9c <test_switch+0x9c>
  9c:   eb 4b                   jmp    e9 <test_switch+0xe9>
  9e:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # a5 <test_switch+0xa5>
  a5:   b8 00 00 00 00          mov    $0x0,%eax
  aa:   e8 00 00 00 00          callq  af <test_switch+0xaf>
  af:   eb 38                   jmp    e9 <test_switch+0xe9>
  b1:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # b8 <test_switch+0xb8>
  b8:   b8 00 00 00 00          mov    $0x0,%eax
  bd:   e8 00 00 00 00          callq  c2 <test_switch+0xc2>
  c2:   eb 25                   jmp    e9 <test_switch+0xe9>
  c4:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # cb <test_switch+0xcb>
  cb:   b8 00 00 00 00          mov    $0x0,%eax
  d0:   e8 00 00 00 00          callq  d5 <test_switch+0xd5>
  d5:   eb 12                   jmp    e9 <test_switch+0xe9>
  d7:   48 8d 3d 00 00 00 00    lea    0x0(%rip),%rdi        # de <test_switch+0xde>
  de:   b8 00 00 00 00          mov    $0x0,%eax
  e3:   e8 00 00 00 00          callq  e8 <test_switch+0xe8>
  e8:   90                      nop
  e9:   b8 00 00 00 00          mov    $0x0,%eax
  ee:   c9                      leaveq
  ef:   c3                      retq
```

可以看到运行方式发生了变化，为和if语句一样依次比较。