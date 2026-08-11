# 开发环境的一些配置

[toc]

## ssh

```bash
sudo apt install openssh-server
```

## samba服务

在windows下查看linux下的文件，从而实现在windows下编辑，在linux下编译的开发方式。

```bash
sudo apt-get install samba
```

修改/etc/samba/smb.conf配置文件，在文件尾追加：

```bash

# 禁止创建特定文件
veto files = /.DS_Store/._*/

[shared-name] #共享名，也是Windows上显示的名字
    path=/home/shared_dir #本地路径
    valid users=username #samba用户名
    public=yes
    writable=yes
```

然后添加samba用户：

```bash
sudo smbpasswd -a username
```

最后重启服务即可：

```bash
sudo service smbd restart
```

此时服务已开启，转到Windows下，在资源管理器地址栏按添加共享文件夹的方式添加即可。

输入用户名和密码时，为在linux端创建samba输入的用户名和密码。

新建系统用户：

先添加组：

具体的 groupid 可以参考：`cat /etc/group`。然后添加组：`groupadd username -g groupid` 。

再添加用户：

`useradd username -u userid -g groupid -s /sbin/nologin -d /dev/null`

例如

```bash
groupadd public -g 2000
useradd public -u 2000 -g 2000 -s /sbin/nologin -d /dev/null
```

## tftp服务

tftp是一个简化版的ftp，可用于向设备传送文件，适用于仅支持tftp的嵌入式设备。

安装

```bash
# 安装tftp服务端
sudo apt-get install tftpd-hpa
# 安装tftp客户端
sudo apt-get install tftp-hpa
```

服务端配置

先创建tftp目录

```bash
mkdir /tftp_dir
sudo chmod 777 /tftp_dir -R
```

配置文件修改

```bash
# /etc/default/tftpd-hpa
 
TFTP_USERNAME="tftp"
TFTP_DIRECTORY="/tftp_dir"
TFTP_ADDRESS=":69"
TFTP_OPTIONS="-l -c -s"
```

重启tftp服务

```bash
sudo service tftpd-hpa restart
```

此时搭建完成。

```bash
# 测试
# 在服务器目录上创建一个文件，并修改权限
touch /tftp_dir/test.txt
echo "Test info" > /tftp_dir/test.txt
chmod 777 /tftp_dir -R

# 使用tftp下载
tftp localhost
tftp# get test.txt
q\n

# 查看test.txt，与服务器端一致
cat test.txt
# 修改test.txt，并推送至服务端
echo "New information" > test.txt

tftp localhost
tftp# put test.txt
q\n

#检查服务端文件是否已更新
cat /tftp_dir/test.txt
```

## NFS服务

支持挂载NFS的嵌入式设备能够将开发机的目录直接挂载，便于开发调试。

安装

```bash
# server
sudo apt-get install nfs-kernel-server
# client
sudo apt install nfs-common
```

配置文件

```bash
# /etc/exports

# safe env
/home/username *(rw,sync,no_subtree_check,no_root_squash)

# not safe env
# insecure: allow client port > 1024
/home/username *(rw,async,no_subtree_check,insecure)
```

重启服务

```bash
sudo service nfs-kernel-server restart
```

测试

```bash
# cmd
sudo mount -t nfs localhost:/home/username /mnt 
# fstab
localhost:/home/username /mnt/nfs nfs 
```

## FTP服务

安装

```bash
sudo apt-get install vsftpd -y
```

配置

```bash
# /etc/vsftpd.conf
anonymous_enable=YES
anon_root=/home/test/nfs
no_anon_password=YES
write_enable=YES
anon_upload_enable=YES
anon_mkdir_write_enable=YES
```

调整权限

```bash
sudo mkdir /home/test/nfs/upload
sudo chown ftp:ftp /home/test/nfs/upload
sudo chmod 777 /home/test/nfs/upload
```

这里upload作为上传用

重启服务

```
sudo service vsftpd restart
```

使用

```
# /home/test/nfs
echo "hello" > /home/test/nfs/1.txt

# ftpget
ftpget localhost 1.txt

# ftpput
ftpput localhost upload/1.txt 1.txt

# ftp
ftp localhost
> ftp / annoymous
> get 1.txt
> exit
```

## 扩展ubuntu分区

<https://www.jianshu.com/p/383ef9e56009>

