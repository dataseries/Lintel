#!/bin/sh
case `pwd` in
   */Lintel | */Lintel.0.20[0-9][0-9].[0-9][0-9].[0-9][0-9])
      : 
      ;;
   *) echo "Being executed in the wrong directory, should be in .../Lintel"
      exit 1
      ;;
esac

./dist/make-release-changelog.sh
. redhat/get-version.sh

cwd=`pwd`
dir=`basename $cwd`
cd ..
[ "$dir" == "Lintel-$VERSION" ] || ln -s $dir Lintel-$VERSION
tar cvvfhz Lintel-$VERSION.tar.gz --exclude=Lintel-$VERSION/_MTN Lintel-$VERSION/
[ "$dir" == "Lintel-$VERSION" ] || rm Lintel-$VERSION

cp Lintel-$VERSION.tar.gz $rpm_topdir/SOURCES/Lintel-$VERSION.tar.gz
