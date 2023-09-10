# iperf3

## 测试TCP下最大网速

```
# server side
iperf3 -s -p 11111
```

```
# client side
# client send, server receives
iperf3 -c localhost -p 11111 -t 10000
```

## 调整发送方向

```
# normal
# client send, server receive

# reverse mode
# client receive, server send
iperf3 -c localhost -p 11111 -R -t 10000
```

## 测试给定速率下UDP的丢包率

```
iperf3 -c localhost -p 11111 -u -b 150M -t 10000
iperf3 -c localhost -p 11111 -u -b 150M -R -t 10000
```

