# vim常用操作

本文主要用来记录 vim 中常用的操作，方便查找（尽管 vim 的帮助文档已经十分强大）。

## 浏览

- 跳转

CTRL-I 跳转到新的光标位置  
CTRL-O 跳转到旧的光标位置  
CTRL-] 跟去tag  
CTRL-T 返回tag

CTRL-E 向下滚动一行
CTRL-Y 向上滚动一行

- 状态

CTRL-G 显示当前信息


- 多文件

:split  
:vsplit  
:split filename 新开窗口并打开文件  
CTRL-W up/down 切换当前窗口  
CTRL-W +/- 改变大小  
:close  
:only 关闭其它窗口
:wall