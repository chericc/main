# 开发环境的一些配置

[toc]

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
sudo apt-get install nfs-kernel-server
```

配置文件

```bash
# /etc/exports
/home/username *(rw,sync,no_subtree_check,no_root_squash)
```

重启服务

```bash
sudo service nfs-kernel-server restart
```

测试

```bash
sudo mount -t nfs localhost:/home/username /mnt 
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

```

### 配置pip源

```bash
python -m pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple/
python -m pip config set install.trusted-host pypi.tuna.tsinghua.edu.cn
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

export http_proxy="http://10.0.0.2:7897"
export https_proxy="http://10.0.0.2:7897"

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

## netplan

```bash

# This is the network config written by 'subiquity'
network:
  ethernets:
    enp0s3:
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
    enp0s9:
      dhcp4: yes
  version: 2

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

# 右下角类似效果：21:58:48 12-12
set -g status-right "%H:%M:%S %d-%b"

# 设置整个状态栏背景颜色 bg(背景色) fg(前景色)
set -g status-bg blue
set -g status-fg white
# set -g status-style bold
# set -g window-active-style fg=white,bg=blue,bold
# set -g window-style fg=gray,bg=black

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

### 不能调用输入法的问题

```bash
export GTK_IM_MODULE=xim
export QT_IM_MODULE=xim
export XMODIFIERS=@im=fcitx

source ~/.bashrc; code
```