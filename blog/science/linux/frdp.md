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
serverAddr = "x.x.x.x"
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


## cfg-p2p

```bash
# frps.toml
bindPort = 17000
auth.additionalScopes = ["NewWorkConns"]
auth.method = "token"
auth.token = "123123abc"
```

```bash
# frpc.toml
serverAddr = "x.x.x.x"
serverPort = 17000
auth.additionalScopes = ["NewWorkConns"]
auth.method = "token"
auth.token = "123123abc"

[[proxies]]
name = "ssh"
type = "tcp"
localIP = "127.0.0.1"
localPort = 22
remotePort = 16000
```