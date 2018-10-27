#!/usr/bin/perl
# TODO Improve this haphazardly test file. Parts are taken from
# http://learn.perl.org/examples/directory_list.html
# http://modernperlbooks.com/books/modern_perl_2016/03-perl-language.html

use warnings;
use v1.3.4;
use strict   qw(subs vars refs);
use if $] < 5.008, "utf8";
use if WANT_WARNINGS, warnings => qw(all);
use Path::Tiny;
use Net::DNS::Resolver;

my $dir = path('foo','bar'); # foo/bar

# Iterate over the content of foo/bar
my $iter = $dir->iterator;
while (my $file = $iter->()) {

    # Don't show local variable as global
    my $dontShowThisVar;
    # See if it is a directory and skip
    next if $file->is_dir();

    # Print out the file name and path
    print "$file\n";
}

my $name;
my @_private_names;
my %Names_to_Addresses;
sub anAwkwardName3;

# with use utf8; enabled
package Ingy::Döt::Net;

# Variable Names and Sigils

my $scalar;
my @array;
my %hash;
# FIXME Not well handled
my ($bad_name, @bad_name, %bad_name);

$hash{ $key }     = 'value';
$array[ $index ]  = 'item';

my @hash_elements  = @hash{ @keys };
my @array_elements = @array[ @indexes ];

# TODO package as...what?..uses?
# Variable Scopes
package Store::Toy;
my $discount = 0.10;
package Store::Music;

# $discount still visible
say "Our current discount is $discount!";

package Store::Toy {
	my $discount = 0.10;
}

   my $name    = 'Donner Odinson, Bringer of Despair';
    my $address = "Room 539, Bilskirnir, Valhalla";
    my $reminder = 'Don\'t forget to escape '
                 . 'the single quote!';

      my $quote     = qq{"Ouch", he said. "That hurt!"};
    my $reminder  =  q^Don't escape the single quote!^;
    my $complaint =  q{It's too early to be awake.};


# Use the heredoc syntax to assign multiple lines to a string:
my $blurb = <<'END_BLURB';

sub DontShowThis1 {
	my $DontShowThis1Arg = shift;
	...
}
He looked up. "Change is the constant on which they all
can agree.  We instead, born out of time, remain perfect
and perfectly self-aware. We only suffer change as we
pursue it. It is against our nature. We rebel against
that change. Shall we consider them greater for it?"
END_BLURB

# Special Perl documentation format
=item DontShowStuff()
    This function does stuff.
    sub DontShowStuff {
        ...
    }
=cut

print<<'END_HERE';
One more time a heredoc test
    sub DontShowThis2 {
        my $DontShowThis2Arg = shift;
        ...
    }
END_HEREfoo

# Nested loops can be confusing, especially with loop control statements.
# If you cannot extract inner loops into named functions,
# use loop labels to clarify your intent:
    LINE:
    while (<$fh>) {
        chomp;

        PREFIX:
        for my $prefix (@prefixes) {
            next LINE unless $prefix;
            say "$prefix: $_";
            # next PREFIX is implicit here
        }
    }

sub foo {
    my $arg = shift;
    ...
}

sub foo_explicit_args {
    my $arg = shift @_;
    ...
}


# The following snippets are taken from
# https://perldoc.perl.org/perlop.html
# and is not yet properly handled

# These three are so far handled...
q{foo{bar}baz}
'foo{bar}baz'
q#foo#
# FIXME...this one not
q XfooX # means the string 'foo'
qXfooX  # WRONG syntax!


 (my $quote = <<'FINIS') =~ s/^\s+//gm;
       The Road goes ever on and on,
    my $DontShowThis4
       down from the door where it began.
    FINIS

# It is possible to stack multiple here-docs in a row
print <<"foo", <<"bar"; # you can stack them
    I said foo.
    my $DontShowThis4
foo
    I said bar.
    my $DontShowThis5
bar

myfunc(<< "THIS", 23, <<'THAT');
    Here's a line
    my $DontShowThis6
    or two.
THIS
    and here's another.
    my $DontShowThis7
THAT

# As of this writing (Kate Version 18.11.70) the highliting has trouble with this too

      if ($some_var) {
      print <<~EOF;
        This is a here-doc
        my $DontShowThis8
        EOF
    }

# FIXME That odd token is not well recognised
 print << "abc\"def";
    testing...
    abc"def

# Don't treat this as heredoc
print 20 << 20;  # 20971520

# kate: space-indent on; indent-width 4; replace-tabs on;
