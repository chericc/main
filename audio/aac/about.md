# aac

## license

https://www.via-corp.com/licensing/aac/aac-faqs/

What technologies are covered under the AAC patent licensing program?

The license encompasses AAC Low Complexity (AAC-LC), High Efficiency AAC (HE-AAC, also sometimes known as aacPlus), MPEG-4 HE-AAC v2, xHE-AAC (Extended HE-AAC), and MPEG-D DRC (Dynamic Range Control). Also included in the license are patents essential to MPEG-4 AAC Scalable, MPEG-4 Low Delay AAC Profile (AAC-LD), MPEG-4 ER AAC ELD (AAC-ELD) and AAC-ELD v2. The license provides comprehensive coverage of those aspects of AAC that are in commercial practice. The License grants licensees the right to use the licensed patents solely for the purpose of practicing these technologies and not for use with any other audio technology.

AAC-LC
HE-AAC
MPEG-4 HE-AAC v2
xHE-AAC
MPEG-D DRC
MPEG-4 AAC Scalable
AAC-LD
AAC-ELD
AAC-ELD v2

MPEG-2 AAC-LC


https://blog.csdn.net/leixiaohua1020/article/details/84481515

MPEG-2 

https://www.mpegla.com/programs/mpeg-2/patent-list/

Please note that the last US patent expired February 13, 2018, and patents are presently active in Malaysia.

## compare

https://avidemux.org/smif/index.php?topic=19615.0

## fdk-aac

### compile fdk-aac

../fdk-aac-2.0.2/configure --enable-example --prefix=$(pwd)/output

直接用静态库编译（stripped）：

release 
root@xjl:~/code/log/aac# ls release/demo  -lah
-rwxr-xr-x 1 root root 619K Jul 25 14:25 release/demo

### decrease size 1

../fdk-aac-2.0.2/configure \
    --enable-shared=no --enable-example \
    --prefix=$(pwd)/output \
    CFLAGS="-ffunction-sections -fdata-sections" \
    CXXFLAGS="-ffunction-sections -fdata-sections"

g++ -o release/demo  release/./aac_fdkaac.o release/./main.o  -O2 -Wall -Ifdk_aac/include/fdk-aac -ffunction-sections -fdata-sections -Wl,--gc-sections -L. -lpthread -L./fdk_aac/lib -lfdk-aac
strip release/demo
root@xjl:~/code/log/aac#
root@xjl:~/code/log/aac#
root@xjl:~/code/log/aac# ls release/demo  -lah
-rwxr-xr-x 1 root root 771K Jul 25 14:48 release/demo

也即，无效（有反作用）。

最终：

../fdk-aac-2.0.2/configure \
    --enable-shared=no --enable-example \
    --prefix=$(pwd)/output

### decrease size 2

none

### cross-compile

../fdk-aac-2.0.2/configure \
    --enable-example \
    --prefix=$(pwd)/output \
    --host=arm-buildroot-linux-uclibcgnueabihf \
    CC=arm-buildroot-linux-uclibcgnueabihf-gcc \
    CXX=arm-buildroot-linux-uclibcgnueabihf-g++ \
    LD=arm-buildroot-linux-uclibcgnueabihf-ld

### fdk-aac demo


timestamp[mydemo.cpp 211 main] [2022-07-25 05:46:29.209] in:640 bytes, out: 768 bytes
timestamp[mydemo.cpp 34 dump_parameters] [2022-07-25 05:46:29.210] handle=0x55b8acb482a0,in:[numBufs=1,bufs={0x55b8acb62be0},bufferid=0,bufsize=640,bufEleSize=2]out:[numBufs=1,bufs={0x7fffbd92aab0},bufferid=3,bufsize=20480,bufEleSize=1]inarg:[numInSamples=320,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]
timestamp[mydemo.cpp 211 main] [2022-07-25 05:46:29.210] in:640 bytes, out: 0 bytes
timestamp[mydemo.cpp 34 dump_parameters] [2022-07-25 05:46:29.210] handle=0x55b8acb482a0,in:[numBufs=1,bufs={0x55b8acb62be0},bufferid=0,bufsize=640,bufEleSize=2]out:[numBufs=1,bufs={0x7fffbd92aab0},bufferid=3,bufsize=20480,bufEleSize=1]inarg:[numInSamples=320,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]
timestamp[mydemo.cpp 211 main] [2022-07-25 05:46:29.210] in:640 bytes, out: 0 bytes
timestamp[mydemo.cpp 34 dump_parameters] [2022-07-25 05:46:29.210] handle=0x55b8acb482a0,in:[numBufs=1,bufs={0x55b8acb62be0},bufferid=0,bufsize=640,bufEleSize=2]out:[numBufs=1,bufs={0x7fffbd92aab0},bufferid=3,bufsize=20480,bufEleSize=1]inarg:[numInSamples=320,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]
timestamp[mydemo.cpp 211 main] [2022-07-25 05:46:29.210] in:640 bytes, out: 0 bytes
timestamp[mydemo.cpp 34 dump_parameters] [2022-07-25 05:46:29.210] handle=0x55b8acb482a0,in:[numBufs=1,bufs={0x55b8acb62be0},bufferid=0,bufsize=640,bufEleSize=2]out:[numBufs=1,bufs={0x7fffbd92aab0},bufferid=3,bufsize=20480,bufEleSize=1]inarg:[numInSamples=320,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]


