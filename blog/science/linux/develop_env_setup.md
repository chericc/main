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

> 本文档此前按 opencode v1 编写，本机当前为 opencode v2（命令名由 `opencode` 变为 `opencode2`，
> 且可同时安装 v1 与 v2）。以下内容已按 v2 语法更新，与 v2 文档（https://opencode.ai/v2/docs）核对：
>
> - 配置路径不变：`~/.config/opencode/opencode.json(c)` 仍为全局配置；`~/.config/opencode/agents/*.md` 仍为 agent 定义文件
> - v1 的 `provider` → v2 的 `providers`（复数）
> - v1 的 `permission`（按工具分组的对象）→ v2 的 `permissions`（有序规则数组），规则格式为 `{ action, resource, effect }`
> - v1 的 `bash` 动作名 → v2 的 `shell`；`*.env*` 防护规则同样从 `permission.bash` 迁移为 `permissions` 中的 `action: shell` 规则
> - 规则匹配策略不变：均为 **last match wins**，宽泛规则在前、例外规则在后
> - v2 会自动翻译 v1 旧格式的配置与 agent frontmatter（可兼容），但新写配置建议直接用 v2 语法

### 全局配置

```bash
# ~/.config/opencode/opencode.jsonc
```

```jsonc
{
  "$schema": "https://opencode.ai/config.json",
  "default_agent": "ask-to-edit",
  "providers": {},
  "permissions": [
    {
      "action": "shell",
      "resource": "git config*",
      "effect": "deny"
    },
    {
      "action": "shell",
      "resource": "git commit*",
      "effect": "ask"
    }
  ]
}
```

### 默认编辑器（EDITOR）

opencode 及 git 等命令行工具在需要打开外部编辑器时（如交互式编辑提交说明），读取 `EDITOR` 环境变量；未设置时通常回退到系统默认（Ubuntu 为 nano）。

在 `~/.bashrc` 末尾追加：

```bash
# 默认编辑器使用 vim
export EDITOR=vim
```

重新加载并验证：

```bash
source ~/.bashrc

echo $EDITOR
# vim
```

说明：

- 只改 `~/.bashrc` 仅影响当前用户；zsh 用户写入 `~/.zshrc`
- 系统级默认编辑器可通过 `sudo update-alternatives --config editor` 修改（Debian/Ubuntu）
- 若设置了 `VISUAL` 变量，多数程序优先取 `VISUAL`，建议两者保持一致（`export VISUAL=vim`）

### 自定义 Provider + Model

配置自定义 AI 提供商及其模型的限制和思考等级。

```bash
# ~/.config/opencode/opencode.json
```

```jsonc
{
  "$schema": "https://opencode.ai/config.json",
  "provider": {
    "scnet": {
      "npm": "@ai-sdk/openai-compatible",
      "name": "SCNet",
      "options": {
        "baseURL": "https://api.scnet.cn/api/llm/v1",
        "apiKey": "sk-tp-xxxx"
      },
      "models": {
        "DeepSeek-V4-Flash-0731": {
          "name": "DeepSeek-V4-Flash-0731",
          "limit": {
            "context": 1000000,
            "output": 384000
          },
          "options": {
            "reasoningEffort": "high"
          }
        }
      }
    }
  },
  "model": "DeepSeek-V4-Flash-0731"
}
```

关键配置说明：

- `limit.context`: 最大输入上下文（tokens），DeepSeek V4 Flash 为 1M
- `limit.output`: 最大输出长度（tokens），DeepSeek V4 Flash 为 384K
- `options.reasoningEffort`: 思考强度等级，DeepSeek V4 Flash 支持 `low` / `high` / `max`，默认 `high`

