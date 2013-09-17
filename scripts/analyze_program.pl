#!/usr/bin/perl
use 5.12.0;
use warnings FATAL => 'all';

use JSON;

my ($filename) = @ARGV;
die "Usage:\n  $0 program.cl\n" unless $filename;

sub read_decl {
    my ($pgm, $decl) = @_;

    while (<$pgm>) {
        if (/^(.*)?{/) {
            $decl .= $1;
            last;
        }
        else {
            $decl .= $_;
        }
    }

    $decl =~ s/\n/ /g;
    return $decl;
}

sub parse_decl {
    my ($text) = @_;
    my $decl = {};

    $text =~ s/^\s*void\s*//;

    $text =~ /^(\w+)\s*\((.*?)\)/ or do die "Not a decl\n$text";

    my $name = $1;
    my @args = split /\s*,\s*/, $2;

    my $spec = {};

    if ($text =~ /\@spec\s*$name\((.*?)\)/) {
        my @spec_args = split /\s*,\s*/, $1;
        for my $sa (@spec_args) {
            $spec->{$sa} = 1;
        }
    }

    $decl->{name} = $name;
    $decl->{args} = [];

    for my $arg (@args) {
        $arg =~ /^(.*?)\s*(\w+)$/;
        my ($a_type, $a_name) = ($1, $2);
        my $a_spec = defined $spec->{$a_name} ? \1 : \0;

        # Constness doesn't matter for arg types.
        $a_type =~ s/\s*const\s*//g;

        push @{$decl->{args}}, { 
            name => $a_name, 
            type => $a_type,
            spec => $a_spec,
        };
    }

    return $decl;
}

my $info = [];

open my $pgm, "<", $filename;
while (<$pgm>) {
    if (/^kernel\s*(.*)/ or /^__kernel\s*(.*)/) {
        my $text = read_decl($pgm, "$1\n");
        my $decl = parse_decl($text);
        push @$info, $decl;
    }
}
close $pgm;
        
say encode_json($info);
