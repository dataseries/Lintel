#
#  (c) Copyright 2000-2005, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#

require 5.005;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
use Config;
if ($Config{osname} eq 'hpux') {
	$cc = 'aCC';
	$warn_flags = '+W749,829'
} elsif ($Config{osname} eq 'linux') {
	$cc = 'g++';
} else {
	die "Unknown c++ compiler for $Config{osname}\n";
}
WriteMakefile(
    'CC' => $cc,
    'LD' => $cc,
    'NAME'	=> 'Lintel',
    'VERSION_FROM' => 'Lintel.pm', # finds $VERSION
    'LIBS'	=> ['-L.. -lLintel-optimize -ltcl'],   # e.g., '-lm' 
    'DEFINE'	=> $warn_flags,     # e.g., '-DHAVE_SOMETHING' 
    'INC'	=> '-I..',     # e.g., '-I/usr/include/other' 
    'XSOPT'     => ''
);