参考：
- [DeepSeek API 文档](https://api-docs.deepseek.com/zh-cn/quick_start/pricing)

### 刷新内置模型列表（models.dev 缓存）

opencode 内置的是它发布时打包的 models.dev 快照，不会实时同步。当 TUI 的 switch model 菜单看不到 models.dev 上已经有的新模型（如 `ollama-cloud/glm-5.3-flash`）时，即使命令行里能查到，菜单也可能不显示，可用 `--refresh` 强制刷新缓存：

```bash
# 查看某个 provider 下所有模型（含详细元数据），并强制刷新 models.dev 缓存
opencode models ollama-cloud --refresh --verbose
```

说明：

- `--refresh`：重新拉取 models.dev 的模型数据（内置快照 → 最新数据）
- `--verbose`：输出模型完整元数据（cost、limit、capabilities、api 端点等），便于排查
- 若刷新后 TUI 菜单仍不显示，直接在该 provider 的 `models` 里手动补全（见上节"自定义 Provider + Model"）
- 本机为 opencode v2 时，命令名用 `opencode2 models ollama-cloud --refresh --verbose`

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
permissions:
  - action: edit
    resource: "*"
    effect: ask
  - action: shell
    resource: "*"
    effect: ask
  - action: shell
    resource: "ls *"
    effect: allow
  - action: shell
    resource: "pwd"
    effect: allow
  - action: shell
    resource: "cat *"
    effect: allow
  - action: shell
    resource: "head *"
    effect: allow
  - action: shell
    resource: "tail *"
    effect: allow
  - action: shell
    resource: "less *"
    effect: allow
  - action: shell
    resource: "more *"
    effect: allow
  - action: shell
    resource: "grep *"
    effect: allow
  - action: shell
    resource: "find *"
    effect: allow
  - action: shell
    resource: "find /"
    effect: ask
  - action: shell
    resource: "find / *"
    effect: ask
  - action: shell
    resource: "which *"
    effect: allow
  - action: shell
    resource: "whereis *"
    effect: allow
  - action: shell
    resource: "sed -n *"
    effect: allow
  - action: shell
    resource: "awk *"
    effect: allow
  - action: shell
    resource: "rg *"
    effect: allow
  - action: shell
    resource: "nl *"
    effect: allow
  - action: shell
    resource: "tac *"
    effect: allow
  - action: shell
    resource: "rev *"
    effect: allow
  - action: shell
    resource: "tr *"
    effect: allow
  - action: shell
    resource: "paste *"
    effect: allow
  - action: shell
    resource: "column *"
    effect: allow
  - action: shell
    resource: "join *"
    effect: allow
  - action: shell
    resource: "fold *"
    effect: allow
  - action: shell
    resource: "zcat *"
    effect: allow
  - action: shell
    resource: "zgrep *"
    effect: allow
  - action: shell
    resource: "zless *"
    effect: allow
  - action: shell
    resource: "bzcat *"
    effect: allow
  - action: shell
    resource: "xzcat *"
    effect: allow
  - action: shell
    resource: "diff *"
    effect: allow
  - action: shell
    resource: "cmp *"
    effect: allow
  - action: shell
    resource: "stat *"
    effect: allow
  - action: shell
    resource: "file *"
    effect: allow
  - action: shell
    resource: "strings *"
    effect: allow
  - action: shell
    resource: "xxd *"
    effect: allow
  - action: shell
    resource: "od *"
    effect: allow
  - action: shell
    resource: "hexdump *"
    effect: allow
  - action: shell
    resource: "md5sum *"
    effect: allow
  - action: shell
    resource: "sha256sum *"
    effect: allow
  - action: shell
    resource: "cksum *"
    effect: allow
  - action: shell
    resource: "readelf *"
    effect: allow
  - action: shell
    resource: "objdump *"
    effect: allow
  - action: shell
    resource: "nm *"
    effect: allow
  - action: shell
    resource: "size *"
    effect: allow
  - action: shell
    resource: "ldd *"
    effect: allow
  - action: shell
    resource: "addr2line *"
    effect: allow
  - action: shell
    resource: "basename *"
    effect: allow
  - action: shell
    resource: "dirname *"
    effect: allow
  - action: shell
    resource: "realpath *"
    effect: allow
  - action: shell
    resource: "readlink *"
    effect: allow
  - action: shell
    resource: "tar -tf *"
    effect: allow
  - action: shell
    resource: "unzip -l *"
    effect: allow
  - action: shell
    resource: "zipinfo *"
    effect: allow
  - action: shell
    resource: "git status*"
    effect: allow
  - action: shell
    resource: "git log *"
    effect: allow
  - action: shell
    resource: "git diff *"
    effect: allow
  - action: shell
    resource: "git show *"
    effect: allow
  - action: shell
    resource: "git branch *"
    effect: allow
  - action: shell
    resource: "git blame *"
    effect: allow
  - action: shell
    resource: "git grep *"
    effect: allow
  - action: shell
    resource: "git reflog"
    effect: allow
  - action: shell
    resource: "git stash list"
    effect: allow
  - action: shell
    resource: "git config*"
    effect: deny
  - action: shell
    resource: "git remote -v"
    effect: allow
  - action: shell
    resource: "git tag"
    effect: allow
  - action: shell
    resource: "echo *"
    effect: allow
  - action: shell
    resource: "printf *"
    effect: allow
  - action: shell
    resource: "wc *"
    effect: allow
  - action: shell
    resource: "sort *"
    effect: allow
  - action: shell
    resource: "uniq *"
    effect: allow
  - action: shell
    resource: "cut *"
    effect: allow
  - action: shell
    resource: "ps *"
    effect: allow
  - action: shell
    resource: "top *"
    effect: allow
  - action: shell
    resource: "df *"
    effect: allow
  - action: shell
    resource: "du *"
    effect: allow
  - action: shell
    resource: "free *"
    effect: allow
  - action: shell
    resource: "date"
    effect: allow
  - action: shell
    resource: "cal"
    effect: allow
  - action: shell
    resource: "uptime"
    effect: allow
  - action: shell
    resource: "*.env*"
    effect: deny
---

You are in "Ask To Edit" mode. Before performing any file edits or executing non-readonly bash commands, you must request permission from the user.

Readonly operations (viewing files, checking git status, etc.) can be performed freely. Any operation that modifies files, creates/deletes content, or changes system state requires explicit approval.

When you need to perform a write operation, clearly explain what you intend to do and wait for the user's confirmation before proceeding.
```

注意事项：

- `permissions` 权限规则采用 **last match wins** 策略，所以 `action: shell / resource: "*"` 的 `ask` 规则放在最前面，具体的 `allow` 规则放在中间，兜底的 `deny`/`ask` 规则放在最后
- `*.env*` 的 `deny` 规则：阻止通过 shell 读取 `.env` 文件（如 `cat .env`、`grep key .env`），补上 opencode 默认 `.env` 读取保护对 shell 命令的绕过
- agent 名由文件名决定（`ask-to-edit.md` → `ask-to-edit`），frontmatter 中不需要 `name` 字段
- 已设置为默认 agent（`default_agent`），启动 opencode2 时自动使用

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
permissions:
  - action: edit
    resource: "*"
    effect: allow
  - action: shell
    resource: "*"
    effect: ask
  - action: shell
    resource: "ls *"
    effect: allow
  - action: shell
    resource: "pwd"
    effect: allow
  - action: shell
    resource: "cat *"
    effect: allow
  - action: shell
    resource: "head *"
    effect: allow
  - action: shell
    resource: "tail *"
    effect: allow
  - action: shell
    resource: "less *"
    effect: allow
  - action: shell
    resource: "more *"
    effect: allow
  - action: shell
    resource: "grep *"
    effect: allow
  - action: shell
    resource: "find *"
    effect: allow
  - action: shell
    resource: "find /"
    effect: ask
  - action: shell
    resource: "find / *"
    effect: ask
  - action: shell
    resource: "which *"
    effect: allow
  - action: shell
    resource: "whereis *"
    effect: allow
  - action: shell
    resource: "sed -n *"
    effect: allow
  - action: shell
    resource: "awk *"
    effect: allow
  - action: shell
    resource: "rg *"
    effect: allow
  - action: shell
    resource: "nl *"
    effect: allow
  - action: shell
    resource: "tac *"
    effect: allow
  - action: shell
    resource: "rev *"
    effect: allow
  - action: shell
    resource: "tr *"
    effect: allow
  - action: shell
    resource: "paste *"
    effect: allow
  - action: shell
    resource: "column *"
    effect: allow
  - action: shell
    resource: "join *"
    effect: allow
  - action: shell
    resource: "fold *"
    effect: allow
  - action: shell
    resource: "zcat *"
    effect: allow
  - action: shell
    resource: "zgrep *"
    effect: allow
  - action: shell
    resource: "zless *"
    effect: allow
  - action: shell
    resource: "bzcat *"
    effect: allow
  - action: shell
    resource: "xzcat *"
    effect: allow
  - action: shell
    resource: "diff *"
    effect: allow
  - action: shell
    resource: "cmp *"
    effect: allow
  - action: shell
    resource: "stat *"
    effect: allow
  - action: shell
    resource: "file *"
    effect: allow
  - action: shell
    resource: "strings *"
    effect: allow
  - action: shell
    resource: "xxd *"
    effect: allow
  - action: shell
    resource: "od *"
    effect: allow
  - action: shell
    resource: "hexdump *"
    effect: allow
  - action: shell
    resource: "md5sum *"
    effect: allow
  - action: shell
    resource: "sha256sum *"
    effect: allow
  - action: shell
    resource: "cksum *"
    effect: allow
  - action: shell
    resource: "readelf *"
    effect: allow
  - action: shell
    resource: "objdump *"
    effect: allow
  - action: shell
    resource: "nm *"
    effect: allow
  - action: shell
    resource: "size *"
    effect: allow
  - action: shell
    resource: "ldd *"
    effect: allow
  - action: shell
    resource: "addr2line *"
    effect: allow
  - action: shell
    resource: "basename *"
    effect: allow
  - action: shell
    resource: "dirname *"
    effect: allow
  - action: shell
    resource: "realpath *"
    effect: allow
  - action: shell
    resource: "readlink *"
    effect: allow
  - action: shell
    resource: "tar -tf *"
    effect: allow
  - action: shell
    resource: "unzip -l *"
    effect: allow
  - action: shell
    resource: "zipinfo *"
    effect: allow
  - action: shell
    resource: "git status*"
    effect: allow
  - action: shell
    resource: "git log *"
    effect: allow
  - action: shell
    resource: "git diff *"
    effect: allow
  - action: shell
    resource: "git show *"
    effect: allow
  - action: shell
    resource: "git branch *"
    effect: allow
  - action: shell
    resource: "git blame *"
    effect: allow
  - action: shell
    resource: "git grep *"
    effect: allow
  - action: shell
    resource: "git reflog"
    effect: allow
  - action: shell
    resource: "git stash list"
    effect: allow
  - action: shell
    resource: "git config*"
    effect: deny
  - action: shell
    resource: "git remote -v"
    effect: allow
  - action: shell
    resource: "git tag"
    effect: allow
  - action: shell
    resource: "echo *"
    effect: allow
  - action: shell
    resource: "printf *"
    effect: allow
  - action: shell
    resource: "wc *"
    effect: allow
  - action: shell
    resource: "sort *"
    effect: allow
  - action: shell
    resource: "uniq *"
    effect: allow
  - action: shell
    resource: "cut *"
    effect: allow
  - action: shell
    resource: "ps *"
    effect: allow
  - action: shell
    resource: "top *"
    effect: allow
  - action: shell
    resource: "df *"
    effect: allow
  - action: shell
    resource: "du *"
    effect: allow
  - action: shell
    resource: "free *"
    effect: allow
  - action: shell
    resource: "date"
    effect: allow
  - action: shell
    resource: "cal"
    effect: allow
  - action: shell
    resource: "uptime"
    effect: allow
  - action: shell
    resource: "*.env*"
    effect: deny
---

You are in "Auto Edit" mode. File edits are performed automatically without asking for permission.

Readonly bash operations (viewing files, checking git status, etc.) can be performed freely. Any bash command that modifies files, creates/deletes content, or changes system state requires explicit approval from the user.

When you need to perform a non-readonly bash operation, clearly explain what you intend to do and wait for the user's confirmation before proceeding.
```

与 Ask To Edit 模式的区别：

| 模式 | 文件编辑 | Shell 命令 |
|------|----------|-----------|
| Ask To Edit | 需要确认 | 只读命令自动，其他需要确认 |
| Auto Edit | 自动执行 | 只读命令自动，其他需要确认 |

切换方式：v2 主 TUI 不支持 `--agent` 启动标志（`--agent` 仅用于子命令），在 TUI 内通过 agent 切换菜单选择即可；非交互运行用 `opencode2 run --agent auto-edit "..."`，最小化界面用 `opencode2 mini --agent auto-edit`

### 清理配置与数据

不卸载 opencode 本身，仅清除其配置、数据和缓存，恢复到全新状态。清理前先退出所有正在运行的 opencode 进程。

```bash
# 1. 全局配置（opencode.jsonc、cli.json、agents/、commands/、skills/、插件依赖 node_modules/ 等）
rm -rf ~/.config/opencode

# 2. 用户数据（登录凭证 auth.json、会话数据库 opencode.db、日志 log/、仓库缓存 repos/）
rm -rf ~/.local/share/opencode

# 3. 缓存（升级用的二进制缓存 bin/）
rm -rf ~/.cache/opencode
```

注意事项：

- 清理后所有登录凭证（API key 等）丢失，需重新执行 `opencode2 auth login`
- 会话历史、消息记录存储在 `~/.local/share/opencode/opencode.db`（SQLite），删除后不可恢复
- `~/.local/share/opencode/log/` 存放服务运行日志（如 `opencode.log`），v1/v2 路径相同
- `~/.config/opencode` 下的 `node_modules/` 是 opencode 自动安装的插件依赖，无需手动保留
- `~/.agents/skills`、`~/.claude/skills` 属于外部技能目录，并非 opencode 独有，仅在确定不需要时一并清理
- v1（`opencode`）与 v2（`opencode2`）复用同一配置目录，上述清理会对两者同时生效

### v1（opencode）配置

> 本节为 v2 升级前（opencode v1）的配置存档，直接从提交历史中拷贝。当前机器已升级到 v2，v1 配置仅作参考，若要使用 v1 请先安装 v1 版本的 opencode。

#### 全局配置

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
      "git config*": "deny",
      "git commit*": "ask"
    }
  }
}
```

#### Ask To Edit 自定义 Agent

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
    "find /": ask
    "find / *": ask
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
    "git config*": deny
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

#### Auto Edit 自定义 Agent

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
    "find /": ask
    "find / *": ask
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
    "git config*": deny
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

#### 清理配置与数据

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

## deepseek-harness

```bash
# remote
npm install -g @deepseek-ai/dsh --proxy http://proxy-server:port
dsh web --port 10086

