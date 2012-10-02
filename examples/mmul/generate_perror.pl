#!/usr/bin/perl
use warnings FATAL => 'all';
use strict;
use 5.10.0;

use IO::Handle;

my $CL_H = "/usr/include/CL/cl.h";

unless (-e $CL_H) {
    say "Requires OpenCL Headers";
    say;
    say "  sudo apt-get install opencl-headers";
    say;
    exit(1);
}

my %CODES = ();

open my $clh, "<", $CL_H;
while (<$clh>) {
    next unless /^#define\s+(CL_\w+)\s+(-?\d+)/;
    my ($code, $symbol) = ($2, $1);
    next if ($code > 0);

    $CODES{$code} = $symbol;
}
close $clh;

# Find minimum error code.
my $top = 0;
for my $key (keys %CODES) {
    $top = $key if $key < $top;
}
$top = -$top;

open my $clc, ">", "pclu_perror.c";
$clc->print(<<"END_CODE");

/*
 * This file is generated automatically.
 * Any changes you make will be lost.
 *
 */

#include "pclu_perror.h"

const char* PCLU_ERROR_SYMBOLS[] = {
END_CODE

for (my $ii = 0; $ii <= $top; ++$ii) {
    my $symbol = $CODES{-$ii} || "UNKNOWN_ERROR_CODE";
    $clc->print(qq{    "$symbol",\n});
}

$clc->print(<<"END_CODE");
    "UNKNOWN_ERROR_CODE"
};

const char*
pclu_strerror(int code) 
{
    int idx = abs(code);
    if (idx > $top)
        idx = ($top + 1);
    return PCLU_ERROR_SYMBOLS[idx];
}

void
pclu_perror(int code)
{
    fprintf(stderr, "OpenCL Error: %s\\n", pclu_strerror(code));
}
END_CODE
close $clc;

open my $pclu_h, ">", "pclu_perror.h";
$pclu_h->print(<<"HEADER");
#ifndef PCLU_PERROR_H
#define PCLU_PERROR_H

#include <stdlib.h>
#include <stdio.h>

// This file is generated automatically.
// Any changes you make will be lost.

const char* pclu_strerror(int code);
void pclu_perror(int code);

#endif
HEADER
close $pclu_h;
