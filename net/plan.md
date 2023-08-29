# TCP封装和HTTP协议支持

## 1. 主要内容

不多不少。

### 1.1 ffmpeg/tcp.c的主要内容



### 1.2 ffmpeg/http.c的主要内容



### 1.3 当前项目对curl的使用方式

#### 1.3.1 调用方式

```C
curl_multi_init
curl_multi_wait
curl_multi_perform
curl_multi_info_read
curl_multi_cleanup
curl_multi_addhandle
curl_multi_removehandle

curl_easy_init
curl_easy_cleanup
curl_easy_getinfo
curl_easy_setopt
 - range(a - b)
 - url
 - nosignal
 - followlocation(自动跳转)
 - 连接形式
   - 长连接
     - TCP保活
     - 闲时时长
     - 保活间隔
   - 短连接
 - writefunction & writedata(自定义指针)
 - connection_timeout
 - user agent
 - fh / dns ipv6
 - fh / network priority v4 or v6
```

#### 1.3.2 需求整理

- HTTP下载
- 多路支持
- 自动跳转
- URL解析（域名解析）
- TCP保活
- 连接超时
- IPv6 DNS
- IPv6和IPv4优先级

### 1.4 要解决的痛点

当前的需求中，大部分需求均在libcurl的功能点之内。因此需要厘清本项目区别于libcurl的地方，即哪些是用libcurl来做不太好的。

| 编号 | 问题                                                         | 解决 |
| ---- | ------------------------------------------------------------ | ---- |
| 1    | 部分网络层功能有缺失。比如IPv4和IPv6的支持，包括IPv6DNS支持、IPv6和IPv4地址优先级等细化功能，直接用libcurl不好定制。 |      |
| 2    | 不能支持基于HTTP的一些上层逻辑。比如HTTP请求失败时，可能需要尝试URL对应的下一个地址，这些依赖于HTTP内容的逻辑直接用libcurl目前看不好实现（不容易控制连接的细节）。 |      |
| 3    | 协议自研。对于常用的定制化需求较强的开源库，自研实现有长期优势。需要注意模块设计要合理，避免整体框架出现大的问题。 |      |

## 2. 需求和用例的准备

### 2.1 TCP的需求和用例

#### 2.1.1 基础用例

| 序号 | 用例内容            | 备注 |
| ---- | ------------------- | ---- |
| 1    | TCP连接、读写、断开 |      |
|      |                     |      |
|      |                     |      |

### 2.2 HTTP的需求和用例

## 3. 计划

### 3.1 前期

| 编号 | 内容            | 完成时间点 |
| ---- | --------------- | ---------- |
| 1    | TCP封装接口确定 | 20230830   |
| 2    | TCP封装实现     | 20230901   |
| 3    | TCP封装自测     | 20230906   |
| 4    | TCP封装结果讨论 | 20230907   |

