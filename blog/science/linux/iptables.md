# iptables

## 列出

```bash

iptables --list

```

## 丢包模拟

如果要模拟设备侧丢包，可以在设备侧设置如：

```bash
# 增加丢包规则
iptables -t filter -A INPUT -m statistic --mode random --probability 0.05 -j DROP

# 修改丢包规则
iptables -t filter -R INPUT 3 -m statistic --mode random --probability 0.03 -j DROP

# 删除丢包规则
iptables -t filter -D INPUT 3

```

## 限速模拟

```bash
iptables -t filter -A INPUT -p tcp -d 192.168.1.15 -m limit --limit 40/sec --limit-burst=20 -j ACCEPT
iptables -t filter -A INPUT -p tcp -d 192.168.1.15 -j DROP
iptables -t filter -D INPUT 4
iptables -t filter -D INPUT 3
iptables -t filter -R INPUT 2 -p tcp -d 192.168.1.15 -m limit --limit 40/sec --limit-burst=20 -j ACCEPT

iptables -t filter -A INPUT -p tcp -d 192.168.1.10 -m limit --limit 40/sec --limit-burst=20 -j ACCEPT
iptables -t filter -A INPUT -p tcp -d 192.168.1.10 -j DROP
```

## 