#!/usr/bin/perl
use 5.12.0;
use warnings FATAL => 'all';

use JSON;
use File::Slurp;
use Data::Dumper;
use IO::Handle;

my ($spec_filename, $src_filename, $dst_filename) = @ARGV;

unless ($spec_filename and $src_filename and $dst_filename) {
    die "Usage:\n  $0 spec.json program.cl spec_out.cl\n";
}

sub read_kern {
    my ($src, $text) = @_;
    my ($obra, $cbra) = (0, 0);

    while (<$src>) {
        $text .= $_;
        $obra += y/{//;
        $cbra += y/}//;
        last if $obra > 0 && ($obra - $cbra) <= 0;
    }

    return $text;
}

sub spec_kern {
    my ($info, $text) = @_;
   
    say $text;

    # Find the function name.
    $text =~ /^\s*(.*?)\s*\(/s or die "bad kernel";
    my @ntoks  = split /\s+/, $1;
    my $k_name = $ntoks[-1];
    
    # Make sure it matches the spec info.
    return $text unless $info->{name} eq $k_name;

    # TODO: The rest of the thing.


    return $text;
}

my $info = decode_json(read_file($spec_filename));

open my $src, "<", $src_filename;
open my $dst, ">", $dst_filename;
while (<$src>) {
    if (/^\s*kernel/ or /^\s*__kernel/) {
        my $text = read_kern($src, $_);
        my $spec = spec_kern($info, $text);
        say "Spec text:\n$spec";
        $dst->print($spec);
    }
    else {
        $dst->print($_);
    }
}
close $dst;
close $src;


