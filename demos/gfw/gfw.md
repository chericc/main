# gfw

## singbox

```bash


curl -fsSL https://sing-box.app/install.sh | bash



```

## cert

```bash
apt install -y certbot
certbot certonly --standalone -d example.cn
/etc/letsencrypt/live/example.cn/cert.pem
/etc/letsencrypt/live/example.cn/privkey.pem

```

## singbox-config

```json
{
  "log": {
    "level": "info"
  },
  "inbounds": [
    {
      "type": "trojan",
      "listen": "::",
      "listen_port": 8888,
      "users": [
        {
          "name": "test",
          "password": "test"
        }
      ],
      "tls": {
        "enabled": true,
        "server_name": "example.cn",
        "key_path": "/etc/letsencrypt/live/example.cn/privkey.pem",
        "certificate_path": "/etc/letsencrypt/live/example.cn/fullchain.pem"
      },
      "multiplex": {
        "enabled": true
      }
    }
  ]
}

```

## client

```json
port: 7890
socks-port: 7891
allow-lan: false
mode: rule
log-level: silent
external-controller: 127.0.0.1:9090
secret: ""
dns:
  enable: true
  ipv6: false
  nameserver:
    - 223.5.5.5
    - 180.76.76.76
    - 119.29.29.29
    - 117.50.11.11
    - 117.50.10.10
    - 114.114.114.114
    - https://dns.alidns.com/dns-query
    - https://doh.360.cn/dns-query
  fallback:
    - 8.8.8.8
    - tls://dns.rubyfish.cn:853
    - tls://1.0.0.1:853
    - tls://dns.google:853
    - https://dns.rubyfish.cn/dns-query
    - https://cloudflare-dns.com/dns-query
    - https://dns.google/dns-query
  fallback-filter:
    geoip: true
    ipcidr:
      - 240.0.0.0/4
      - 0.0.0.0/32
      - 127.0.0.1/32
    domain:
      - +.google.com
      - +.facebook.com
      - +.youtube.com
      - +.xn--ngstr-lra8j.com
      - +.google.cn
      - +.googleapis.cn
      - +.gvt1.com
proxies:
  - 
    name: 123
    type: trojan
    server: example.cn
    port: 8888
    password: test
    alpn:
      - h2
      - http/1.1
    skip-cert-verify: false
```