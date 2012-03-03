#!/bin/sh
set -e -x
case `pwd` in
   */Lintel | */Lintel.0.20[0-9][0-9].[0-9][0-9].[0-9][0-9])
      : 
      ;;
   *) echo "Being executed in the wrong directory, should be in .../Lintel"
      exit 1
      ;;
esac

if [ -d .git ]; then
    ./dist/make-release-changelog.sh
fi

if [ ! -f Release.info -o ! -f ChangeLog ]; then
    echo "Error: Ought to either both Release.info and Changelog.mtn from a release or should have just created it from monotone repository"
    exit 1
fi

. redhat/get-version.sh

cwd=`pwd`
dir=`basename $cwd`
cd ..
[ "$dir" = "Lintel-$VERSION" -o -d Lintel-$VERSION ] || ln -s $dir Lintel-$VERSION
tar cvvfhz Lintel-$VERSION.tar.gz --exclude=Lintel-$VERSION/.git Lintel-$VERSION/
[ "$dir" = "Lintel-$VERSION" ] || rm Lintel-$VERSION

mkdir -p $rpm_topdir/SOURCES
cp Lintel-$VERSION.tar.gz $rpm_topdir/SOURCES/Lintel-$VERSION.tar.gz
