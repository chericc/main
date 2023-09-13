# iperf3

注：使用iperf进行测试时，注意发送端和接收端的版本要一致；
当前已经发现，使用同一个服务端的情况下，不同版本的客户端测试时，UDP丢包率不一致；

## 测试TCP下最大网速

```
# server side
iperf3 -s -p 11111
```

```
# client side
# client --> server
iperf3 -c localhost -p 11111 -t 10000
```

## 调整发送方向

```
# normal
# client --> server

# reverse mode
# server --> client
iperf3 -c localhost -p 11111 -R -t 10000
```

## 测试给定速率下UDP的丢包率

```
iperf3 -c localhost -p 11111 -u -b 150M -t 10000
iperf3 -c localhost -p 11111 -u -b 150M -R -t 10000
```

