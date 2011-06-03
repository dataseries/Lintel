#!/bin/sh -x
VERSION_REL=$1
set -e

PBR=/var/cache/pbuilder/result/$VERSION_REL
[ -d $PBR ] || mkdir $PBR
LP=/var/www/localpkgs/$VERSION_REL
[ -L $LP ] || ln -s $PBR $LP

cd $LP
apt-ftparchive packages . >Packages
apt-ftparchive sources . >Sources
rm Packages.gz Sources.gz 2>/dev/null || true
gzip -9v Packages Sources

cat >Release.config <<EOF 
APT::FTPArchive::Release {
    Origin "local-packages";
    Label "local-packages";
    Suite "custom";
    Architectures "amd64 i386";
    Description "Local rebuild packages";
}
EOF

apt-ftparchive -c=Release.config release . >Release
# sudo HOME=/var/www/localpkgs gpg --gen-key
# Real name: Local packages signing key
# Email address: na@example.com
# Comment: local package signing key
# empty passphrase
rm Release.gpg
HOME=/var/www/localpkgs gpg --sign -ba -o Release.gpg Release

