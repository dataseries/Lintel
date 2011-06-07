#!/bin/sh
. redhat/get-version.sh
perl redhat/patch-spec.pl $PATCH_SPEC_OS $VERSION $RELEASE >redhat/Lintel.spec || exit 1

CHECK_VERSION=`grep Version: redhat/Lintel.spec | awk '{print $2}'`
if [ "$CHECK_VERSION" = "" -o "$CHECK_VERSION" = "0." ]; then
    echo "Missing version in Lintel.spec"
    exit 1
fi

if [ "$CHECK_VERSION" != "$VERSION" ]; then
    echo "Bad version in redhat/Lintel.spec; $CHECK_VERSION != $VERSION"
    exit 1
fi
