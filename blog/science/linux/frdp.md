# frdp

## site

```bash
https://github.com/fatedier/frp
```

## cfg

```bash

# bridge server

# frps.toml
bindPort = 17000

./frps -c ./frps.toml

# target host

# frpc.toml
serverAddr = "39.107.190.156"
serverPort = 17000

[[proxies]]
name = "ssh"
type = "tcp"
localIP = "127.0.0.1"
localPort = 22
remotePort = 16000

./frpc -c ./frpc.toml

# access target host

ssh -p 16000 -l test x.x.x.x

```
