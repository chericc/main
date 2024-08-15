# frp

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
auth.method = "token"
auth.token="ccc"
```

```bash
# frpc.toml
serverAddr = "x.x.x.x"
serverPort = 17000
auth.method = "token"
auth.token="ccc"
# set up a new stun server if the default one is not available.
# natHoleStunServer = "xxx"

[[proxies]]
name = "p2p_ssh"
type = "xtcp"
secretKey = "xxx"
localIP = "127.0.0.1"
localPort = 22
```

```bash
# frpc.toml
serverAddr = "x.x.x.x"
serverPort = 17000
auth.method = "token"
auth.token="ccc"
# set up a new stun server if the default one is not available.
# natHoleStunServer = "xxx"

webServer.addr = "127.0.0.1"
webServer.port = 7400
webServer.user = "admin"
webServer.password = "admin"

transport.useEncryption = true
transport.useCompression = true

[[visitors]]
name = "p2p_ssh_visitor"
type = "xtcp"
serverName = "p2p_ssh"
secretKey = "xxx"
bindAddr = "127.0.0.1"
bindPort = 16000
# when automatic tunnel persistence is required, set it to true
keepTunnelOpen = false
```

```bash
ssh -p 16000 127.0.0.1
rsync -e "ssh -p 16000" dir 127.0.0.1://dir
```