关键步骤：

```bash

LVM only

1. 在虚拟机上修改磁盘大小（扩展大小）
2. 输入 parted -l 修复分区表
3. 使用 parted 追加容量，
3.1 parted /dev/sda
3.2 p free
3.3 resizepart 3
3.4 q
4. 更新物理卷 
4.1 pvresize /dev/sda
4.2 pvdisplay
5. LVM扩容
5.1 lvdisplay
5.2 lvextend -l +100%FREE /dev/ubuntu-vg/ubuntu-lv
5.3 resize2fs /dev/ubuntu-vg/ubuntu-lv

```

## python配置

### install & update

```bash
python3 -m pip install module
python3 -m pip install --upgrade pip
```

```bash

# help
conda config --help

conda config --set auto_activate_base false
conda init --reverse $SHELL

source ~/miniconda3/bin/activate
conda init --all

# source
conda config --add channels https://mirrors.tuna.tsinghua.edu.cn/anaconda/pkgs/free/

# config
conda config --set show_channel_urls true
conda config --set auto_activate_base true

# show all configs
conda config --show

# update 
conda update --update-all

# windows/powershell
get-executionpolicy
set-executionpolicy remotesigned
set-executionpolicy restricted

# 
conda info --envs

# 
conda create -n python27 python=2.7
conda remove -n python27 --all

# conda shell
eval "$(/home/test/miniconda3/bin/conda shell.bash hook)"

# conda shell reverse
conda init --reverse $SHELL

```

### 配置pip源

```bash
python -m pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple/
python -m pip config set install.trusted-host pypi.tuna.tsinghua.edu.cn

# 配置 index-url（包下载地址）
python -m pip config set global.index-url https://mirrors.ustc.edu.cn/pypi/web/simple
```

## aria

```bash
aria2c -x 5 -s 5 -k 1M --dir=./tmp "https://..."
```

## 代理

### tinyproxy

```bash

# tiny proxy is a proxy server.

sudo apt-get install tinyproxy
sudo vim /etc/tinyproxy/tinyproxy.conf

---
Port 8888 --> Port xxx
# Allow 127.0.0.1
# Allow 192.168.1.0/24
---

service tinyproxy restart

```

```bash

# use privoxy to forward http to sock5

sudo apt-get install privoxy
forward-socks5t / 0.0.0.0:55555 . 
listen-address 0.0.0.0:55554

```

### http proxy

```bash

export http_proxy="http://127.0.0.1:7897"
export https_proxy="http://127.0.0.1:7897"

export http_proxy="http://10.0.0.3:7897"
export https_proxy="http://10.0.0.3:7897"

export http_proxy="http://10.0.0.4:7897"
export https_proxy="http://10.0.0.4:7897"

export http_proxy="http://192.168.1.235:7897"
export https_proxy="http://192.168.1.235:7897"

unset http_proxy
unset https_proxy

# proxy setting for git

# for https
git config --global http.proxy "socks5://192.168.1.203:51837"
git config --global https.proxy "socks5://192.168.1.203:51837"

# test proxy
curl --socks5 192.168.1.203:51837 www.baidu.com
curl --connect-timeout 2 -x 192.168.1.203:58591 www.baidu.com

# for ssh
Host github.com
    User git
    # linux
    ProxyCommand nc -v -x 192.168.1.203:51837 %h %p
    # macos
    ProxyCommand nc -X 5 -x 127.0.0.1:1080 %h %p
    # windows
    ProxyCommand connect -S 127.0.0.1:7890 %h %p

```

## sshfs

```bash
# /etc/fuse.conf
user_allow_other

sshfs -o allow_other work:///home/test/ ./mnt

```

## netplan

```bash

# This is the network config written by 'subiquity'
network:
  ethernets:
    enp0s3:
      optional: yes
      dhcp4: no
      addresses:
        - "192.168.1.201/24"
        - "192.168.2.30/24"
      routes:
        - to: "default"
          via: "192.168.1.1"
        - to: "192.168.2.0/24"
          via: "192.168.2.30"
      nameservers:
        addresses: [223.5.5.5]
    enp0s8:
      #dhcp4: yes
      dhcp4: no
      addresses: [192.168.2.30/24]
      optional: yes
    enp0s9:
      dhcp4: yes
      optional: yes
  version: 2

# 00-installer-config-wifi.yaml
# This is the network config written by 'subiquity'
network:
  version: 2
  wifis:
    wlo1:
      optional: true
      dhcp4: true
      access-points:
        Mi:
          password: 12345678

```

