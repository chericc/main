# netcat

Usage:

```bash

# server side
nc -l -s 127.0.0.1 -p 20000

# client side
nc 127.0.0.1 20000
```

Both server and client side will read from stdin and write to network, read from network and wirte to stdout.