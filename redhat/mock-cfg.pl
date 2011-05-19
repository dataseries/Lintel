#!/usr/bin/perl -w
use strict;

my ($os, $version, $arch) = split(/-/o, $ARGV[0]);

eval "$os()";
die $@ if $@;

#
#if ($os eq 'centos') {
#    centos();
#} else {
#    die "Unknown os '$os'";
#}

sub writeConfig {
    my ($cfg) = @_;

    my $dest = "/etc/mock/lintel-$os-$version-$arch.cfg";
    open(OUT, ">$dest-tmp") || die "Can't open $dest-tmp for write: $!";
    print OUT $cfg || die "write failed: $!";
    close(OUT) || die "close failed: $!";
    rename("$dest-tmp", $dest) || die "rename $dest-tmp $dest failed: $!";
}

sub centos {
    my $base_version = $version;
    $base_version =~ s/\..*$//o;
    my $mirror = "http://mirrors2.kernel.org/centos";
    my $epel_mirror = "http://mirror.hmc.edu/epel";
    my $cfg = <<"END_OF_CENTOS_CFG";
config_opts['root'] = 'lintel-centos-$version-$arch'
config_opts['target_arch'] = '$arch'
config_opts['chroot_setup_cmd'] = 'install buildsys-build'

config_opts['yum.conf'] = """
[main]
cachedir=/var/cache/yum
debuglevel=1
logfile=/var/log/yum.log
reposdir=/dev/null
retries=20
obsoletes=1
gpgcheck=0
assumeyes=1
# grub/syslinux on x86_64 need glibc-devel.i386 which pulls in glibc.i386, need to exclude all
# .i?86 packages except these.
exclude=[1-9A-Za-fh-z]*.i?86 g[0-9A-Za-km-z]*.i?86 gl[0-9A-Za-hj-z]*.i?86 gli[0-9A-Zac-z]*.i?86 glib[0-9A-Za-bd-z]*.i?86
# repos

[core]
name=base
baseurl=$mirror/$version/os/$arch/

[update]
name=updates
baseurl=$mirror/$version/updates/$arch/

[groups]
name=groups
baseurl=http://buildsys.fedoraproject.org/buildgroups/rhel$base_version/$arch/

[extras]
name=epel
baseurl=$epel_mirror/$base_version/$arch

[local]
name=local
baseurl=http://kojipkgs.fedoraproject.org/repos/dist-${base_version}E-epel-build/latest/$arch/
cost=2000
enabled=0

"""
END_OF_CENTOS_CFG
    if ($arch eq 'i386') {
       $cfg =~ s/exclude=/\#exclude=/o;
    }
    writeConfig($cfg);
}

sub fedora {
    my $mirror = "http://mirrors2.kernel.org/fedora";
    my $cfg = <<"END_OF_FEDORA_CFG";
config_opts['root'] = 'lintel-fedora-$version-$arch'
config_opts['target_arch'] = '$arch'
config_opts['chroot_setup_cmd'] = 'groupinstall buildsys-build'
# mock 1.0.3 has an error trying to set the ccache maximum cache size on Fedora 14
config_opts['plugin_conf']['ccache_enable'] = False

config_opts['yum.conf'] = """
[main]
cachedir=/var/cache/yum
debuglevel=1
reposdir=/dev/null
logfile=/var/log/yum.log
retries=20
obsoletes=1
gpgcheck=0
assumeyes=1
# grub/syslinux on x86_64 need glibc-devel.i386 which pulls in glibc.i386, need to exclude all
# .i?86 packages except these.
#exclude=[0-9A-Za-fh-z]*.i?86 g[0-9A-Za-km-z]*.i?86 gl[0-9A-Za-hj-z]*.i?86 gli[0-9A-Zac-z]*.i?86 glib[0-9A-Za-bd-z]*.i?86
# The above is not needed anymore with yum multilib policy of "best" which is the default in Fedora.

# repos

[fedora]
name=fedora
baseurl=$mirror/releases/$version/Everything/$arch/os
# mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-13&arch=x86_64
#failovermethod=priority

[updates-released]
name=updates
baseurl=$mirror/updates/$version/$arch
#mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=updates-released-f13&arch=x86_64
#failovermethod=priority

[local]
name=local
baseurl=http://kojipkgs.fedoraproject.org/repos/dist-f$version-build/latest/$arch/
cost=2000
enabled=0
"""
END_OF_FEDORA_CFG
    if ($arch eq 'i386') {
       $cfg =~ s/exclude=/\#exclude=/o;
    }
    writeConfig($cfg);
}

sub scilinux {
    my $base_version = $version;
    $base_version =~ s/\..*$//o;
    my $mirror = "http://mirrors.200p-sf.sonic.net/scientific";
    my $epel_mirror = "http://mirror.hmc.edu/epel";
    my $cfg = <<"END_OF_SCILINUX_CFG";
config_opts['root'] = 'lintel-scilinux-$version-$arch'
config_opts['target_arch'] = '$arch'
config_opts['chroot_setup_cmd'] = 'groupinstall buildsys-build'
# mock 1.0.3 has an error trying to set the ccache maximum cache size on Scientific Linux 6
config_opts['plugin_conf']['ccache_enable'] = False

config_opts['yum.conf'] = """
[main]
cachedir=/var/cache/yum
debuglevel=1
logfile=/var/log/yum.log
reposdir=/dev/null
retries=20
obsoletes=1
gpgcheck=0
assumeyes=1
# grub/syslinux on x86_64 need glibc-devel.i386 which pulls in glibc.i386, need to exclude all
# .i?86 packages except these.
# exclude=[1-9A-Za-fh-z]*.i?86 g[0-9A-Za-km-z]*.i?86 gl[0-9A-Za-hj-z]*.i?86 gli[0-9A-Zac-z]*.i?86 glib[0-9A-Za-bd-z]*.i?86
# repos

[core]
name=base
baseurl=$mirror/$version/$arch/os/

[updates-security]
name=updates-security
baseurl=$mirror/$version/$arch/updates/security

[updates-fastbugs]
name=updates-fastbugs
baseurl=$mirror/$version/$arch/updates/fastbugs

# [groups]
# name=groups
# baseurl=http://buildsys.fedoraproject.org/buildgroups/rhel$base_version/$arch/

[extras]
name=epel
baseurl=$epel_mirror/$base_version/$arch

[local]
name=local
baseurl=http://kojipkgs.fedoraproject.org/repos/dist-${base_version}E-epel-build/latest/$arch/
cost=2000
enabled=1

"""
END_OF_SCILINUX_CFG
#    if ($arch eq 'i386') {
#       $cfg =~ s/exclude=/\#exclude=/o;
#    }
    writeConfig($cfg);
}
