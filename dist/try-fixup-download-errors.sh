#!/bin/sh
[ $# = 2 ] || exit 1
OS=$1
FILE=$2

download() {
    PKGS="$1"
    if [ `echo $PKGS | wc -w` -ge 1 ]; then
        http_proxy=http://localhost:3128/
        export http_proxy
        for i in $PKGS; do
            echo -n "$i: "
            wget -O - --no-cache $i 2>/dev/null | sha1sum
        done
    fi
}

fixup_yum() {
    download "`grep http $FILE | grep 'Package does not match intended' | awk '{print $1}' | sed 's/:$//' | sort | uniq`"
}

fixup_apt() {
    download "`grep 'Failed to fetch' $FILE | grep http | awk '{print $4}' | sort | uniq`"
    download "`grep 'was corrupt' $FILE | grep http | awk '{print $2}' | sort | uniq`"
}


case $OS in
    *centos*|*fedora*|*scilinux*) fixup_yum ;;
    *debian*|*ubuntu*) fixup_apt ;;
    *) echo "Don't know how to fixup download errors on $OS" ;;
esac
