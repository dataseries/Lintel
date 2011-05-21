# TODO: I think there is a circular dependency somewhere between this code, the pbuilder code
# and the dataseries rules for rebuilding, perhaps as an interaction between the cache and
# the rebuilds needed to get the lintel repositories.  It's not fatal, but a little weird.

# You need a rule like
# /var/www/localpkgs/%/re-archive.dep: /var/www/localpkgs/%/lintel_$(VERSION).dsc
# 	touch $@
# to use this submake bit; see Lintel/debian/rules for an example of how to do this
# if 

DEBIAN_VERSIONS := lenny squeeze wheezy
PBUILDER_DEBIAN := $(foreach dist, $(DEBIAN_VERSIONS), \
	               $(foreach arch, amd64 i386, debian/stamp/pbuilder/debian-$(dist)-$(arch)))

# Ubuntu releases(EOL): dapper(2011-06) hardy (2013-04) karmic (2011-04) lucid (2015-04) 
#                       maverick(2012-04) natty (2012-10) oneiric (2011-10 .. 2013-04)
# Not going to support dapper, it's about to expire and it has a Release format old enough
# that lenny debootstrap doesn't recognize it.
UBUNTU_VERSIONS := hardy karmic lucid maverick natty
PBUILDER_UBUNTU := $(foreach dist, $(UBUNTU_VERSIONS), \
		       $(foreach arch, amd64 i386, debian/stamp/pbuilder/ubuntu-$(dist)-$(arch)))

.PRECIOUS: %.tgz %.gpg Release Sources Packages Release.config
.SECONDARY:

tars: tars-debian tars-ubuntu

tars-debian: $(foreach dist,$(DEBIAN_VERSIONS), \
	         $(foreach arch, amd64 i386, /var/cache/pbuilder/debian-$(dist)-$(arch).tgz))

tars-ubuntu: $(foreach dist,$(UBUNTU_VERSIONS), \
	         $(foreach arch, amd64 i386, /var/cache/pbuilder/ubuntu-$(dist)-$(arch).tgz))

pbuilder: pbuilder-debian pbuilder-ubuntu

pbuilder-debian: $(PBUILDER_DEBIAN)

pbuilder-ubuntu: $(PBUILDER_UBUNTU)

DMIRROR=http://mirrors2.kernel.org/debian
/var/cache/pbuilder/debian-%.tgz: /var/www/localpkgs/debian-%/Release.gpg
	touch /var/cache/pbuilder
	pbuilder --create --basetgz $@-new --distribution `echo $* | sed 's/-.*//'` \
	    --debootstrapopts --arch=`echo $* | sed 's/.*-//'` \
	    --othermirror "deb http://localhost/localpkgs/debian-$* ./" --mirror $(DMIRROR)
	mv $@-new $@

/tmp/ubuntu-%: /var/www/localpkgs/ubuntu-%/Release.gpg
	echo xit

UMIRROR=http://mirrors2.kernel.org/ubuntu/
/var/cache/pbuilder/ubuntu-%.tgz: /var/www/localpkgs/ubuntu-%/Release.gpg
	[ ! -z "$$http_proxy" ]
	touch /var/cache/pbuilder
	# cdebootstrap does not work for maverick
	pbuilder --create --basetgz $@-new --distribution `echo $* | sed 's/-.*//'` \
	    --debootstrapopts --arch=`echo $* | sed 's/.*-//'` --debootstrap debootstrap \
	    --othermirror "deb $(UMIRROR) `echo $* | sed 's/-.*//'` universe |deb http://localhost/localpkgs/ubuntu-$* ./" \
	    --mirror $(UMIRROR)
	mv $@-new $@

/var/www/localpkgs/%/Packages: /var/www/localpkgs/%/re-archive.dep
	[ -d /var/cache/pbuilder/result/$* ] || mkdir /var/cache/pbuilder/result/$*
	[ -L /var/www/localpkgs/$* ] || ln -s /var/cache/pbuilder/result/$* /var/www/localpkgs/$*
	cd /var/www/localpkgs/$* && apt-ftparchive packages . >Packages-new
	mv $@-new $@

/var/www/localpkgs/%/Sources: /var/www/localpkgs/%/re-archive.dep
	cd /var/www/localpkgs/$* && apt-ftparchive sources . >Sources-new
	mv $@-new $@

%.gz: %
	gzip -9v < $< >$@-new
	mv $@-new $@

/var/www/localpkgs/%/Release.config:
	./debian/localpkgs config >$@-new
	mv $@-new $@

/var/www/localpkgs/%/Release: /var/www/localpkgs/%/Packages.gz /var/www/localpkgs/%/Sources.gz /var/www/localpkgs/%/Release.config
	cd /var/www/localpkgs/$* && apt-ftparchive -c=Release.config release . >Release-new
	mv $@-new $@

# sudo HOME=/var/www/localpkgs gpg --gen-key
# Real name: Local packages signing key
# Email address: na@example.com
# Comment: local package signing key
# empty passphrase
/var/www/localpkgs/%/Release.gpg: /var/www/localpkgs/%/Release
	HOME=/var/www/localpkgs gpg --sign -ba -o $@-new $<
	mv $@-new $@

