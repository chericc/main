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

# 大小比较
if [ $log_size -gt 0 ]; then echo "bigger"; fi

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


## 分隔符调整

```bash

# 用ffprobe探测文件夹中的所有文件的信息

OLDIFS="$IFS"
IFS=$'\n'

for file in `find . -type f`
do 
echo "$file"
ffprobe "$file" -loglevel quiet -show_streams
echo ""
sleep 0.2
done

IFS="$OLDIFS"

```

## 生成时间
```bash
echo Pack time: $(date +"%Y-%m-%d %T") > version.txt
```

## 获取配置文件中的 = 后的名字

```bash
DeviceModel = IPC_NYX_JZ_ID_P10

echo "AAA_BV1 = CCC_DV123" | sed "s/.*= *\(.*\)/\1/g"
```

## 按列求和

```bash
line 761, (rtsp_ts) send seq(38691), expect seq(38690), lost num(1), sum(1302), timeout_ms(101)
line 761, (rtsp_ts) send seq(40251), expect seq(40250), lost num(1), sum(1210), timeout_ms(101)

找出其中lost num后所有数字的和

cat file | sed -E "s/.*lost num\(([^\)]+)\).*/\1/g" | awk '{sum+=$1}END{print sum}'
```

## md5sum 过滤掉路径

```bash
md5sum /home/ambarella_test | awk '{print $1," ambarella_test"}'
```

```bash
echo "74407221ab35e399e5cfb8312b3f67f4  /home/test/code//Smart-D2S-24A-P_RV1109/trunk/partition/script/../rootfs/app/usr/lib/libmcvsdk_video.so" | sed  "s/\/.*\///g"
```

## 生成 compile_commands.json 

```bash
bear make 
```

## 截取文件中的指定行

首先找出关键字所在的行

```bash
grep "log changed" logfile  -n

832317:1648548254: 4763974: -------------- log changed begin 4059359 -------------
832630:1648548254: 4763975: -------------- log changed end 4059359 -------------
1433902:1648548790: 9082410: -------------- log changed begin 6327523 -------------
1434191:1648548790: 9082411: -------------- log changed end 6327523 -------------
1831422:1648549247: 39757687: -------------- log changed begin 6327523 -------------
1831977:1648549247: 39757696: -------------- log changed end 6327523 -------------
2010277:1648549520: 56874045: -------------- log changed begin 6327523 -------------
2011182:1648549520: 56874046: -------------- log changed end 6327523 -------------
2213778:1648549827: 65926909: -------------- log changed begin 6327523 -------------
2215303:1648549827: 65927626: -------------- log changed end 6327523 -------------
2363449:1648550050: 66983134: -------------- log changed begin 6327523 -------------
2364466:1648550050: 66983135: -------------- log changed end 6327523 -------------
```

```
sed -n '2010277,2011182p' logfile > part4.txt
sed -n '2213778,2215303p' logfile > part5.txt
sed -n '2363449,2364466p' logfile > part6.txt
```

## find 查找目录，并过滤掉 svn 相关目录

```bash
find . -path *svn* -prune -o -type d
```

## 找出所有文件，并按大小排序

```bash
find ./ -type f -printf '%s %p\n' | sort -rn
```

## 找出一个目录下的全部文件个数

```bash
find . -type f | wc -l
```

## 找出一个目录下文件名中带有特定字符（比如8K）的某类型文件的总大小

```bash
find . -type f | grep -i 8k | grep -i -E "mp4$|ts$" | xargs -d "\n" ls -la | awk '{sum += $5}; END {x=1024*1024*1024;print sum/x " G";}'
```

## 分区和格式化

```bash
# 注意，分区不是必须的

# 分区
fdisk

# 格式化（建立文件系统）
mkfs.vfat -F 32 /dev/...
```

## 文件加密解密

```bash
PASS=`cat ~/pass.txt`
OUTPUT=secret-backup.tgz
SRC=secret-backup.md

# encrypt
tar --atime-preserve -zcf - $SRC | openssl des3 -k $PASS -pbkdf2 -nosalt -out $OUTPUT
md5sum $OUTPUT

# decrypt
cp -f $SRC $SRC.bak
openssl des3 -in $OUTPUT -d -k $PASS -pbkdf2 -nosalt | tar zxf -
```

## 跟踪某个文件的写入

```bash
tail -f -s 1 [file]
```

## mount tmpfs

```bash
mount tmpfs ./tmpfs -t tmpfs -o size=1G
```

## 当前时间

```bash
# 2023-09-26 08:58:35
date +"%F %T"
```

## 记录会话内容

```bash
# script会记录会话的内容
script
```

## 遍历文件夹中的文件

```bash

# 遍历所有文件，并修改封装格式(.mkv --> .mp4)
for file in `ls ./*.mkv`
do
new_name=`echo $file | sed -E "s/(.*)\.mkv/\1.mp4/g"`
echo "Processing: $file --> $new_name"
ffmpeg -i $file -codec copy $new_name
done
```

```bash
# 遍历所有文件，获取编码
IFS=$'\n'
outputfile=codec.txt
truncate -s 0 $outputfile
for file in `find . -type f | grep -E "\.mkv$|\.mp4$|\.ts"`
do
echo $file >> $outputfile
ffprobe -loglevel quiet -show_streams "$file" | grep codec_name >> $outputfile
done

```

## 7z

```bash

sudo apt-get install p7zip-full

7z a -t7z -m0=lzma -mx=1 -mfb=32 -md=256k -ms=on -mmt archive.7z dir1

```

## tmux

```bash

tmux new -s session_name

tmux ls
tmux at -t session_name

```

## 匹配单词（识别单词边界）

```bash

\b 表示单词边界

echo "abc abcd" | grep -E "\babc\b"

```

## 进程相关

```bash

# 判断某个进程是否存在

pidname=adbd
if [ -n $(pidof $pidname) ]
then 
    echo "$pidname exist"
fi

# 当前进程ID
echo $$

# 上一个运行的进程ID
echo $!


# 进程控制

# 脚本中创建的子脚本进程会因为父进程被杀而被接管，不容易查找；
# 可以通过创建标志文件控制这些进程

```

## 数字计算

```bash

# 计算
param_1=6
param_2=3
result=$(($param_1/$param_2))

```

## grep

```bash

grep abc . -rnsHa

```

## 迁移用户目录

```bash

pkill -u username
usermod -d dst_dir -m username

```