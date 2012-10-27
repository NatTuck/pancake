#!/usr/bin/perl
use 5.10.0;
use warnings FATAL => 'all';

my @libs  = split /\s+/, `llvm-config --libfiles`;
my @flags = ();

for my $lib (@libs) {
    my ($flag,) = $lib =~ m{lib(\w+)\.(so|a)$};

    if (-e $lib) {
	push @flags, "-l$flag";
    }
}

say join(" ", @flags);