## webserver

### tomcat9

安装tomcat并启用目录浏览。
注意整个目录路径都需要有权限。

```bash

# install
sudo apt-get install tomcat9

# config
sudo vim /etc/tomcat9/server.xml

<Service >
 # config this line to configure port
 <Connector port=""> ...
 <Engine>
  <Host>
   # add this line
   <Context path="" docBase="/home/test/video" privileged="true" reloadable="true"></Context>
  </Host>
 </Engine>
</Service>

sudo vim /etc/tomcat9/web.xml

<web-app >
 <servlet>
  ...
  <init-param>
   <param-name>listings</param-name>
   # change to true make listings work.
   <param-value>true</param-value>
  </init-param>
 <servlet>
```

## proxy


## tmux

```bash
# new session
tmux new -t session-name
# attach session
tmux at -t session-name

# choose session
in tmux
ctrl+b s 
/esc

# detach session
ctrl+b d

# copy mode
crtl+b [

# search in copy mode
ctrl+f
```

```bash
# ~/.tmux.conf
set -g prefix C-b
unbind C-a
#unbind C-a

set-option -sg escape-time 500
set -g focus-events off

# 右下角类似效果：21:58:48 12-12
set -g status-right "%H:%M:%S %d-%b"

# 设置整个状态栏背景颜色 bg(背景色) fg(前景色)
set -g status-style "bg=#882244"

# 分别设置状态栏左右颜色
# set -g status-left "bg=#3a3a3a"
# set -g status-left "fg=#bcbcbc"


set -g base-index 1
set -g pane-base-index 1


set -g status-interval 1    # 状态栏刷新时间(右下角秒针会跳动)
set -g status-justify left  # 状态栏窗口列表(window list)左对齐

set -g visual-activity on # 启用活动警告
set -wg monitor-activity on # 非当前窗口有内容更新时在状态栏通知
set -g message-style "bg=#202529, fg=#91A8BA" # 指定消息通知的前景、后景色

set -wg window-status-current-format " #I:#W#F " # 状态栏当前窗口名称格式(#I：序号，#w：窗口名 称，#F：间隔符)
set -wg window-status-current-style "fg=#d7fcaf,bg=#60875f" # 状态栏当前窗口名称的样式
set -wg window-status-separator "" # 状态栏窗口名称之间的间隔



# 命令回滚/历史数量限制
set -g history-limit 20480
#set -sg escape-time 0
set -g display-time 1500
set -g remain-on-exit off
```

```bash
# 右下角类似效果：21:58:48 12-12
set -g status-right "%H:%M:%S %d-%b"

# 设置整个状态栏背景颜色 bg(背景色) fg(前景色)
set -g status-style "bg=#882244"

# 分别设置状态栏左右颜色
# set -g status-left "bg=#3a3a3a"                                                                                                                                                                                # set -g status-left "fg=#bcbcbc"


set -g base-index 1
set -g pane-base-index 1
                                                                                                                                                                                                                 
set -g status-interval 1    # 状态栏刷新时间(右下角秒针会跳动)
set -g status-justify left  # 状态栏窗口列表(window list)左对齐

set -g visual-activity on # 启用活动警告
set -wg monitor-activity on # 非当前窗口有内容更新时在状态栏通知
set -g message-style "bg=#202529, fg=#91A8BA" # 指定消息通知的前景、后景色

set -wg window-status-current-format " #I:#W#F " # 状态栏当前窗口名称格式(#I：序号，#w：窗口名 称，#F：间隔符)
set -wg window-status-current-style "fg=#d7fcaf,bg=#60875f" # 状态栏当前窗口名称的样式
set -wg window-status-separator "" # 状态栏窗口名称之间的间隔



# 命令回滚/历史数量限制
set -g history-limit 20480
set -sg escape-time 0
set -g display-time 1500
set -g remain-on-exit off
```


```bash
# 快捷键
set -g prefix C-b
unbind C-n
unbind C-a
```

```bash
# 更新配置
tmux source-file ~/.tmux.conf
```

## ubuntu

### ibus-fonts

