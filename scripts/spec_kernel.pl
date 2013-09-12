#!/usr/bin/perl
use 5.12.0;
use warnings FATAL => 'all';

use JSON;
use File::Slurp;

my ($program_filename, $spec_filename) = @ARGV;
unless ($program_filename and $spec_filename) {
    die "Usage:\n  $0 program.cl spec.json\n";
}

my $info = decode_json(read_file($spec_filename));

sub spec_program {
    my ($inp, $out) = @_;

    while (<$inp>) {
        if (/kernel/ or /__kernel/) {
            
        }
        else {
            $out->print($_);
        }
    }
}




open my $inp, "<", $program_filename;
open my $out, ">", $prog
