# Shell脚本示例

[toc]

## 输入与输出

```bash
#!/bin/bash

var_filename='date.txt'

# 输出命令的执行结果
# 注意这里的符号不是引号
echo `date`

# 输出至文件
echo `date` > ${var_filename}

# 从文件读内容
var_txt=$(cat ${var_filename})
echo ${var_txt}

# 从文件读取内容的另一个方式
var_text=`cat ${var_filename}`
echo ${var_text}
```

## 字符串测试

```bash
#!/bin/sh

STRING="1911"
STRING_2="1912"

# 字符串相同
if [ "$STRING" = "$STRING_2" ]; then echo "same string"; fi

# 字串长度为零
if [ -z "$STRING" ]; then echo "zero string"; fi

# 字符串长度非零
if [ -n "$STRING" ]; then echo "not zero string"; fi
if [ "$STRING" ]; then echo "not zero string"; fi
```

## 文件测试

```bash
#!/bin/sh

# 文件测试
TESTFILE="testfile"

# 文件
if [ -e "$TESTFILE" ]; then echo "exist"; fi

# 可读文件
if [ -r "$TESTFILE" ]; then echo "exist"; fi

# 可写文件
if [ -w "$TESTFILE" ]; then echo "exist"; fi

# 可执行文件
if [ -x "$TESTFILE" ]; then echo "exist"; fi

# 普通文件
if [ -f "$TESTFILE" ]; then echo "exist"; fi

# 文件夹
if [ -d "$TESTFILE" ]; then echo "exist"; fi

```

## 流程控制

```bash
#!/bin/sh

TEST=$1

```

## printf

```bash
#!/bin/sh

# 格式化输出

var_width=20
var_format_head="%-${var_width}s %-${var_width}s %-${var_width}s\n"
var_format_content="%-${var_width}s %-${var_width}s %-${var_width}.2f\n"

echo ${var_format_head}
echo ${var_format_content}

printf "${var_format_head}" "姓名" "性别" "战斗力"
printf "${var_format_content}" "张无忌" "男" 90
printf "${var_format_content}" "张三丰" "男" 93
printf "${var_format_content}" "杨过" "男" 90
```

## 内置变量

```bash
#!/bin/bash

# 参数的个数
echo "arg number:$#"
# 参数组合成字串
echo "args: $*"
# 当前进程
echo "pid: $$"
# 上一个进程
echo "last pid: $!"
# 当前配置
echo "option: $-"
# 上一个推出状态
echo "exit state: $?"
```

## 变量

```bash
#!/bin/bash

# 变量定义
var_ref="pwd"

# 引用变量执行命令
${var_ref}

# 引用变量执行命令
# 输出执行命令的结果
echo $(${var_ref})

# 通过指令定义
var_path=$(pwd)
echo ${var_path}

# 单引号格式按原样解析
var_name='path: $var_path'
echo ${var_name}

# 变量可以被重新赋值
# 双引号格式会解析其中的变量
var_name="path: $var_path"
echo ${var_name}
```

## 数组

```bash
#!/bin/bash

var_array_str=("A" "B" "C")

# 数组的访问
echo ${var_array_str[0]}
echo ${var_array_str[1]}
echo ${var_array_str[2]}
echo ${var_array_str[3]}

# 全部成员，两种写法等价
echo ${var_array_str[*]}
echo ${var_array_str[@]}

# 成员的遍历，以及计数
i=0
for var_str in ${var_array_str[*]}
do
    echo "array[${i}]=$var_str";
    # 变量计数的方式
    # let i+=1
    ((i++))
done
```