# host
ssh -N -L 10086:localhost:10086 ubuntu
```


## pi 编辑模式与只读 Bash 守卫

pi（`@earendil-works/pi-coding-agent`）本身没有内置的权限 / 编辑确认模式，这类能力通过扩展（extension）实现。这里实现两种编辑模式，并对 `bash` 工具加一层只读守卫。

### 行为概览

| 模式 | edit / write | bash 只读命令 | bash 非只读命令 |
|------|--------------|---------------|-----------------|
| `ask-to-edit`（默认） | 弹 diff 确认 | 直接执行 | 弹窗确认 |
| `auto-edit` | 自动执行 | 直接执行 | 弹窗确认 |

两种模式下 bash 行为一致：只读命令自动放行，非只读命令一律弹窗确认；无 UI 时（`-p` / `--mode json`）非只读命令直接阻止并返回原因。

### 扩展文件

放到全局扩展目录即可被自动发现，无需修改 `settings.json`：

```bash
~/.pi/agent/extensions/edit-modes.ts
```

### 完整代码

```ts
/**
 * Edit Modes Extension
 *
 * Approval modes for the built-in `edit` / `write` tools, plus a read-only
 * gate for the built-in `bash` tool.
 *
 * Edit modes:
 *   ask-to-edit (default)  Show a diff and wait for approval before every edit/write.
 *   auto-edit              Run edit/write without prompting.
 *
 * Bash (same in BOTH edit modes):
 *   - read-only commands run without asking;
 *   - any other command (writes, redirections, unknown commands, wrappers, ...)
 *     shows a confirmation dialog.
 *
 * Switch modes:
 *   /edit-mode [ask-to-edit|auto-edit]   set explicitly (no arg opens a picker)
 *   /ask-to-edit                         switch to ask-to-edit
 *   /auto-edit                           switch to auto-edit
 *   Alt+E                                cycle between the two modes
 *
 * The active mode is shown in the footer and persisted per session, so it is
 * restored on /reload, /resume and tree navigation. The startup mode can be set
 * with `--edit-mode <mode>` (default: ask-to-edit).
 *
 * Scope: this gates the built-in `edit` and `write` tools, and the `bash` tool
 * for non-read-only commands. The bash check is a heuristic fail-closed
 * classifier, not a security sandbox. In non-interactive sessions there is no
 * UI to approve with, so blocked actions are refused instead of silently allowed.
 */

