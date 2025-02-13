# n2n

## build

```bash
./autogen.sh
./configure
make
```

## supernode

```bash
tools/n2n-keygen test 123
* test wY4Ayb3rOGzSO2MvRJf-DwVr-HLQ4pplvF-IZYE6JJe

--> ./community.list
```

community.list:

```bash
test_comm # this is allowed community
* test wY4Ayb3rOGzSO2MvRJf-DwVr-HLQ4pplvF-IZYE6JJe
```

```bash
./supernode -v -p 10010 -f -c com.txt -a 10.10.50.0-10.10.50.0/24 -c ./community.list
```

```

## edge

```bash
-c: community name
-l: server
-e: prefer ip
./edge -c test_comm -l 127.0.0.1:10010 -f -e 10.10.50.1 -k 123 -I test -A4 -J 123
```

## windows

happynet

高级参数：

-A4 -J 123 -k 123 -Itest

其中服务密钥对应：./edge 中的 -k