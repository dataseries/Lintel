#!/bin/sh
BUILD=`pwd`
mtn -d $BUILD/tmp.db co -b ssd.hpl.hp.com/`echo $1 | sed 's/-.*//'` $1
cd $1
echo "Monotone-Revision: `mtn automate get_base_revision_id`" >Release.info
echo "Creation-Date: $NOW" >>Release.info
echo "   current revision is `grep Monotone-Revision Release.info | awk '{print $2}'`"
`dirname $0`/mtn-log-sort >Changelog.mtn
