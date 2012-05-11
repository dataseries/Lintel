#!/bin/bash
set -e
maybeStartGPGAgent() {
    . $HOME/tmp/make-dist/gpg-agent.sh
    [ "`gpg-agent 2>&1`" = "gpg-agent: gpg-agent running and available" ] && exit 0
    echo "Starting gpg-agent..."
    gpg-agent --daemon --default-cache-ttl 28800 --max-cache-ttl 86400 >$HOME/tmp/make-dist/gpg-agent.sh
    exit 0
}
    
restartDaemon() {
    killall -TERM gpg-agent >/dev/null 2>&1 || true
    lintel-flock --filename=/tmp/make-dist.lock -- $0 --maybe-start-daemon
    . $HOME/tmp/make-dist/gpg-agent.sh
    gpg --use-agent --batch --no-tty -a --detach-sign --default-key Tesla_Software_Signing_Key $0
    rm $0.asc
    exit 0
}

case "$1" in
    --maybe-start-daemon) maybeStartGPGAgent ;;
    --restart-daemon) restartDaemon ;;

    *-*-*) : ;;
    *) echo "Usage: $0 <os>-<rel>-<arch>"; exit 1 ;;
esac

lintel-flock --filename=/tmp/make-dist.lock -- $0 --maybe-start-daemon
. $HOME/tmp/make-dist/gpg-agent.sh
if [ "`gpg-agent 2>&1`" != "gpg-agent: gpg-agent running and available" ]; then
    echo "Error: no gpg-agent?"
    exit 1
fi

ROOTDIR=/var/www/pb-pkgs/production

TRIPLE=$1
set -- `echo $TRIPLE | tr -- '-' ' '`
OS=$1
VER=$2
ARCH=$3

signRPMs() {
    cat $HOME/.rpmmacros >/tmp/$TRIPLE.rpmmacros
    cat >>/tmp/$TRIPLE.rpmmacros <<EOF
%__gpg_check_password_cmd /bin/true
%__gpg_sign_cmd /usr/bin/gpg /usr/bin/gpg --force-v3-sigs --batch --no-verbose --no-armor --use-agent -u "%{_gpg_name}" -sbo %{__signature_filename} %{__plaintext_filename}
%_signature gpg
%_gpg_name  Tesla_Software_Signing_Key
EOF
    cat >/tmp/$TRIPLE.rpm-sign <<EOF
#!/usr/bin/expect -f
spawn rpm --macros=/tmp/$TRIPLE.rpmmacros --resign {*}\$argv
expect -exact "Enter pass phrase: "
send -- "Secret passphrase\r"
expect eof
EOF
    chmod +x /tmp/$TRIPLE.rpm-sign
    rpm --delsign $ROOTDIR/$OS/$VER/$ARCH/*.rpm
    /tmp/$TRIPLE.rpm-sign $ROOTDIR/$OS/$VER/$ARCH/*.rpm
    createrepo $ROOTDIR/$OS/$VER/$ARCH
#    rm /tmp/$TRIPLE.rpmmacros /tmp/$TRIPLE.rpm-sign
}

case $OS in
    debian|ubuntu) 
        TO_SIGN=$ROOTDIR/$OS/dists/$VER/Release; 
        PUBKEYS="$ROOTDIR/$OS/$VER/Lintel.pubkey $ROOTDIR/$OS/$VER/DataSeries.pubkey"
        ;;
    centos|fedora|scilinux)
        signRPMs
        TO_SIGN=$ROOTDIR/$OS/$VER/$ARCH/repodata/repomd.xml;
        PUBKEYS="$ROOTDIR/$OS/$VER/$ARCH/Lintel.pubkey $ROOTDIR/$OS/$VER/$ARCH/DataSeries.pubkey";
        ;;
    opensuse)
        signRPMs
        TO_SIGN=$ROOTDIR/$OS/$VER/$ARCH/repodata/repomd.xml;
        PUBKEYS="$ROOTDIR/$OS/$VER/$ARCH/repodata/repomd.xml.key $ROOTDIR/$OS/$VER/$ARCH/Lintel.pubkey $ROOTDIR/$OS/$VER/$ARCH/DataSeries.pubkey";
        ;;
    *) echo "Unknown OS '$OS'"; exit 1 ;;
esac
# --display $DISPLAY 
for file in $TO_SIGN; do
    [ ! -f $file.asc ] || rm $file.asc
    echo "Signing $file"
    gpg --use-agent --batch --no-tty -a --detach-sign --default-key Tesla_Software_Signing_Key $file
    case $OS in
        debian|ubuntu) mv $file.asc $file.gpg ;;
    esac
done

for file in $PUBKEYS; do
    gpg --export -a Tesla_Software_Signing_Key >$file 
done

echo "Signed repo for $TRIPLE"