import {
	isToolCallEventType,
	type EditToolCallEvent,
	type ExtensionAPI,
	type ExtensionContext,
	type WriteToolCallEvent,
} from "@earendil-works/pi-coding-agent";

type EditMode = "ask-to-edit" | "auto-edit";

const MODES: readonly EditMode[] = ["ask-to-edit", "auto-edit"];
const STATE_TYPE = "edit-mode";
const STARTUP_FLAG = "edit-mode";
const STATUS_KEY = "edit-mode";

const MAX_PREVIEW_LINES = 40;
const MAX_PREVIEW_WIDTH = 120;
const MAX_BASH_PREVIEW_LINES = 20;

function isEditMode(value: unknown): value is EditMode {
	return value === "ask-to-edit" || value === "auto-edit";
}

function truncateForPreview(text: string, maxLines: number, maxWidth: number): string {
	const lines = text.split("\n");
	const visible = lines
		.slice(0, maxLines)
		.map((line) => (line.length > maxWidth ? `${line.slice(0, maxWidth - 1)}…` : line));
	if (lines.length > maxLines) {
		visible.push(`… (${lines.length - maxLines} more lines)`);
	}
	return visible.join("\n");
}

// ---------------------------------------------------------------------------
// Bash read-only classifier
// ---------------------------------------------------------------------------

type ShellToken = { type: "word"; value: string } | { type: "operator"; value: string };

interface TokenizedShell {
	tokens: ShellToken[];
	hasWriteRedirection: boolean;
	hasSubstitution: boolean;
}

const SAFE_REDIRECT_TARGETS = ["/dev/null", "/dev/stderr", "/dev/stdout"];

