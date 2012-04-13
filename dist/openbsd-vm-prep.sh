#!/bin/sh
set -e
set -x
echo 'ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAIEA2oOZJWek+M1jLDY1hzMcE/F+A8KQEXyuP83YsVIqXYXYX0sRVKZBwwQIUdx1Hqc9iprNmjcUO/Xkcz6Z3ssPcR0be4XVlvSoB6GBHh1ID2Yc7ksSBzDreGF00tkhsbC9jsHFUDrSJp2OQreNb3ZIrFMsxNq4PIRx1Hst6VP5cqk= anderse@eric-nw8k' >/home/anderse/.ssh/authorized_keys
chown anderse /home/anderse/.ssh
chown anderse /home/anderse/.ssh/authorized_keys

if [ `ifconfig -a | grep 192.168.184 | wc -l` = 1 ]; then
    HOST_ADDR=192.168.184.1
else
    HOST_ADDR=192.168.122.1
fi

export PKG_PATH=http://ftp5.usa.openbsd.org/pub/OpenBSD/`uname -r`/packages/`uname -m`/
export http_proxy=http://$HOST_ADDR:3128/
pkg_add -r zsh cmake boost p5-Digest-SHA1 git wget libxml

cat >/root/.zshrc <<EOF
export PACKAGEROOT=ftp://ftp4.us.freebsd.org/
export HTTP_PROXY=http://$HOST_ADDR:3128/
export http_proxy=http://$HOST_ADDR:3128/
EOF
echo "anderse ALL=(ALL) NOPASSWD: ALL" >>/etc/sudoers
cp /root/.zshrc /home/anderse/.zshrc
chmod +x /home/anderse/.zshrc
chsh -s zsh anderse
chsh -s zsh root
