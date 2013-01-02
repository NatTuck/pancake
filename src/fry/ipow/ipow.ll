; ModuleID = 'ipow.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @ipow(i32 %xx, i32 %nn) nounwind uwtable {
entry:
  %xx.addr = alloca i32, align 4
  %nn.addr = alloca i32, align 4
  %yy = alloca i32, align 4
  %ii = alloca i32, align 4
  store i32 %xx, i32* %xx.addr, align 4
  store i32 %nn, i32* %nn.addr, align 4
  store i32 1, i32* %yy, align 4
  store i32 0, i32* %ii, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %ii, align 4
  %1 = load i32* %nn.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %yy, align 4
  %3 = load i32* %xx.addr, align 4
  %mul = mul nsw i32 %2, %3
  store i32 %mul, i32* %yy, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32* %ii, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %ii, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32* %yy, align 4
  ret i32 %5
}
