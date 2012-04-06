#!/bin/sh
[ $# = 2 ] || exit 1
OS=$1
FILE=$2

fixup_yum() {
    RPMS=`grep http $FILE | grep 'Package does not match intended' | awk '{print $1}' | sed 's/:$//' | sort | uniq`
    if [ `echo $RPMS | wc -w` -ge 1 ]; then
        http_proxy=http://localhost:3128/
        export http_proxy
        for i in $RPMS; do
            echo -n "$i: "
            wget -O - --no-cache $i 2>/dev/null | sha1sum
        done
    fi
}

case $OS in
    *centos*|*fedora*|*scilinux*) fixup_yum ;;
    *) echo "Don't know how to fixup download errors on $OS" ;;
esac