```bash
sudo apt install chrome-gnome-shell
https://extensions.gnome.org/
ibus-tweaker
clipboard indicator
vitals
text scaler
window list
```

### server

```bash
# quicker boot up (sshd, samba)
sudo apt purge cloud-init
```

### 命令行输出切换为英文

Ubuntu 中文版默认 `LANG=zh_CN.UTF-8`，导致 git 等命令输出为中文。如需切换为英文：

先安装英文语言包：

```bash
sudo apt install language-pack-en
```

验证语言包已生成：

```bash
locale -a | grep en_US
# en_US.utf8
```

在 `~/.bashrc` 末尾追加环境变量（仅对当前用户生效，新开的终端自动生效）：

```bash
# 命令行输出使用英文
export LANG=en_US.UTF-8
```

重新加载配置：

```bash
source ~/.bashrc
```

验证：

```bash
locale
git status
```

说明：

- 只改 `~/.bashrc` 仅影响当前用户；系统级生效可执行 `sudo update-locale LANG=en_US.UTF-8`（修改 `/etc/default/locale`）
- 不装语言包直接设 `LANG=en_US.UTF-8` 会出现 `cannot change locale` 警告，因此必须先安装 `language-pack-en`
- 若不想安装语言包，也可用系统自带的 `C.UTF-8`（`export LANG=C.UTF-8`），输出同样是英文且无警告

## apache2

### configs

```bash
# /etc/apache2/apache2.conf
<Directory />
        Options FollowSymLinks
        AllowOverride None
        Require all denied
</Directory>

<Directory /usr/share>
        AllowOverride None
        Require all granted
</Directory>

<Directory /var/www/>
        Options Indexes FollowSymLinks
        AllowOverride None
        Require all granted
</Directory>

# /etc/apache2/ports.conf
Listen 10086
```

## oom

### zram-tools

```bash
# 使用 zram 压缩内存，大幅节省内存使用
sudo apt install zram-tools
```

### early-oom

## windows11

```bash
reg.exe add "HKCU\Software\Classes\CLSID\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\InprocServer32" /f /ve
```

### winmerge

```bash
d: .*nginx\\html.*
```

## gcc

```bash

可以降低编译内存使用量

add_compile_options(--param=ggc-min-expand=10)
add_compile_options(--param=ggc-min-heapsize=8192)
```

## ccache

```bash
mkdir -p ~/.ccache/
touch ~/.ccache/ccache.conf

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()
```


## opencode

### 全局配置

```bash
# ~/.config/opencode/opencode.jsonc
```

```json
{
  "$schema": "https://opencode.ai/config.json",
  "default_agent": "ask-to-edit",
  "provider": {},
  "permission": {
    "bash": {
      "git commit*": "ask"
    }
  }
}
```

### Ask To Edit 自定义 Agent

类似 Claude Code 的 "Ask To Edit" 模式，所有编辑和非只读命令都需要用户确认。

Agent 定义文件放在 `~/.config/opencode/agents/ask-to-edit.md`，文件名即 agent 名。

```bash
# ~/.config/opencode/agents/ask-to-edit.md
```

```markdown
---
description: All edits and non-readonly commands require permission before execution
mode: primary
permission:
  edit: ask
  bash:
    "*": ask
    "ls *": allow
    "pwd": allow
    "cat *": allow
    "head *": allow
    "tail *": allow
    "less *": allow
    "more *": allow
    "grep *": allow
    "find *": allow
    "which *": allow
    "whereis *": allow
    "sed -n *": allow
    "awk *": allow
    "rg *": allow
    "nl *": allow
    "tac *": allow
    "rev *": allow
    "tr *": allow
    "paste *": allow
    "column *": allow
    "join *": allow
    "fold *": allow
    "zcat *": allow
    "zgrep *": allow
    "zless *": allow
    "bzcat *": allow
    "xzcat *": allow
    "diff *": allow
    "cmp *": allow
    "stat *": allow
    "file *": allow
    "strings *": allow
    "xxd *": allow
    "od *": allow
    "hexdump *": allow
    "md5sum *": allow
    "sha256sum *": allow
    "cksum *": allow
    "readelf *": allow
    "objdump *": allow
    "nm *": allow
    "size *": allow
    "ldd *": allow
    "addr2line *": allow
    "basename *": allow
    "dirname *": allow
    "realpath *": allow
    "readlink *": allow
    "tar -tf *": allow
    "unzip -l *": allow
    "zipinfo *": allow
    "git status*": allow
    "git log *": allow
    "git diff *": allow
    "git show *": allow
    "git branch *": allow
    "git blame *": allow
    "git grep *": allow
    "git reflog": allow
    "git stash list": allow
    "git config -l": allow
    "git remote -v": allow
    "git tag": allow
    "echo *": allow
    "printf *": allow
    "wc *": allow
    "sort *": allow
    "uniq *": allow
    "cut *": allow
    "ps *": allow
    "top *": allow
    "df *": allow
    "du *": allow
    "free *": allow
    "date": allow
    "cal": allow
    "uptime": allow
    "*.env*": deny
---

You are in "Ask To Edit" mode. Before performing any file edits or executing non-readonly bash commands, you must request permission from the user.

Readonly operations (viewing files, checking git status, etc.) can be performed freely. Any operation that modifies files, creates/deletes content, or changes system state requires explicit approval.

When you need to perform a write operation, clearly explain what you intend to do and wait for the user's confirmation before proceeding.
```

