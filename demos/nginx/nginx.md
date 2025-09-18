# nginx

## cmd

```bash
cd ./sbin
./nginx -p ../ -t
./nginx -p
```

## test webdav

```bash
cadaver
```

## webdav design

对于涉及到文件上传和下载的功能，直接交给fastcgi处理不是最优解。（涉及cgi的机制问题）

利用 auth_request 对请求进行权限的预处理，处理之后将文件传输任务交给nginx去做。

对于部分完全需要cgi处理的请求内容，应该通过location字段过滤。


###

```conf
server {
    listen 80;
    server_name dav.example.com;
    root /data/webdav;

    # 1. 开启WebDAV
    dav_methods PUT DELETE MKCOL COPY MOVE;
    dav_access user:rw group:rw all:r;
    create_full_put_path on;

    # 2. 定义一个内部位置用于权限检查
    location /auth {
        internal; # 标记为内部使用，外部无法访问
        fastcgi_pass 127.0.0.1:9000;
        include fastcgi_params;
        # 告诉后端这是认证请求，并传递原始请求的URI和方法
        fastcgi_param REQUEST_ORIGINAL_URI $request_uri;
        fastcgi_param REQUEST_ORIGINAL_METHOD $request_method;
        # 快速返回：后端只需返回200（OK）或401/403（Forbidden）
    }

    # 3. 对所有请求先进行认证/预检查
    location / {
        # 首先将请求代理到 /auth 进行权限验证
        auth_request /auth;
        # 如果auth_request返回2xx，则继续执行WebDAV操作（如直接处理文件上传下载）
        # 如果返回401或403，则向客户端返回错误

        # 如果是文件上传（PUT），Nginx会直接将文件写入`root`指定的目录
        # 如果是文件下载（GET），Nginx会直接从`root`目录读取文件发送
    }

    # 4. 单独处理PROPFIND等需要动态响应的请求
    location ~* (PROPFIND|OPTIONS) {
        # 这些方法不涉及文件内容流，直接交给FastCGI生成动态XML响应
        fastcgi_pass 127.0.0.1:9000;
        include fastcgi_params;
    }
}


```