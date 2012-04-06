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

. $1/version
./dist/make-release-changelog.sh

cwd=`pwd`
dir=`basename $cwd`
cd ..
[ "$dir" = "Lintel-$RELEASE_VERSION" ] || ln -snf $dir Lintel-$RELEASE_VERSION
tar cvvfhz Lintel-$RELEASE_VERSION.tar.gz --exclude=Lintel-$RELEASE_VERSION/.git --exclude=\*\~ Lintel-$RELEASE_VERSION/
[ "$dir" = "Lintel-$RELEASE_VERSION" ] || rm Lintel-$RELEASE_VERSION

mv Lintel-$RELEASE_VERSION.tar.gz /var/www/pb-sources
