if [ ! -f debian/changelog ]; then
    echo "Missing debian/changelog; can't extract consistent version"
    exit 1
fi
VERSION=`perl -ne 'if (/^lintel \(([0-9\.]+)-(\d+)\) /o) { print "$1\n"; exit(0);} ' <debian/changelog`
if [ -z "$VERSION" ]; then
    echo "missing version in debian/changelog"
    exit 1
fi
export VERSION
RELEASE=`perl -ne 'if (/ $ENV{VERSION}-(\d+)$/o) { print "$1\n"; exit(0);} ' <redhat/Lintel.spec.in`

if [ -z "$RELEASE" ]; then
    echo "missing release for version $VERSION in redhat/Lintel.spec.in"
    exit 1
fi
