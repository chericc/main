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


## pi

pi（`@earendil-works/pi-coding-agent`）通过扩展（extension）扩展能力。扩展放到全局目录 `~/.pi/agent/extensions/` 即可被自动发现，无需修改 `settings.json`；新增 / 修改扩展后，运行中的 pi 需要 `/reload` 才会生效。

| 扩展 | 说明 | 文档 |
|------|------|------|
| `edit-modes.ts` | `ask-to-edit` / `auto-edit` / `auto-all` 三种编辑模式；`bash` 只读守卫与危险命令审批；`git commit` 在任何模式下都必须逐次确认（不可自动批准，弹窗只有 `Allow once` / `Deny`）；edit diff 与 bash 命令共用同一个审批框：默认折叠前 6 行预览，点击预览或按 `v` / `ctrl+o` 弹出全屏可滚动的完整内容（点击 / `Esc` 关闭） | [pi/edit-modes.md](pi/edit-modes.md) |
| `compact-tools.ts` | 让内置工具（`bash` / `read` / `grep` / `find` / `ls` / `write`）结果默认只显示 1 行，点击工具行或 `ctrl+o` 展开 / 收拢 | [pi/compact-tools.md](pi/compact-tools.md) |
| `token-speed.ts` | 在 footer 常驻显示生成速度：最近 5 次回复的**中位数 + 平均值**（tok/s），过短回复不计入，消息结束时计算 | [pi/token-speed.md](pi/token-speed.md) |

```bash
~/.pi/agent/extensions/edit-modes.ts
~/.pi/agent/extensions/compact-tools.ts
~/.pi/agent/extensions/token-speed.ts
```

### npm 插件

第三方插件通过 `pi install` 安装，记录在 `~/.pi/agent/settings.json` 的 `packages` 中，统一安装在 `~/.pi/agent/npm/node_modules/` 下；安装 / 修改后运行中的 pi 需要 `/reload` 生效。

```bash
# 查看已安装插件（包名 + 安装路径）
pi list

# 安装 / 卸载 / 更新
pi install npm:<package>
pi remove npm:<package>
pi update --extensions
```