timestamp[aac_fdkaac.cpp 229 Encode] [2022-07-25 05:48:13.025] Encode result: input=64 B, output=768 B
timestamp[main.cpp 68 run] [2022-07-25 05:48:13.025] in: 64 bytes, out: 768 bytes
timestamp[aac_fdkaac.cpp 34 dump_parameters] [2022-07-25 05:48:13.025] handle=0x55add5ec0f90,in:[numBufs=1,bufs={0x55add5edb8f0},bufferid=0,bufsize=64,bufEleSize=2]out:[numBufs=1,bufs={0x7ffe36e2f4d0},bufferid=3,bufsize=8192,bufEleSize=1]inarg:[numInSamples=1024,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]
timestamp[aac_fdkaac.cpp 229 Encode] [2022-07-25 05:48:13.025] Encode result: input=64 B, output=768 B
timestamp[main.cpp 68 run] [2022-07-25 05:48:13.025] in: 64 bytes, out: 768 bytes
timestamp[aac_fdkaac.cpp 34 dump_parameters] [2022-07-25 05:48:13.025] handle=0x55add5ec0f90,in:[numBufs=1,bufs={0x55add5edb8f0},bufferid=0,bufsize=64,bufEleSize=2]out:[numBufs=1,bufs={0x7ffe36e2f4d0},bufferid=3,bufsize=8192,bufEleSize=1]inarg:[numInSamples=1024,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]
timestamp[aac_fdkaac.cpp 229 Encode] [2022-07-25 05:48:13.025] Encode result: input=64 B, output=768 B
timestamp[main.cpp 68 run] [2022-07-25 05:48:13.025] in: 64 bytes, out: 768 bytes
timestamp[aac_fdkaac.cpp 34 dump_parameters] [2022-07-25 05:48:13.025] handle=0x55add5ec0f90,in:[numBufs=1,bufs={0x55add5edb8f0},bufferid=0,bufsize=64,bufEleSize=2]out:[numBufs=1,bufs={0x7ffe36e2f4d0},bufferid=3,bufsize=8192,bufEleSize=1]inarg:[numInSamples=1024,numAncBytes=0]outarg:[numOutBytes=0,numInsamples=0,numAncBytes=0,bitResState=0]


注意：要把 AACENC_InArgs 中的值和实际输入的数据大小对应起来。

## faac

### compile

../faac-1_30/configure --prefix=$(pwd)/output 

../faac_reducemem_bak/configure \
    --prefix=$(pwd)/output \
    --host=arm-linux-uclibceabi \
    CC=arm-gcc6.3-linux-uclibceabi-gcc \
    CXX=arm-gcc6.3-linux-uclibceabi-g++ \
    LD=arm-gcc6.3-linux-uclibceabi-ld

../faac-1_30/configure \
    --prefix=$(pwd)/output \
    --host=arm-linux-uclibceabi \
    CC=arm-gcc6.3-linux-uclibceabi-gcc \
    CXX=arm-gcc6.3-linux-uclibceabi-g++ \
    LD=arm-gcc6.3-linux-uclibceabi-ld

### 内存优化

使用动态库形式，优化前：

--------------------------------------------------------------------------------
  n        time(i)         total(B)   useful-heap(B) extra-heap(B)    stacks(B)