function tokenizeShell(command: string): TokenizedShell {
	const tokens: ShellToken[] = [];
	let current = "";
	let hasWriteRedirection = false;
	let hasSubstitution = false;
	let quote: "'" | '"' | null = null;
	let i = 0;

	const pushWord = () => {
		if (current.length > 0) {
			tokens.push({ type: "word", value: current });
			current = "";
		}
	};

	const isWordBoundary = (ch: string | undefined) => ch === undefined || /[\s;&|<>]/.test(ch);

	while (i < command.length) {
		const ch = command[i] as string;

		if (quote === "'") {
			if (ch === "'") quote = null;
			else current += ch;
			i++;
			continue;
		}

		if (quote === '"') {
			if (ch === "\\") {
				const next = command[i + 1];
				if (next === '"' || next === "\\" || next === "$" || next === "`") {
					current += next;
					i += 2;
					continue;
				}
				current += ch;
				i++;
				continue;
			}
			if (ch === '"') {
				quote = null;
				i++;
				continue;
			}
			if (ch === "`") hasSubstitution = true;
			if (ch === "$" && command[i + 1] === "(") hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}

		if (ch === "\\") {
			const next = command[i + 1];
			if (next !== undefined) {
				current += next;
				i += 2;
				continue;
			}
			i++;
			continue;
		}
		if (ch === "'") {
			quote = "'";
			i++;
			continue;
		}
		if (ch === '"') {
			quote = '"';
			i++;
			continue;
		}
		if (ch === "`") {
			hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}
		if (ch === "$" && command[i + 1] === "(") {
			hasSubstitution = true;
			current += ch;
			i++;
			continue;
		}

		if (ch === ">") {
			let j = i + 1;
			if (command[j] === ">") j++;
			while (command[j] === " " || command[j] === "\t") j++;
			if (command[j] === "&") {
				// File-descriptor duplication (2>&1), not a file write.
				j++;
				while (!isWordBoundary(command[j])) j++;
			} else {
				const start = j;
				while (!isWordBoundary(command[j])) j++;
				const target = command.slice(start, j).replace(/^['"]|['"]$/g, "");
				const safe =
					SAFE_REDIRECT_TARGETS.includes(target) || target.startsWith("/dev/fd/");
				if (!safe) hasWriteRedirection = true;
			}
			pushWord();
			i = j;
			continue;
		}
		if (ch === "<") {
			pushWord();
			i++;
			if (command[i] === "<") i++;
			if (command[i] === "<") i++;
			continue;
		}
		if (ch === "|") {
			pushWord();
			if (command[i + 1] === "|") {
				tokens.push({ type: "operator", value: "||" });
				i += 2;
			} else {
				tokens.push({ type: "operator", value: "|" });
				i++;
			}
			continue;
		}
		if (ch === ";" || ch === "\n") {
			pushWord();
			tokens.push({ type: "operator", value: ";" });
			i++;
			continue;
		}
		if (ch === "&") {
			if (command[i + 1] === "&") {
				pushWord();
				tokens.push({ type: "operator", value: "&&" });
				i += 2;
				continue;
			}
			if (command[i + 1] === ">") {
				hasWriteRedirection = true;
				pushWord();
				i += 2;
				if (command[i] === ">") i++;
				continue;
			}
			pushWord();
			tokens.push({ type: "operator", value: "&" });
			i++;
			continue;
		}
		if (ch === " " || ch === "\t") {
			pushWord();
			i++;
			continue;
		}

		current += ch;
		i++;
	}

	pushWord();
	if (command.includes("<(") || command.includes(">(")) hasSubstitution = true;
	return { tokens, hasWriteRedirection, hasSubstitution };
}

/** Commands that only read data; any argument is safe (subject to flag checks). */
const READ_ONLY_SIMPLE = new Set([
	"ls", "cat", "tac", "bat", "head", "tail", "less", "more", "nl", "wc", "uniq", "cut", "tr",
	"column", "rev", "fold", "paste", "join", "comm", "diff", "cmp", "md5sum", "sha1sum",
	"sha256sum", "shasum", "cksum", "basename", "dirname", "realpath", "readlink", "stat",
	"file", "du", "df", "tree", "grep", "egrep", "fgrep", "rg", "ag", "ack", "awk", "gawk",
	"mawk", "jq", "yq", "xmllint", "pwd", "echo", "printf", "true", "false", ":", "test", "[",
	"which", "type", "whereis", "whoami", "id", "groups", "uname", "hostname", "date", "uptime",
	"locale", "tty", "ps", "pgrep", "free", "vmstat", "iostat", "nproc", "arch", "who", "w",
	"last", "cal", "bc", "man", "help", "history", "dirs", "jobs", "umask", "sleep", "cd",
	"pushd", "popd", "export", "set", "unset", "alias", "unalias",
]);

const GIT_READ_ONLY_SUBCOMMANDS = new Set([
	"status", "log", "diff", "show", "describe", "rev-parse", "rev-list", "ls-files", "ls-tree",
	"cat-file", "blame", "shortlog", "show-ref", "for-each-ref", "name-rev", "merge-base",
	"whatchanged", "grep", "fsck", "count-objects", "check-ignore", "check-attr", "version",
	"help",
]);

function gitReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	const rest = args.slice(1);

	if (sub.startsWith("-")) {
		if (sub === "--version" || sub === "-v" || sub === "--help" || sub === "-h") return true;
		if (["-C", "--git-dir", "--work-tree", "-c", "--config-env"].includes(sub)) {
			return rest.length >= 2 && gitReadOnly(rest.slice(1));
		}
		return false;
	}
	if (GIT_READ_ONLY_SUBCOMMANDS.has(sub)) return true;

	if (sub === "branch" || sub === "tag") return rest.every((a) => a.startsWith("-"));
	if (sub === "remote") {
		return rest.length === 0 || rest.every((a) => a.startsWith("-")) || ["show", "get-url"].includes(rest[0] as string);
	}
	if (sub === "config") {
		if (rest.some((a) => a === "--list" || a === "-l" || a.startsWith("--get"))) return true;
		return rest.filter((a) => !a.startsWith("-")).length <= 1;
	}
	if (sub === "stash") return rest[0] === "list" || rest[0] === "show";
	if (sub === "worktree") return rest.length === 0 || rest[0] === "list";
	if (sub === "reflog") return rest.length === 0 || rest[0] === "show";
	if (sub === "submodule") return rest.length === 0 || rest[0] === "status" || rest[0] === "summary";
	if (sub === "notes") return rest.length === 0 || rest[0] === "list" || rest[0] === "show";
	if (sub === "symbolic-ref") return rest.filter((a) => !a.startsWith("-")).length <= 1;

	return false;
}

const GH_READ_ONLY_VERBS = new Set(["view", "list", "status", "diff", "checks", "ls", "get"]);

function ghReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	if (["--version", "-v", "--help", "-h"].includes(args[0] as string)) return true;
	if (args[0] === "auth" && (args[1] === "status" || args[1] === "token")) return true;
	if (args[0] === "config" && (args[1] === "get" || args[1] === "list")) return true;
	return GH_READ_ONLY_VERBS.has(args[1] as string);
}

