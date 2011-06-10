#!/bin/sh
set -e
set -x
restore_dbs() {
    if [ -x /usr/bin/db45_load ]; then
        LOAD=/usr/bin/db45_load
    else
        LOAD=/usr/bin/db_load
    fi
    cd /var/lib/rpm
    rm __db.* || true
    
    for i in *.dump; do
        j=`basename $i .dump`
        mv $j $j.orig
        echo "restore $j from $i"
        $LOAD $j < $i
    done
}

if [ "$1" = "--shell" ]; then
    /bin/sh
    exit 0
elif [ "$1" = "--debian" ]; then
    /tmp/hooks/D00apt-key
    apt-key list
    apt-get -y update

    LINTEL_PACKAGES="liblintel0 liblintelpthread0 lintel-utils liblintel-dev-doc liblintel-dev liblintel-perl"
    DATASERIES_PACKAGES="libdataseries0 dataseries-utils libdataseries-dev-doc libdataseries-dev"
    for package in $LINTEL_PACKAGES $DATASERIES_PACKAGES; do
        echo "----------------- $package"
        apt-get -y install $package
        apt-get -y remove $package
        apt-get -y install $package
        apt-get -y purge $package
        apt-get -y autoremove
    done
    apt-get -y install $LINTEL_PACKAGES $DATASERIES_PACKAGES
elif [ "$1" = "--yum" ]; then
    (restore_dbs) # () to make cd in function not change our working directory
    rm -rf /var/cache/yum/local
    LINTEL_PACKAGES="Lintel-debuginfo Lintel-devel Lintel-docs Lintel-utils perl-Lintel Lintel-libs"
    DATASERIES_PACKAGES="DataSeries-debuginfo DataSeries-devel DataSeries-docs DataSeries-utils DataSeries-libs"
    for package in $LINTEL_PACKAGES $DATASERIES_PACKAGES; do
        echo "----------------- $package"
        yum -y install $package
        yum -y erase $package
        # no equivalent to purge/autoremove?
    done
    yum -y install $LINTEL_PACKAGES $DATASERIES_PACKAGES
elif [ "$1" = "--zypper" ]; then
    # db fixup happened during entry to chroot
    LINTEL_PACKAGES="Lintel-devel Lintel-docs Lintel-utils perl-Lintel Lintel-libs"
    DATASERIES_PACKAGES="DataSeries-devel DataSeries-docs DataSeries-utils DataSeries-libs"
    for package in $LINTEL_PACKAGES $DATASERIES_PACKAGES; do
        echo "----------------- $package"
        zypper install -t package -y $package
        zypper remove -t package -y $package
        # no equivalent to purge/autoremove?
    done
    zypper install -t package -y $LINTEL_PACKAGES $DATASERIES_PACKAGES
else
    echo "Usage: $0 --os-type"
    false
fi
# check a smattering of programs
[ ! -f /usr/share/bp_modules/BatchParallel/nettrace2ds.pm ] || batch-parallel nettrace2ds help
batch-parallel make help
csv2ds -h
dsstatgroupby --help
lintel-latex-rebuild --selfcheck
deptool help init
exit 0
