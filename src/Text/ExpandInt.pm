package Text::ExpandInt;

use Text::Expand;

require Exporter;

@ISA = qw/Exporter/;
@EXPORT_OK = qw/expand/;

warn "Text::ExpandInt::expand is obsolete and should be replaced with 
Text::Expand::expandString.  This shim will be removed after 2008-06-01";

sub expand {
    return Text::Expand::expandString(@_);
}

1;
