# Compatibility package that has to last until all the OS's we release on have Digest::SHA.
# Debian Squeeze does, Wheezy dropped Digest::SHA1.
# Ubuntu Hardy Heron does not have Digest::SHA as an available package.
package Lintel::SHA1;

use Carp 'confess';

our $impl_package;

eval 'use Digest::SHA';

if (!$@) {
    $impl_package = 'Digest::SHA';
} else {
    eval 'use Digest::SHA1';
    if (!$@) {
        $impl_package = 'Digest::SHA1';
    } else {
        confess "Unable to find either Digest::SHA or Digest::SHA1";
    }
}

1;


