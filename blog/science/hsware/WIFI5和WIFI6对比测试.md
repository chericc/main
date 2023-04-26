# WIFI5和WIFI6对比测试

## 环境

- 设备驱动均为系统最新驱动（20201209）
- 路由器：TL-XDR1850易展版
- WIFI5设备：macbook pro 2019,13'
- WIFI6设备：intel WIFI 6 ax200，windows 10 ubuntu(WSL)

## 延迟

- 摘取可见的最小值，最大值，频繁值。

- ping路由器lan地址
  - 空载（）
    - `WIFI6,5G(1.16ms,1.58ms,1.30ms)`
    - `WIFI5,5G(2.089ms,9.661ms,8.9ms)`
  - 被观察WIFI6负载（下行1MB/s，上行500KB/s，均值）
    - `WIFI6,5G(1.05ms,43.9ms,2ms)`
    - `WIFI5,5G(3.173ms,10.533ms,9.0ms)`
  - 被观察WIFI5负载（下行1MB/s，上行500KB/s，均值）
    - `WIFI6,5G(1.23ms,24.5ms,1.3ms)`
    - `WIFI5,5G(1.773ms,5.231ms,2ms)`