| 插件 | 说明 | 文档 |
|------|------|------|
| `npm:@juicesharp/rpiv-todo` | 给模型提供 `todo` 工具：编辑器上方的实时任务面板、`/todos` 命令；列表从会话回放，可跨 `/reload` 与压缩 | [rpiv-todo](https://github.com/juicesharp/rpiv-mono/tree/main/packages/rpiv-todo) |
| `npm:@ollama/pi-web-search` | 提供 `web_search` / `web_fetch` 工具：走本机 Ollama 的搜索 / 抓取 API（需本地 Ollama 运行） | [pi-web-search](https://github.com/ollama/pi-web-search) |
| `npm:billion-context-pi` | 模型驱动的上下文管理：提供 `compress` / `decompress` / `search_context` / `acp_status` 工具与 `acp_delegate` 子代理，替代 pi 内置 auto-compaction | [billion-context-pi](https://github.com/ranxianglei/billion-context-pi) |

```bash
~/.pi/agent/npm/node_modules/@ollama/pi-web-search
~/.pi/agent/npm/node_modules/@juicesharp/rpiv-todo
~/.pi/agent/npm/node_modules/billion-context-pi
```

### 全局 settings.json

`~/.pi/agent/settings.json` 当前内容：

```json
{
  "defaultModel": "deepseek-v4.1-flash:cloud",
  "defaultProvider": "ollama",
  "doubleEscapeAction": "none",
  "editorPaddingX": 1,
  "fullscreenExitOutput": "transcript",
  "fullscreenScrollbar": "auto",
  "hideThinkingBlock": true,
  "lastChangelogVersion": "0.85.1",
  "modelThinkingLevels": {
    "ollama/deepseek-v4.1-flash:cloud": "high"
  },
  "outputPad": 1,
  "packages": [
    "npm:@ollama/pi-web-search",
    "npm:@juicesharp/rpiv-todo",
    "npm:billion-context-pi"
  ],
  "showCacheMissNotices": true,
  "terminal": {
    "trueColor": true
  },
  "theme": "dark",
  "treeFilterMode": "no-tools",
  "tuiMode": "fullscreen"
}
```

- `tuiMode: "fullscreen"` 是**鼠标交互的前提**：点击工具行展开 / 收拢（compact-tools）、点击预览打开 / 关闭全屏 diff 或完整命令（edit-modes）。regular 模式下 pi 不抓鼠标，只能用键盘（`ctrl+o`、`v`、`Esc` 等）。
- `defaultModel` / `defaultProvider` / `modelThinkingLevels`：默认走本机 ollama 的 `deepseek-v4.1-flash:cloud`（1M 上下文，thinking level `high`）；provider 与模型定义在 `~/.pi/agent/models.json`（`openai-completions` API、`http://127.0.0.1:11434/v1`）。
- `theme` / `hideThinkingBlock` / `fullscreenScrollbar` / `outputPad` / `showCacheMissNotices` / `doubleEscapeAction` / `treeFilterMode` / `fullscreenExitOutput` / `editorPaddingX` 为个人显示与交互偏好，可按需调整。
- `lastChangelogVersion` 由 pi 自动维护，无配置意义。
- `terminal.trueColor`：强制真彩色输出，避免 SSH 下 256 色降级导致配色刺眼，详见下节。

常用配置：

- `~/.pi/agent/config/pi-task-models/config.json`：子任务（task）模型配置，含 `profiles` 与 `tasks` 两组；本机仅定义 `fast` profile（`ollama/deepseek-v4.1-flash:cloud`，`thinkingLevel: "high"`），`tasks` 为空
- `rpiv-todo`：配置文件 `~/.config/rpiv-todo/config.json`（`maxWidgetLines`、`collapseKey` 默认 `ctrl+shift+t`、`guidance`）
- `@ollama/pi-web-search`：提供 `web_search` / `web_fetch` 工具，需本地 Ollama 运行（搜索 / 抓取由 Ollama 的 API 完成）；无额外配置
- `billion-context-pi`：零配置即可用（自动读取模型上下文窗口）；配置文件 `~/.pi/acp.json`（全局）/ `<project>/.pi/acp.json`（项目），当前配置见下节《billion-context-pi 配置》；常用键 `delegate: false`（关掉内置子代理，改用其他 sub-agent）；日志 `~/.pi/acp.log`（`tail -f ~/.pi/acp.log`，10 MB 轮转到 `.old`，环境变量 `ACP_LOG_FILE` 可改路径）；`/acp` 查看上下文分布与压缩块

### billion-context-pi 配置（`~/.pi/acp.json`）

billion-context-pi 的配置独立于 pi 的 `settings.json`，放在 `~/.pi/acp.json`（全局）或 `<project>/.pi/acp.json`（项目，逐字段覆盖全局）；优先级为 环境变量 > 项目文件 > 全局文件 > 内置默认值。未知键、缺失文件、坏 JSON 都会被静默忽略。当前生效的全局配置：

```json
{
  "compress": {
    "nudgeGrowthTokens": 200000
  }
}
```

说明：

- `compress.nudgeGrowthTokens`：**软压缩 nudge（软提醒）的门槛**，默认 `50000`。设置后同时映射到内核的 `nudge.growthFloor` 与 `nudge.growthCap`。
- 软 nudge 需要**同时**满足两道闸门：
  - 待压缩内容 ≥ `nudgeGrowthTokens`（本机为 **200000**）
  - 自上次 nudge 以来上下文增长 ≥ `max(20000, 0.45 × nudgeGrowthTokens)`（本机为 **90000**；默认 50000 时为 22500）
- 调到 200000 是为了**降低压缩频率**（默认值下提醒过于频繁）。代价是上下文会涨得更大才会被建议压缩；`compress.maxContextLimit`（默认窗口 75%）的强制 nudge 与 95% 的紧急截断不受此键影响，仍作兜底。
- 若要改成“上下文到某个绝对水位线（如 ~150K）就建议压缩”，应改 `compress.maxContextLimit`（按窗口百分比，1M 窗口约 `"14.53%"`），而非本键。
- **热生效**：插件在每个 `context` 事件（每次调用模型前）都会重新读取 `acp.json`，内容变化即应用，**无需 `/reload`、无需重启**；只有加载期读取的键（如 `enabled`、`toolPrompts`、`delegate.enabled`）才需要新会话。
- 验证：`grep 'config-reloaded' ~/.pi/acp.log | tail`，或看 `nudgeReason` 里的 `threshold` / `floor` 是否为新值。

### keybindings.json（全屏键盘滚动粒度）

pi 全屏（`tuiMode: "fullscreen"`）下的鼠标滚轮步长是内置行为：默认每格 1 行、按住 `Alt` 时 ×5，没有暴露成设置项，无法通过 settings.json / 扩展干净地修改；键盘滚动则通过 `~/.pi/agent/keybindings.json` 配置，把默认的整页滚动拆成三档，便于细看长输出：

```json
{
  "tui.altScreen.pageUp": ["ctrl+pageUp"],
  "tui.altScreen.pageDown": ["ctrl+pageDown"],
  "tui.altScreen.halfPageUp": ["pageUp"],
  "tui.altScreen.halfPageDown": ["pageDown"],
  "tui.altScreen.lineUp": ["alt+pageUp"],
  "tui.altScreen.lineDown": ["alt+pageDown"]
}
```

| 按键 | 行为 |
|------|------|
| `PageUp` / `PageDown` | 半页 |
| `Alt+PageUp` / `Alt+PageDown` | 单行 |
| `Ctrl+PageUp` / `Ctrl+PageDown` | 整页（pi 默认速度） |

注意事项：

- 这些动作只在 fullscreen 下生效；regular 模式下 `PageUp` / `PageDown` 仍用于编辑器翻页
- 修改后在运行中的 pi 里执行 `/reload` 即可生效，无需重启
- 部分终端（如 GNOME Terminal）会截获 `Ctrl+PageUp` / `Ctrl+PageDown` 用于切换标签页，若整页不生效可换成其他键
- fullscreen 下 `PageUp` / `PageDown` 改为半页滚动后，编辑器翻页不再有默认快捷键（影响很小）
- 可用动作与默认绑定见 pi 文档 `docs/keybindings.md`；`tui.altScreen.halfPageUp` / `halfPageDown` / `lineUp` / `lineDown` 默认不绑定

### 真彩色（truecolor）：避免 256 色降级

**问题：** 从 Windows Terminal 经 SSH 连接本机时，SSH 不会转发 `COLORTERM` 环境变量，登录 shell 里只有 `TERM=xterm-256color` 而 `COLORTERM` 为空。pi 据此判定终端不支持真彩色，把主题中的 24-bit 十六进制颜色降级到 256 色。pi 的近似算法按 6×6×6 色块取值，会把内置 `dark` 主题的工具背景色 `toolSuccessBg #283228` 映射成 256 色 22 号 `#005f00`，渲染出来就是刺眼的亮绿色大色块（工具输出框背景）；`toolErrorBg`、`userMessageBg` 等也有类似偏差。这是颜色降级导致的，并非主题配色本身的问题，走真彩色即可解决，无需自定义主题。

**解决：** 让 pi 输出真彩色，两种方式（建议都配）：

1. 在 `~/.bashrc` 末尾追加（SSH 重新登录后生效）：

```bash
# pi / truecolor
export COLORTERM=truecolor
```

2. 在 `~/.pi/agent/settings.json` 中强制开启（`terminal.trueColor` 是 JSON-only 高级选项，见 pi 文档 `docs/settings.md`）：

```json
{
  "terminal": {
    "trueColor": true
  }
}
```

**验证与注意事项：**

- `echo $COLORTERM` 应输出 `truecolor`
- 修改后需完全退出并重启 pi，正在运行的实例不会重新读取该设置；修改 `settings.json` 前先退出正在运行的 pi，否则它保存设置时可能覆盖手动加入的字段
- Windows Terminal、iTerm2、Kitty、WezTerm、VS Code 等现代终端都支持真彩，可以放心开启
- 若仍出现大色块怪色，优先检查 pi 启动所在 shell 的 `COLORTERM` 是否为空