注意事项：

- bash 权限规则采用 **last match wins** 策略，所以 `"*": ask` 放在最前面，具体的 `allow` 规则放在中间，兜底的 `deny`/`ask` 规则放在最后
- `"*.env*": deny`：阻止通过 bash 读取 `.env` 文件（如 `cat .env`、`grep key .env`），补上 opencode 默认 `.env` 读取保护对 bash 命令的绕过
- agent 名由文件名决定（`ask-to-edit.md` → `ask-to-edit`），frontmatter 中不需要 `name` 字段
- 已设置为默认 agent，启动 opencode 时自动使用

### Auto Edit 自定义 Agent

类似 Claude Code 的 "Edit Automatically" 模式，文件编辑自动执行，但非只读 bash 命令需要用户确认。

Agent 定义文件放在 `~/.config/opencode/agents/auto-edit.md`。

```bash
# ~/.config/opencode/agents/auto-edit.md
```

```markdown
---
description: Edits files automatically, but non-readonly bash commands require permission
mode: primary
permission:
  edit: allow
  bash:
    "*": ask
    "ls *": allow
    "pwd": allow
    "cat *": allow
    "head *": allow
    "tail *": allow
    "less *": allow
    "more *": allow
    "grep *": allow
    "find *": allow
    "which *": allow
    "whereis *": allow
    "sed -n *": allow
    "awk *": allow
    "rg *": allow
    "nl *": allow
    "tac *": allow
    "rev *": allow
    "tr *": allow
    "paste *": allow
    "column *": allow
    "join *": allow
    "fold *": allow
    "zcat *": allow
    "zgrep *": allow
    "zless *": allow
    "bzcat *": allow
    "xzcat *": allow
    "diff *": allow
    "cmp *": allow
    "stat *": allow
    "file *": allow
    "strings *": allow
    "xxd *": allow
    "od *": allow
    "hexdump *": allow
    "md5sum *": allow
    "sha256sum *": allow
    "cksum *": allow
    "readelf *": allow
    "objdump *": allow
    "nm *": allow
    "size *": allow
    "ldd *": allow
    "addr2line *": allow
    "basename *": allow
    "dirname *": allow
    "realpath *": allow
    "readlink *": allow
    "tar -tf *": allow
    "unzip -l *": allow
    "zipinfo *": allow
    "git status*": allow
    "git log *": allow
    "git diff *": allow
    "git show *": allow
    "git branch *": allow
    "git blame *": allow
    "git grep *": allow
    "git reflog": allow
    "git stash list": allow
    "git config -l": allow
    "git remote -v": allow
    "git tag": allow
    "echo *": allow
    "printf *": allow
    "wc *": allow
    "sort *": allow
    "uniq *": allow
    "cut *": allow
    "ps *": allow
    "top *": allow
    "df *": allow
    "du *": allow
    "free *": allow
    "date": allow
    "cal": allow
    "uptime": allow
    "*.env*": deny
---

You are in "Auto Edit" mode. File edits are performed automatically without asking for permission.

Readonly bash operations (viewing files, checking git status, etc.) can be performed freely. Any bash command that modifies files, creates/deletes content, or changes system state requires explicit approval from the user.

When you need to perform a non-readonly bash operation, clearly explain what you intend to do and wait for the user's confirmation before proceeding.
```

