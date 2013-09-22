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

sub extract_header {
    my ($text) = @_;
    
    my $parens = 1;
    my $ii = index($text, '(') + 1;
    for (; $parens > 0; ++$ii) {
        die "can't extract header:\n $text\n" if ($ii + 1 > length($text));
        ++$parens if substr($text, $ii, 1) eq '(';
        --$parens if substr($text, $ii, 1) eq ')';
    }

    return substr($text, 0, $ii);
}

sub unroll_pragmas {
    my ($info, $text0) = @_;
    my ($text1, @loops) = split(/\bfor\b/, $text0);

    for my $loop (@loops) {
        my $hdr    = extract_header($loop);
        my $unroll = "";

        my (undef, $cond, undef) = split(';', $hdr);
        for my $arg (@{$info->{args}}) {
            next unless $arg->{spec};
            my $name = $arg->{name};
            if ($cond =~ /$name/) {
                my $value = $arg->{value};
                $unroll = ""  if ($value % 4 == 0);
                $unroll = "" if ($value < 32);
            }
        }

        $text1 .= "\n#pragma unroll $unroll\n";
        $text1 .= "for $loop";
    }


    return $text1;
}


sub spec_kern {
    my ($info, $text) = @_;
   
    # Find the function name.
    $text =~ /^\s*(.*?)\s*\(/s or die "bad kernel";
    my @ntoks  = split /\s+/, $1;
    my $k_name = $ntoks[-1];
   
    # Make sure it matches the spec info.
    return $text unless $info->{name} eq $k_name;

    unless (defined $ENV{PANCAKE_NOSPEC}) {
        # Rename the specialized arguments.
        $text =~ /^(([^\(]*?)$k_name\s*\((.*?)\))/s;
        my $full_head = $1;
        my $head_dcls = $2;
        my @args = split /\s*,\s*/, $3;
        
        my $nargs = scalar @args;
        for (my $ii = 0; $ii < $nargs; ++$ii) {
            my $a_info = $info->{args}[$ii];
            if ($a_info->{spec}) {
                $args[$ii] =~ s/\s*$//s;
                $args[$ii] .= "___";
            }
        }

        my $spec_head = "$head_dcls $k_name(" . join(', ', @args) . ")";
        substr $text, 0, length($full_head), $spec_head;

        # Insert specialized values as locals.
        my $spec_locals = "";
        for my $ai (@{$info->{args}}) {
            if ($ai->{spec}) {
                $spec_locals .= sprintf("    const %s %s = %s;\n",
                    $ai->{type}, $ai->{name}, $ai->{value});
            }
        }
        substr $text, index($text, '{') + 1, 0, "\n$spec_locals";
    }

    # Neuter comments
    $text =~ s/\/\/.*?\n//sg;
    $text =~ s/\/\*.*?\*\///sg;

    if (defined $ENV{PANCAKE_UNROLL}) {
        # Insert unroll pragmas on appropriate for loops
        $text = unroll_pragmas($info, $text);
    }
        
    say "Spec text:\n$text";

    return $text;
}

my $info = decode_json(read_file($spec_filename));

open my $src, "<", $src_filename;
open my $dst, ">", $dst_filename;
while (<$src>) {
    if (/^\s*kernel/ or /^\s*__kernel/) {
        my $text = read_kern($src, $_);
        my $spec = spec_kern($info, $text);
        $dst->print($spec);
    }
    else {
        $dst->print($_);
    }
}
close $dst;
close $src;


