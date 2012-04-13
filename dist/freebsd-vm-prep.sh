#!/bin/sh
export PACKAGEROOT=ftp://ftp4.us.freebsd.org/
export HTTP_PROXY=http://192.168.122.1:3128/
pkg_add -r sudo zsh perl cmake boost-libs p5-Digest-SHA1 git wget libxml2
cat >/root/.zshrc <<EOF
export PACKAGEROOT=ftp://ftp4.us.freebsd.org/
export HTTP_PROXY=http://192.168.122.1:3128/
export http_proxy=http://192.168.122.1:3128/
EOF
echo "anderse ALL=(ALL) NOPASSWD: ALL" >>/usr/local/etc/sudoers
cp /root/.zshrc /home/anderse/.zshrc
chmod +x /home/anderse/.zshrc
chsh -s zsh anderse
chsh -s zsh root
mkdir /home/anderse/.ssh
echo 'ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAIEA2oOZJWek+M1jLDY1hzMcE/F+A8KQEXyuP83YsVIqXYXYX0sRVKZBwwQIUdx1Hqc9iprNmjcUO/Xkcz6Z3ssPcR0be4XVlvSoB6GBHh1ID2Yc7ksSBzDreGF00tkhsbC9jsHFUDrSJp2OQreNb3ZIrFMsxNq4PIRx1Hst6VP5cqk= anderse@eric-nw8k' >/home/anderse/.ssh/authorized_keys
chown anderse /home/anderse/.ssh
chown anderse /home/anderse/.ssh/authorized_keys
