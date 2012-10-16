#!/usr/bin/perl
use 5.10.0;
use warnings FATAL => 'all';

say `llvm-config --libs`;
exit 0;

my @libs  = split /\s+/, `llvm-config --libfiles`;
my @flags = ();

for my $lib (@libs) {
    my ($flag,) = $lib =~ m{lib(\w+)\.(so|a)$};

    if (-e $lib) {
	push @flags, "-l$flag";
    }
}

@flags = ();

while (<DATA>) {
    for my $flag (split /\s+/, $_) {
	push @flags, $flag;
    }
}

say join(" ", @flags);

__DATA__
-lclangFrontend -lclangSerialization -lclangDriver -lclangCodeGen 
-lclangParse -lclangSema -lclangStaticAnalyzerFrontend 
-lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore 
-lclangAnalysis -lclangRewrite -lclangEdit -lclangAST 
-lclangLex -lclangBasic -lLLVMInstrumentation -lLLVMAsmParser 
-lLLVMLinker -lLLVMArchive -lLLVMipo -lLLVMVectorize -lLLVMBitWriter 
-lLLVMBitReader -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86Info 
-lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMSelectionDAG 
-lLLVMAsmPrinter -lLLVMMCParser -lLLVMInterpreter -lLLVMJIT 
-lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMCodeGen 
-lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils 
-lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMMC -lLLVMObject 
-lLLVMCore -lLLVMSupport   -lpthread -ldl -lm 

