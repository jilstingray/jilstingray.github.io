---
layout: post
title: 搭建 CloudBeaver 集群 (伪)
date: 2024-10-30
mathjax: false
---

Cloudbeaver 社区版本身不支持集群模式，但是可以通过一些手段来实现。


## 工作区文件同步

Cloudbeaver 的工作区、数据源等配置以文件形式存储在 workspace 目录下，为了确保各节点的信息一致，可以用 rsync + inotify 实现文件同步。

这里将每个节点都同时配置为主从节点，每个节点都可以接收文件变更，然后通过 rsync 同步到其他节点。

首先需要安装 rsync 和 inotify-tools。

```bash
sudo yum -y install rsync inotify-tools
```

创建权限为 400 的文件存储 rsync 用户密码，主节点格式为 `password`，从节点格式为 `username:password`。

```bash
sudo echo "password" > /root/rsyncd.passwd
sudo chown root:root /root/rsyncd.passwd
sudo chmod 400 /root/rsyncd.passwd

sudo echo "cbuser:password" > /etc/rsyncd.passwd
sudo chown root:root /etc/rsyncd.passwd
sudo chmod 400 /etc/rsyncd.passwd
```

编辑 rsync 配置文件 /etc/rsyncd.conf。

```bash
# /etc/rsyncd.conf
uid = root
gid = root
use chroot = no
max connections = 10
strict modes = no
pid file = /var/run/rsyncd.pid
lock file = /var/run/rsyncd.lock
log file = /var/log/rsyncd.log

[cloudbeaver]
path = /opt/cloudbeaver/workspace/
comment = cloudbeaver
ignore errors
read only = no
write only = no
hosts allow = 192.168.5.0/24
list = false
uid = cbuser
gid = cbuser
auth users = cbuser
secrets file = /etc/rsyncd.passwd
```

启动 rsyncd 服务后, 测试同步是否能正常工作。

```bash
sudo rsync -vzrtopg --delete --progress /opt/cloudbeaver/workspace/ cbuser@192.168.5.31::cloudbeaver --password-file=/root/rsyncd.passwd
```

配置 inotify 脚本监控文件变更。

```bash
#!/bin/bash
# inotify-tools.sh

/usr/bin/inotifywait -mrq --timefmt '%d/%m/%y %H:%M' --format '%T %w%f%e' -e close_write, delete, create, attrib /opt/cloudbeaver/workspace/ | while read file

rsync -vzrtopg -- delete -- progress /opt/cloudbeaver/workspace/ cbuser@192.168.5.31::cloudbeaver --password-file=/root/rsyncd.passwd
echo "${file} was rsynced to 192.168.5.31" >> /tmp/inotify-tools.log 2>&1
# 以此类推可以添加多个节点
done
```

```bash
nohup /bin/bash /root/inotify-tools.sh &
```

添加开机启动

```bash
echo "/bin/bash /root/inotify-tools.sh &" >> /etc/rc.local
```

## 配置 Nginx 一致性模块

Cloudbeaver 多个节点无法共享会话状态，为了确保用户在一次会话内的每次请求都能到达同一个节点，需要配置 Nginx 的 ip_hash 一致性模块，只要用户的 IP 不改变，用户当前的会话就能够保持下去。

```nginx
stream {
	upstream cloudbeaver {
        hash $request_uri consistent;
        server 192.168.5.30:8978 weight=1;
        server 192.168.5.31:8978 weight=1;
    }
        
    # 七层代理
    server {
        listen 18978;
        server_name xxxxxxxxxx;

        location / {
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header REMOTE-HOST $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_pass http://cloudbeaver;
        }
    }

    # 四层代理
    server {
        listen 18978;
        proxy_pass cloudbeaver;
    }
}
```

接下来用户只需要访问 Nginx 的 18978 端口就可以了，不同 IP 的用户会被分配到不同的节点上。

## 参考

[使用Rsync结合Inotify实现双机文件热备](https://www.chancel.me/markdown/using-rsync-with-inotify-to-achieve-dual-machine-file-hot-backup)

[nginx 添加 hash一致性模块 并配置使用](https://www.cnblogs.com/guanxiaohe/p/16227526.html)