--------------------------------------------------------------------------------
  0              0                0                0             0            0
  1      3,428,149       12,192,760       12,188,948         3,812            0
99.97% (12,188,948B) (heap allocation functions) malloc/new/new[], --alloc-fns, etc.
->98.77% (12,042,996B) 0x4843FD6: faacEncOpen (in /root/code/log/aac/faac_lib/lib/libfaac.so.0.0.0)
| ->98.77% (12,042,996B) 0x10A96A: AAC::AacFaac::Init() (aac_faac.cpp:115)
|   ->98.77% (12,042,996B) 0x10C924: run(char const*, char const*) (main.cpp:48)
|     ->98.77% (12,042,996B) 0x10D19C: main (main.cpp:176)

使用动态库形式，调整最大通道支持为 64 --> 1

peak:
408,392

99.81% (407,616B) (heap allocation functions) malloc/new/new[], --alloc-fns, etc.
->46.20% (188,664B) 0x4843FE2: faacEncOpen (in /root/code/log/aac/faac_lib/lib/libfaac.so.0.0.0)
| ->46.20% (188,664B) 0x10A96A: AAC::AacFaac::Init() (aac_faac.cpp:115)
|   ->46.20% (188,664B) 0x10C924: run(char const*, char const*) (main.cpp:48)
|     ->46.20% (188,664B) 0x10D19C: main (main.cpp:176)


Number of snapshots: 55
 Detailed snapshots: [1 (peak), 10, 11, 23, 26, 36]

--------------------------------------------------------------------------------
  n        time(i)         total(B)   useful-heap(B) extra-heap(B)    stacks(B)
--------------------------------------------------------------------------------
  0              0                0                0             0            0
  1      4,812,491          407,920          407,230           690            0


再将 coder.h 中的 bwpInfo 成员去掉，

Number of snapshots: 55
 Detailed snapshots: [1 (peak), 10, 11, 23, 26, 36]

--------------------------------------------------------------------------------
  n        time(i)         total(B)   useful-heap(B) extra-heap(B)    stacks(B)
--------------------------------------------------------------------------------
  0              0                0                0             0            0
  1      4,812,478          248,160          247,470           690            0
99.72% (247,470B) (heap allocation functions) malloc/new/new[], --alloc-fns, etc.
->29.30% (72,704B) 0x4902A99: ??? (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28)
| ->29.30% (72,704B) 0x4011B99: call_init.part.0 (dl-init.c:72)
|   ->29.30% (72,704B) 0x4011CA0: call_init (dl-init.c:30)
|     ->29.30% (72,704B) 0x4011CA0: _dl_init (dl-init.c:119)
|       ->29.30% (72,704B) 0x4001139: ??? (in /usr/lib/x86_64-linux-gnu/ld-2.31.so)
|         ->29.30% (72,704B) 0x2: ???
|           ->29.30% (72,704B) 0x1FFF0006CA: ???
|             ->29.30% (72,704B) 0x1FFF0006D5: ???
|               ->29.30% (72,704B) 0x1FFF0006DF: ???
|
->13.20% (32,768B) 0x4844702: faacEncEncode (in /root/code/log/aac/faac_lib/lib/libfaac.so.0.0.0)
| ->13.20% (32,768B) 0x10B4F5: AAC::FAACHelper::encodeFrame(AAC::AudioData const*, AAC::InputArgs const*, AAC::AudioData*, AAC::OutputArgs*) (aac_faac.cpp:401)
|   ->13.20% (32,768B) 0x10B342: AAC::FAACHelper::encode(AAC::AudioData const*, AAC::InputArgs const*, AAC::AudioData*, AAC::OutputArgs*) (aac_faac.cpp:354)
|     ->13.20% (32,768B) 0x10CD85: run(char const*, char const*) (main.cpp:119)
|       ->13.20% (32,768B) 0x10CF68: main (main.cpp:169)
|
->11.65% (28,904B) 0x4843FE2: faacEncOpen (in /root/code/log/aac/faac_lib/lib/libfaac.so.0.0.0)
| ->11.65% (28,904B) 0x10A99F: AAC::FAACHelper::init() (aac_faac.cpp:122)
|   ->11.65% (28,904B) 0x10C92F: run(char const*, char const*) (main.cpp:48)
|     ->11.65% (28,904B) 0x10CF68: main (main.cpp:169)

可以看到 faacEncOpen 对应的内存占用再次下降。

