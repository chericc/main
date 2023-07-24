# 一些脚本命令

## 生成时间
```bash
echo Pack time: $(date +"%Y-%m-%d %T") > version.txt
```

## 获取配置文件中的 = 后的名字

```bash
DeviceModel = IPC_NYX_JZ_ID_P10

echo "AAA_BV1 = CCC_DV123" | sed "s/.*= *\(.*\)/\1/g"
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

## 传参问题

关于函数传参时参数是否可能被转换的问题（double --> int）

拷贝形式的参数传参，没有问题；
指针形式的参数传参，注意这里存在默认的指针类型转换（编译器可能只是警告），存在问题。

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


tar --atime-preserve -zcf - $SRC | openssl des3 -k $PASS -pbkdf2 -nosalt -out $OUTPUT
md5sum $OUTPUT


cp -f $SRC $SRC.bak
openssl des3 -in $OUTPUT -d -k $PASS -pbkdf2 -nosalt | tar zxf -
```

## 跟踪某个文件的写入

```bash
tail -f -s 1 [file]
```