与 Ask To Edit 模式的区别：

| 模式 | 文件编辑 | Bash 命令 |
|------|----------|-----------|
| Ask To Edit | 需要确认 | 只读命令自动，其他需要确认 |
| Auto Edit | 自动执行 | 只读命令自动，其他需要确认 |

切换方式：启动时 `opencode --agent auto-edit`

### 清理配置与数据

不卸载 opencode 本身，仅清除其配置、数据和缓存，恢复到全新状态。清理前先退出所有正在运行的 opencode 进程。

```bash
# 1. 全局配置（opencode.jsonc、agents/、commands/、skills/、插件依赖 node_modules/ 等）
rm -rf ~/.config/opencode

# 2. 用户数据（登录凭证 auth.json、会话数据库 opencode.db、日志 log/、仓库缓存 repos/）
rm -rf ~/.local/share/opencode

# 3. 缓存（升级用的二进制缓存 bin/）
rm -rf ~/.cache/opencode
```

注意事项：

- 清理后所有登录凭证（API key 等）丢失，需重新执行 `opencode auth login`
- 会话历史、消息记录存储在 `~/.local/share/opencode/opencode.db`（SQLite），删除后不可恢复
- `~/.config/opencode` 下的 `node_modules/` 是 opencode 自动安装的插件依赖，无需手动保留
- `~/.agents/skills`、`~/.claude/skills` 属于外部技能目录，并非 opencode 独有，仅在确定不需要时一并清理

## zsh

### Tab 补全行为优化

**问题：** 默认情况下，zsh 开启了 `AUTO_MENU`，导致按两次 Tab 后会进入菜单选择模式并自动填入第一个匹配项。如果路径不对，还需要手动删除。

**解决：** 在 `~/.zshrc` 中添加：

```zsh
unsetopt AUTO_MENU
```

**效果：**
- 第一次 Tab：补全到最长公共前缀
- 第二次 Tab：列出所有匹配项（不会自动填入）
- 可以继续输入字符缩小范围，再按 Tab 进一步补全

## Ghostty 配置

Ghostty 是 macOS 上的终端模拟器。配置文件路径：

```bash
~/.config/ghostty/config.ghostty
```

当前生效的配置：

```ini
# 将 CJK 汉字及标点映射到系统字体 PingFang SC，保持默认英文字体不变
font-codepoint-map = U+4E00-U+9FFF=PingFang SC   # CJK 统一表意文字（汉字）
font-codepoint-map = U+3000-U+303F=PingFang SC   # CJK 符号和标点（。、「」等）
font-codepoint-map = U+FF00-U+FFEF=PingFang SC   # 全角形式（，：；！？等）
font-codepoint-map = U+2000-U+206F=PingFang SC   # 通用标点（—破折号、…省略号等）
font-codepoint-map = U+3400-U+4DBF=PingFang SC   # CJK 扩展A（生僻字/姓名用字）
```

配置修改后按 `cmd + shift + ,` 热加载。

### SSH terminfo

通过 SSH 连接远程服务器（如 Ubuntu）时，如果出现 `missing or unsuitable terminal: xterm-ghostty` 警告，
说明远程系统缺少 Ghostty 的 terminfo 条目。可以使用以下命令将本地的 terminfo 复制到远程服务器：

```bash
infocmp -x xterm-ghostty | ssh USER@SERVER -- tic -x -
```

注意事项：

- **macOS 版本要求**：macOS Sonoma (14.x) 及以上的系统自带的 `infocmp` 可用。如果使用 macOS Ventura (13.x) 或更早版本，
  需要先通过 Homebrew 安装较新版本的 ncurses：
  ```bash
  brew install ncurses
  ```
  然后使用完整路径调用：
  ```bash
  /opt/homebrew/opt/ncurses/bin/infocmp -x xterm-ghostty | ssh USER@SERVER -- tic -x -
  ```

- 如果远程 `tic` 提示 `older tic versions may treat the description field as an alias`，可以忽略。

- `tic` 默认写入系统数据库 `/usr/share/terminfo`，如果没有写权限则回退到 `$HOME/.terminfo`。

- 如果远程 ncurses 版本 >= 6.5-20241228，则系统已自带 `xterm-ghostty` 条目，无需手动安装。