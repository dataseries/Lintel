update() {
    if cmp /tmp/$1.$$ $1 >/dev/null 2>&1; then
        rm /tmp/$1.$$
    else
        mv /tmp/$1.$$ $1
    fi
}

if [ -d .git ]; then
    echo "Git-Revision: `git show --format=%H HEAD | head -1`" >/tmp/Release.info.$$
    echo "Creation-Date: `date +%Y-%m-%d-%H-%M`" >>/tmp/Release.info.$$
    echo "BEGIN_EXTRA_STATUS" >>/tmp/Release.info.$$
    git status >>/tmp/Release.info.$$
    echo "END_EXTRA_STATUS" >>/tmp/Release.info.$$
    git log >/tmp/ChangeLog.$$
    update Release.info
    update ChangeLog
fi

if [ ! -f Release.info -o ! -f ChangeLog ]; then
    echo "Error: Ought to either both Release.info and ChangeLog from a release or should have just created it from monotone repository"
    exit 1
fi

