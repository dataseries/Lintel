#!/bin/sh -x
if [ $# != 2 ]; then
    echo "Usage: <os-version-arch> <pbuilder-hookdir>"
    exit 1
fi
VERSION_REL=$1
PBUILDER_HOOKDIR=$2

set -e

[ `id -u` = 0 ]
PBR=/var/cache/pbuilder/result/$VERSION_REL
[ -d $PBR ] || mkdir $PBR
LP=/var/www/localpkgs/$VERSION_REL
[ -L $LP ] || ln -s $PBR $LP

cd $LP
HOME=/var/www/localpkgs

[ -d conf ] || mkdir conf
cat >conf/distributions <<EOF 
Origin: Eric Anderson
Label: localpkgs
Suite: unstable
Codename: sid
Architectures: i386 amd64 all source
Components: main
Description: Local Packages
SignWith: na@example.com
EOF

if [ `ls incoming/*changes | wc -w` != 0 ]; then
    rm -rf dists lists pool db
    for i in incoming/*changes; do
        echo "XXX $i"
        reprepro include sid $i
    done
else
    reprepro export
fi

[ -d pool ] || mkdir pool

for i in `find dists pool -type f`; do 
    curl http://localhost/localpkgs/$VERSION_REL/$i >/dev/null 2>&1
done

if [ ! -f $PBUILDER_HOOKDIR/D00apt-key ]; then
    KEYDATA=`gpg --export | perl -e 'undef $/; $a = <STDIN>; print unpack("H*", $a), "\n"; '`
    mkdir -p $PBUILDER_HOOKDIR
    cat >$PBUILDER_HOOKDIR/D00apt-key.$$ <<'EOF'
#!/usr/bin/perl
print "---------------------------------------\n";
print "Running hook script...\n";
open(FILE, ">/tmp/key") or die "?1";
print FILE pack("H*", "__KEYDATA__");
close(FILE);
system("apt-get -y --force-yes install gnupg") == 0 or die "?6";
system("apt-key add /tmp/key") == 0 or die "?2";
system("rm -rf /var/lib/apt/lists/localhost*") == 0 or die "?4";
system("apt-get -y update") == 0 or die "?3";
# latex over 5 years old complains on install, hack the file so that
# it won't complain.
if (-f "/etc/lsb-release") {
    open(FILE, "/etc/lsb-release") or die "?5";
    if (grep(/hardy/, <FILE>) > 0) { 
        system("apt-get -y --force-yes install texlive-latex-base");
        my $file = "/usr/share/texmf-texlive/tex/latex/base/latex.ltx";
        open(FILE, $file) or die "?7";
        my @data = <FILE>;
        grep(s/^(.ifnum.count..)65/${1}99/, @data);
        open(FILE, ">$file") or die "?8";
        print FILE @data;
        close(FILE);
        print "Hacked latex.ltx to work, and reconfiguring...\n";
        system("dpkg --configure -a") == 0 or die "?9";
    }
}
EOF
    sed -e "s/__KEYDATA__/$KEYDATA/" -i $PBUILDER_HOOKDIR/D00apt-key.$$
    chmod +x $PBUILDER_HOOKDIR/D00apt-key.$$
    mv $PBUILDER_HOOKDIR/D00apt-key.$$ $PBUILDER_HOOKDIR/D00apt-key
fi
# pbuilder --update --basetgz /var/cache/pbuilder/$VERSION_REL.tgz