const PKG_READ_ONLY_SUBCOMMANDS = new Set([
	"ls", "list", "view", "show", "info", "outdated", "why", "root", "prefix", "bin", "ping",
	"doctor", "fund", "help",
]);

function packageManagerReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (["--version", "-v", "--help", "-h", "version"].includes(sub)) return true;
	if (sub === "config") return args.length <= 1 || ["get", "list", "ls"].includes(args[1] as string);
	return PKG_READ_ONLY_SUBCOMMANDS.has(sub);
}

function bunReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (["--version", "-v", "--help", "-h"].includes(sub)) return true;
	if (sub === "pm" && (args[1] === "ls" || args[1] === "list")) return true;
	return PKG_READ_ONLY_SUBCOMMANDS.has(sub);
}

const CONTAINER_READ_ONLY_SUBCOMMANDS = new Set([
	"ps", "images", "logs", "inspect", "version", "info", "stats", "top", "port", "diff",
	"history", "search", "events",
]);
const CONTAINER_READ_ONLY_CHILD = new Set(["ls", "ps", "logs", "inspect", "top", "stats", "port", "diff", "config", "df", "info", "events"]);

function containerReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (sub.startsWith("-")) return true; // docker --version / --help
	if (CONTAINER_READ_ONLY_SUBCOMMANDS.has(sub)) return true;
	if (["image", "container", "volume", "network", "compose", "system"].includes(sub)) {
		return CONTAINER_READ_ONLY_CHILD.has(args[1] as string);
	}
	return false;
}

function kubectlReadOnly(args: string[]): boolean {
	if (args.length === 0) return true;
	const sub = args[0] as string;
	if (sub.startsWith("-")) return true;
	if (["get", "describe", "logs", "explain", "api-resources", "api-versions", "cluster-info", "version", "top", "events"].includes(sub)) return true;
	if (sub === "auth") return args[1] === "can-i";
	if (sub === "config") {
		return ["view", "get-contexts", "current-context", "get-clusters", "get-users", "get-context"].includes(args[1] as string);
	}
	return false;
}

function runtimeReadOnly(cmd: string, args: string[]): boolean {
	if (args.length === 0) return false;
	const versionFlags = new Set(["--version", "-V", "-v", "--help", "-h", "-version", "version"]);
	if (cmd === "go") return ["version", "env", "list", "doc"].includes(args[0] as string);
	if (cmd === "cargo") return ["tree", "metadata", "search", "--version", "-V", "--list"].includes(args[0] as string);
	if (cmd === "pip" || cmd === "pip3") return ["list", "show", "freeze", "check", "index"].includes(args[0] as string);
	if (cmd === "poetry") return ["show", "check", "env", "--version", "-V"].includes(args[0] as string);
	if (cmd === "gem") return ["list", "search", "info", "environment", "--version", "-v"].includes(args[0] as string);
	if (cmd === "deno") return ["info", "--version", "-V"].includes(args[0] as string);
	return args.every((a) => versionFlags.has(a));
}

const RUNTIME_COMMANDS = new Set([
	"node", "python", "python3", "pip", "pip3", "poetry", "cargo", "rustc", "go", "java", "javac",
	"dotnet", "ruby", "gem", "php", "composer", "mvn", "gradle", "make", "cmake", "deno",
]);

function baseName(p: string): string {
	const idx = Math.max(p.lastIndexOf("/"), p.lastIndexOf("\\"));
	return idx >= 0 ? p.slice(idx + 1) : p;
}

function analyzeWords(words: string[], depth = 0): boolean {
	if (depth > 4) return false;
	let i = 0;
	while (i < words.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(words[i] as string)) i++;
	if (i >= words.length) return true; // only variable assignments

	const cmd = baseName(words[i] as string);
	const args = words.slice(i + 1);
	if (cmd.length === 0 || cmd.includes("$")) return false;

	if (cmd === "git") return gitReadOnly(args);
	if (cmd === "gh") return ghReadOnly(args);
	if (cmd === "npm" || cmd === "pnpm" || cmd === "yarn") return packageManagerReadOnly(args);
	if (cmd === "bun") return bunReadOnly(args);
	if (cmd === "docker" || cmd === "podman") return containerReadOnly(args);
	if (cmd === "kubectl") return kubectlReadOnly(args);
	if (RUNTIME_COMMANDS.has(cmd)) return runtimeReadOnly(cmd, args);

	if (cmd === "env" || cmd === "time" || cmd === "nohup") {
		let j = 0;
		while (j < args.length && /^[A-Za-z_][A-Za-z0-9_]*=/.test(args[j] as string)) j++;
		if (j >= args.length) return true;
		return analyzeWords(args.slice(j), depth + 1);
	}
	if (cmd === "timeout") return args.length >= 2 && analyzeWords(args.slice(2), depth + 1);

	if (cmd === "find" || cmd === "fd" || cmd === "fdfind") {
		return !args.some((a) => a.startsWith("-delete") || a.startsWith("-exec") || a.startsWith("-ok") || a.startsWith("-fprint") || a.startsWith("-fls"));
	}
	if (cmd === "sed") {
		return !args.some((a) => a.startsWith("-i") || a.startsWith("--in-place"));
	}
	if (cmd === "sort") {
		return !args.some((a) => a.startsWith("-o") || a.startsWith("--output"));
	}
	return READ_ONLY_SIMPLE.has(cmd);
}

function analyzeBashCommand(command: string): { readOnly: boolean; reason: string } {
	const trimmed = command.trim();
	if (trimmed.length === 0) return { readOnly: true, reason: "empty command" };

	const { tokens, hasWriteRedirection, hasSubstitution } = tokenizeShell(command);
	if (hasSubstitution) return { readOnly: false, reason: "contains command or process substitution" };
	if (hasWriteRedirection) return { readOnly: false, reason: "contains output redirection to a file" };

	const segments: string[][] = [];
	let current: string[] = [];
	for (const token of tokens) {
		if (token.type === "operator") {
			if (current.length > 0) segments.push(current);
			current = [];
		} else {
			current.push(token.value);
		}
	}
	if (current.length > 0) segments.push(current);
	if (segments.length === 0) return { readOnly: true, reason: "empty command" };

	for (const segment of segments) {
		if (!analyzeWords(segment)) {
			return { readOnly: false, reason: `not read-only: ${segment.join(" ")}` };
		}
	}
	return { readOnly: true, reason: "read-only" };
}

