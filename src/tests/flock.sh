#!/bin/sh
if [ "$1" = "-count" ]; then
    set -e
    F=`cat $LOCKFILE-count`
    perl -e 'select(undef,undef,undef,0.25)'
    echo `expr $F + 1` >$LOCKFILE-count
    exit 0
fi

if [ "$1" = "-bglock" ]; then
    perl $builddir/lintel-flock --filename=$LOCKFILE --callpid=$$
    sleep 2
    echo "Exiting..."
    exit 0
fi

if [ $# != 3 ]; then
    echo "Usage: $0 <srcdir> <builddir> <installdir>"
    exit 1
fi

srcdir=$1
builddir=$2
installdir=$3
LOCKFILE=`mktemp`
PERL5LIB=$srcdir:$installdir/share/perl5:$PERL5LIB

export srcdir builddir installdir LOCKFILE PERL5LIB

verify_unlocked() {
    V=`$builddir/lintel-flock --filename=$LOCKFILE --command=true --waittime=0`
    if [ $? != 0 ]; then
	echo "$LOCKFILE unexpectedly locked"
	exit 1
    fi
}

verify_locked() {
    V=`$builddir/lintel-flock --filename=$LOCKFILE --command=true --waittime=0`
    if [ $? = 0 ]; then
	echo "$LOCKFILE unexpectedly unlocked"
	exit 1
    fi
    if [ "$V" != "Unable to get lock, not running command" ]; then
	echo "ERROR: locked but successful exit? ($V)"
	exit 1
    fi
}
    
### Blocked lock test

verify_unlocked
echo "Starting sleep..."
$builddir/lintel-flock --filename=$LOCKFILE --command='sleep 10' &
SLEEP_PID=$!
echo "Starting lock timeout..."
V=`$builddir/lintel-flock --filename=$LOCKFILE --command=false --waittime=2`
verify_locked
echo "lock timeout succeeded"
kill $SLEEP_PID
wait
echo "Done with blocked lock test"

### Parallel test

verify_unlocked
echo "Starting parallel test ($0)"
echo 0 >$LOCKFILE-count
for i in 1 2 3 4 5 6 7 8 9 10; do
    $builddir/lintel-flock --filename=$LOCKFILE --command="$0 -count $LOCKFILE-count" &
done
wait
if [ "`cat $LOCKFILE-count`" = "10" ]; then
    echo "Success on parallel test"
else
    echo "Failed parallel test `cat $LOCKFILE-count` != 10"
    exit 1
fi

rm $LOCKFILE-count

### Scripting test 1

echo "Simple script test..."
verify_unlocked

$0 -bglock &
perl -e 'select(undef,undef,undef,0.5);'
verify_locked
sleep 3
verify_unlocked

### Scripting test 2

echo "Interchange test..."
verify_unlocked
[ -f $LOCKFILE-locked ] && exit 1

(
    for i in 1 2 3; do
	echo "  a-lock-$i"
	unlock=`$builddir/lintel-flock --filename=$LOCKFILE --callpid=$$`
	case $unlock in 
	    success:*) : ;;
            *) echo "?"; exit 1 ;;
        esac

	echo "  a-wait-$i"
        while [ ! -f $LOCKFILE-locked-$i ]; do
	    perl -e 'select(undef,undef,undef,0.1);'
	done

	# other side has seen us locked.

	echo "  a-cleanup-$i"
	[ -f $LOCKFILE-locked-$i ] || exit 1
	rm $LOCKFILE-locked-$i
	
	$builddir/lintel-flock --unlock="$unlock"

	while [ ! -f $LOCKFILE-unlocked-$i ]; do
	    perl -e 'select(undef,undef,undef,0.1);'
	done

	echo "  a-unlocked-$i"
	# other side has seen us unlocked
	[ -f $LOCKFILE-unlocked-$i ] || exit 1
	rm $LOCKFILE-unlocked-$i
    done
    touch $LOCKFILE-success-a
) &

(
    for i in 1 2 3; do
	echo "  b-wait-$i"
	waiting=true
	while $waiting; do
	    unlock=`$builddir/lintel-flock --filename=$LOCKFILE --callpid=$$ --waittime=1`
	    case $unlock in 
		success:*) $builddir/lintel-flock --unlock="$unlock" ;;
	        timeout) waiting=false ;;
		*) echo "?"; exit 1 ;;
	    esac
        done

	# seen them locked.

	echo "  b-locked-$i"
	touch $LOCKFILE-locked-$i
	
	echo "  b-wait2-$i"
	$builddir/lintel-flock --filename=$LOCKFILE --command=true
	[ $? = 0 ] || exit 1
	
	echo "  b-unlocked-$i"
	verify_unlocked
	touch $LOCKFILE-unlocked-$i
    done
    touch $LOCKFILE-success-b
) &

wait
if [ -f $LOCKFILE-success-a -a -f $LOCKFILE-success-b ]; then
    echo "Success on interchange test"
else
    echo "Interchange test failed"
    exit 1
fi

rm $LOCKFILE-success-[ab]

### cleanup

rm $LOCKFILE
echo "Success."



