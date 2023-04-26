# homebrew

## 1 install

### 1.1 进入官网复制安装命令下载

```bash
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
==> Checking for `sudo` access (which may request your password)...
Password:
==> This script will install:
/usr/local/bin/brew
/usr/local/share/doc/homebrew
/usr/local/share/man/man1/brew.1
/usr/local/share/zsh/site-functions/_brew
/usr/local/etc/bash_completion.d/brew
/usr/local/Homebrew
==> The following existing directories will be made group writable:
/usr/local/bin
==> The following existing directories will have their owner set to xxx:
/usr/local/bin
==> The following existing directories will have their group set to admin:
/usr/local/bin
==> The following new directories will be created:
/usr/local/etc
/usr/local/include
/usr/local/lib
/usr/local/sbin
/usr/local/share
/usr/local/var
/usr/local/opt
/usr/local/share/zsh
/usr/local/share/zsh/site-functions
/usr/local/var/homebrew
/usr/local/var/homebrew/linked
/usr/local/Cellar
/usr/local/Caskroom
/usr/local/Frameworks

...
```

国内安装直接安装可能会非常慢，因此最好的方法是手动下载安装脚本，然后将其中的源替换为国内的源，再进行安装。  

### 1.2 调整安装源后再安装

首先将脚本下载下来

```bash
curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh > install.sh
```

修改脚本中的几个地址，形式如下：  

```bash

#HOMEBREW_BREW_DEFAULT_GIT_REMOTE="https://github.com/Homebrew/brew"
#HOMEBREW_CORE_DEFAULT_GIT_REMOTE="https://github.com/Homebrew/homebrew-core"

HOMEBREW_BREW_DEFAULT_GIT_REMOTE="https://mirrors.ustc.edu.cn/brew.git"
HOMEBREW_CORE_DEFAULT_GIT_REMOTE="https://mirrors.ustc.edu.cn/homebrew-core.git"

```

修改完成之后进行安装：  

```bash
bash install.sh
```

### 1.3 error

```bash
---
error: RPC failed; curl 56 LibreSSL SSL_read: error:02FFF036:system library:func(4095):Connection reset by peer, errno 54

git config --global http.postBuffer 524288000
git config --global https.postBuffer 524288000
---
```

## 2 un-install

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/uninstall.sh)"
```

## 3 repo configure

相关可参考：  　

<http://mirrors.ustc.edu.cn/help/homebrew-cask.git.html>  

```bash

brew tap --custom-remote --force-auto-update homebrew/cask https://mirrors.ustc.edu.cn/homebrew-cask.git

```