// ---------------------------------------------------------------------------
// Edit/write previews
// ---------------------------------------------------------------------------

function buildEditPreview(input: EditToolCallEvent["input"]): string {
	const parts: string[] = [];
	input.edits.forEach((edit, index) => {
		if (input.edits.length > 1) {
			parts.push(`@@ change ${index + 1} of ${input.edits.length} @@`);
		}
		for (const line of edit.oldText.split("\n")) parts.push(`- ${line}`);
		for (const line of edit.newText.split("\n")) parts.push(`+ ${line}`);
	});
	return truncateForPreview(parts.join("\n"), MAX_PREVIEW_LINES, MAX_PREVIEW_WIDTH);
}

function buildWritePreview(input: WriteToolCallEvent["input"]): string {
	const parts = input.content.split("\n").map((line) => `+ ${line}`);
	return truncateForPreview(parts.join("\n"), MAX_PREVIEW_LINES, MAX_PREVIEW_WIDTH);
}

// ---------------------------------------------------------------------------
// Extension
// ---------------------------------------------------------------------------

export default function (pi: ExtensionAPI) {
	let mode: EditMode = "ask-to-edit";
	let bashAutoApprove = false;

	pi.registerFlag(STARTUP_FLAG, {
		description: "Start in an edit approval mode: ask-to-edit | auto-edit",
		type: "string",
		default: "ask-to-edit",
	});

	function applyStatus(ctx: ExtensionContext): void {
		if (!ctx.hasUI) return;
		const color = mode === "auto-edit" ? "success" : "warning";
		const suffix = bashAutoApprove ? " · bash:auto" : "";
		ctx.ui.setStatus(STATUS_KEY, ctx.ui.theme.fg(color, `✎ ${mode}${suffix}`));
	}

	function setMode(next: EditMode, ctx: ExtensionContext, options: { persist?: boolean; notify?: boolean } = {}): void {
		const { persist = true, notify = true } = options;
		mode = next;
		if (persist) pi.appendEntry(STATE_TYPE, { mode });
		applyStatus(ctx);
		if (notify && ctx.hasUI) ctx.ui.notify(`Edit mode: ${mode}`, "info");
	}

	pi.on("session_start", async (_event, ctx) => {
		// "Allow all bash commands (this session)" is intentionally not persisted.
		bashAutoApprove = false;

		let restored: EditMode | undefined;
		for (const entry of ctx.sessionManager.getBranch()) {
			if (entry.type === "custom" && entry.customType === STATE_TYPE) {
				const candidate = (entry.data as { mode?: unknown } | undefined)?.mode;
				if (isEditMode(candidate)) restored = candidate;
			}
		}

		const flagValue = pi.getFlag(STARTUP_FLAG);
		mode = restored ?? (isEditMode(flagValue) ? flagValue : "ask-to-edit");
		applyStatus(ctx);
	});

	pi.registerCommand("edit-mode", {
		description: "Set edit approval mode (ask-to-edit | auto-edit)",
		getArgumentCompletions: (prefix: string) =>
			MODES.filter((m) => m.startsWith(prefix)).map((m) => ({ value: m, label: m })),
		handler: async (args, ctx) => {
			const arg = args.trim();
			if (!arg) {
				if (!ctx.hasUI) {
					ctx.ui.notify(`Edit mode: ${mode}`, "info");
					return;
				}
				const choice = await ctx.ui.select(`Edit mode (current: ${mode})`, [...MODES]);
				if (choice && isEditMode(choice)) setMode(choice, ctx);
				return;
			}
			if (!isEditMode(arg)) {
				ctx.ui.notify(`Unknown edit mode: ${arg}. Use ask-to-edit or auto-edit.`, "error");
				return;
			}
			setMode(arg, ctx);
		},
	});

	pi.registerCommand("ask-to-edit", {
		description: "Ask for approval before every edit/write",
		handler: async (_args, ctx) => setMode("ask-to-edit", ctx),
	});

	pi.registerCommand("auto-edit", {
		description: "Auto-approve every edit/write",
		handler: async (_args, ctx) => setMode("auto-edit", ctx),
	});

	pi.registerShortcut("alt+e", {
		description: "Cycle edit mode (ask-to-edit ↔ auto-edit)",
		handler: async (ctx) => setMode(mode === "auto-edit" ? "ask-to-edit" : "auto-edit", ctx),
	});

	pi.on("tool_call", async (event, ctx) => {
		// --- bash: read-only allowed, everything else asks (in both edit modes) ---
		if (isToolCallEventType("bash", event)) {
			if (bashAutoApprove) return;
			const verdict = analyzeBashCommand(event.input.command);
			if (verdict.readOnly) return;

			const preview = truncateForPreview(event.input.command, MAX_BASH_PREVIEW_LINES, MAX_PREVIEW_WIDTH);

			if (!ctx.hasUI) {
				return {
					block: true,
					reason:
						`Blocked by read-only bash guard: ${verdict.reason}, and no interactive UI is available ` +
						`to approve it. Use a read-only command or run an interactive session.`,
				};
			}

			const choice = await ctx.ui.select(
				`⚠ Non-read-only bash (${verdict.reason}):\n\n$ ${preview}\n`,
				["Allow once", "Allow all commands (this session)", "Deny"],
			);

			if (choice === "Allow once") return;
			if (choice === "Allow all commands (this session)") {
				bashAutoApprove = true;
				applyStatus(ctx);
				ctx.ui.notify("Bash: auto-approving all commands for this session", "warning");
				return;
			}
			return { block: true, reason: `User denied non-read-only bash command: ${verdict.reason}` };
		}

		// --- edit/write: gated only in ask-to-edit mode ---
		if (mode === "auto-edit") return;

		const isEditCall = isToolCallEventType("edit", event);
		const isWriteCall = isToolCallEventType("write", event);
		if (!isEditCall && !isWriteCall) return;

		const path = event.input.path;
		const summary = isEditCall
			? `edit ${path} (${event.input.edits.length} change${event.input.edits.length === 1 ? "" : "s"})`
			: `write ${path}`;

		if (!ctx.hasUI) {
			return {
				block: true,
				reason:
					`Blocked by ask-to-edit mode: ${summary} needs approval, but no interactive UI is available. ` +
					`Switch to auto-edit (--edit-mode auto-edit) to allow edits in non-interactive sessions.`,
			};
		}

		const preview = isEditCall ? buildEditPreview(event.input) : buildWritePreview(event.input);
		const title = `✎ ask-to-edit: ${summary}\n\n${preview}\n`;

		const choice = await ctx.ui.select(title, [
			"Allow once",
			"Allow all edits (switch to auto-edit)",
			"Deny",
		]);

		if (choice === "Allow once") return;
		if (choice === "Allow all edits (switch to auto-edit)") {
			setMode("auto-edit", ctx);
			return;
		}
		if (choice === "Deny") {
			return { block: true, reason: `User denied: ${summary} (ask-to-edit mode)` };
		}

		// Cancelled the dialog (Esc / Ctrl+C) counts as a denial.
		return { block: true, reason: `User cancelled approval for: ${summary} (ask-to-edit mode)` };
	});
}
```

### 使用方式

```text
/edit-mode                  # 打开选择器
/edit-mode auto-edit        # 直接设置（Tab 可补全）
/auto-edit                  # 切到自动编辑
/ask-to-edit                # 切到确认编辑
Alt+E                       # 两种模式循环
pi --edit-mode auto-edit    # 启动时指定默认模式
```

- 当前模式显示在 footer 状态栏。
- 模式按 session 持久化（`pi.appendEntry`），`/reload`、`/resume`、`/tree` 后自动恢复；新 session 回到 `--edit-mode` 指定的默认值（未指定则 `ask-to-edit`）。
- `Allow all commands (this session)` 只对当前 session 生效，不写入 session、不跨会话，footer 会显示 `✎ ask-to-edit · bash:auto`。

### edit / write 确认弹窗

`ask-to-edit` 模式下每次 `edit` / `write` 都会弹出 diff 预览，选项：

- `Allow once`：只允许这一次
- `Allow all edits (switch to auto-edit)`：允许并切到 `auto-edit`
- `Deny`：拒绝；按 Esc / Ctrl+C 取消弹窗同样视为拒绝

`auto-edit` 模式下 `edit` / `write` 直接执行。

### bash 只读判定规则（fail-closed）

判定逻辑：先把命令按 `;`、`&&`、`||`、`|`、`&` 拆成多段，**每一段**都必须命中只读白名单，否则视为非只读。启发式、fail-closed，不是安全沙箱。

自动放行的判断：

1. 段内命令（取 basename）在白名单中，且带有的参数不触发写操作。
2. 输出重定向只允许到 `/dev/null`、`/dev/stderr`、`/dev/stdout`、`/dev/fd/*`；`2>&1` 这类 fd 重定向不算写文件。
3. 管道 / 列表两侧都必须是只读命令。

直接判为非只读的情况：

- 未识别的命令、`$VAR` 作为命令名
- 输出重定向到普通文件（`>` / `>>` / `&>`）
- 命令替换 `$(...)`、反引号，进程替换 `<(...)` / `>(...)`
- 写入类命令：`rm`、`mv`、`cp`、`mkdir`、`touch`、`chmod`、`chown`、`tee`、`dd`、`ln`、`truncate` 等
- `find` 携带 `-delete` / `-exec` / `-ok` / `-fprint` / `-fls`
- `sed -i` / `--in-place`、`sort -o` / `--output`
- 包装器 `env` / `time` / `nohup` / `timeout` 会递归检查内层命令；`sudo`、`source` / `.`、`eval`、`xargs`、`command` 一律视为非只读

常见只读命令（部分）：

| 类别 | 命令 |
|------|------|
| 文件 / 文本 | `ls` `cat` `head` `tail` `wc` `sort`（无 `-o`）`uniq` `cut` `tr` `grep` `rg` `diff` `find`（无写参数）`grep` `jq` |
| 系统信息 | `pwd` `echo` `which` `whoami` `uname` `date` `ps` `free` `df` `du` |
| git | `status` `log` `diff` `show` `branch`（仅列表）`tag`（仅列表）`rev-parse` `config --get/--list` `stash list` 等 |
| 包管理器 | `npm` / `pnpm` / `yarn` 的 `ls` `list` `view` `show` `outdated` `why` `config get` |
| 容器 | `docker ps/images/logs/inspect/compose logs` 等、`kubectl get/describe/logs/config view` |
| 运行时版本 | `node --version`、`python3 -V`、`go version`、`cargo --version` 等 |

非只读命令弹窗选项：

- `Allow once`：只允许这一次
- `Allow all commands (this session)`：本次 session 内所有 bash 命令不再询问（不持久化）
- `Deny`：拒绝

### 验证

扩展可被自动发现且无加载错误：

```bash
# 通过 pi 的扩展发现接口确认
extensions: ['/home/test/.pi/agent/extensions/edit-modes.ts']
errors: []
```

行为测试覆盖：26 条只读命令全部放行、32 条非只读命令全部弹窗/拒绝、`auto-edit` 下仍拦截非只读 bash、session 级放行与重置、无 UI 时只读放行 / 非只读阻止、edit 模式恢复。

### 注意事项

- 这是启发式守卫，不是沙箱；扩展以当前用户权限运行，shell 语法仍可能绕过（例如允许的只读命令自身带副作用）。
- 只拦 `bash` 工具，不拦用户手动执行的 `!` / `!!` 命令。
- 新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效。
