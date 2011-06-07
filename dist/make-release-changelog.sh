update() {
    if cmp /tmp/$1.$$ $1 >/dev/null 2>&1; then
        rm /tmp/$1.$$
    else
        mv /tmp/$1.$$ $1
    fi
}

if [ -d _MTN ]; then
    echo "Monotone-Revision: `mtn automate get_base_revision_id`" >/tmp/Release.info.$$
    echo "Creation-Date: `date +%Y-%m-%d-%H-%M`" >>/tmp/Release.info.$$
    echo "BEGIN_EXTRA_STATUS" >>/tmp/Release.info.$$
    mtn status >>/tmp/Release.info.$$
    echo "END_EXTRA_STATUS" >>/tmp/Release.info.$$
    `dirname $0`/../dist/mtn-log-sort >/tmp/Changelog.mtn.$$
    update Release.info
    update Changelog.mtn
fi

if [ ! -f Release.info -o ! -f Changelog.mtn ]; then
    echo "Error: Ought to either both Release.info and Changelog.mtn from a release or should have just created it from monotone repository"
    exit 1
fi

