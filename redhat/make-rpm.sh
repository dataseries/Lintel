#!/bin/sh
case `pwd` in
   */Lintel | */Lintel.0.20[0-9][0-9].[0-9][0-9].[0-9][0-9])
      : 
      ;;
   *) echo "Being executed in the wrong directory, should be in .../Lintel"
      exit 1
      ;;
esac

if [ -d _MTN ]; then
    if [ -f Release.info ]; then
	echo "Warning, overwriting Release.info with current information from monotone"
	rm Release.info
    fi
    echo "Monotone-Revision: `mtn automate get_base_revision_id`" >Release.info
    echo "Creation-Date: `date +%Y-%m-%d`" >>Release.info
    echo "BEGIN_EXTRA_STATUS" >>Release.info
    mtn status >>Release.info
    echo "END_EXTRA_STATUS" >>Release.info
    mtn log >Changelog.mtn
fi

if [ ! -f Release.info -o ! -f Changelog.mtn ]; then
    echo "Error: Ought to either both Release.info and Changelog.mtn from a release or should have just created it from monotone repository"
    exit 1
fi

REL_DATE=`grep Creation-Date Release.info | awk '{print $2}'`
REL_VERSION=0.`echo $REL_DATE | sed 's/-/./g'`
sed "s/__VERSION__/$REL_VERSION/" <redhat/Lintel.spec.in >redhat/Lintel.spec

VERSION=`grep Version: redhat/Lintel.spec | awk '{print $2}'`
if [ "$VERSION" = "" ]; then
    echo "Missing version in Lintel.spec"
    exit 1
fi

if [ "$VERSION" != "$REL_VERSION" ]; then
    echo "Bad version in redhat/Lintel.spec; $VERSION != $REL_VERSION"
    exit 1
fi

rpm_topdir=`grep '^._topdir ' $HOME/.rpmmacros | awk '{print $2}'`

[ "$rpm_topdir" == "" ] && rpm_topdir=/usr/src/redhat

cwd=`pwd`
dir=`basename $cwd`
cd ..
[ "$dir" == "Lintel-$VERSION" ] || mv $dir Lintel-$VERSION
tar cvvfz $rpm_topdir/SOURCES/Lintel-$VERSION.tar.gz Lintel-$VERSION
[ "$dir" == "Lintel-$VERSION" ] || mv Lintel-$VERSION $dir 
cd $dir

rpmbuild -ba redhat/Lintel.spec || exit 1

if [ -d _MTN ]; then
    rm Release.info
    rm Changelog.mtn
    rm redhat/Lintel.spec
fi

echo "SUCCESS: built rpm package"
