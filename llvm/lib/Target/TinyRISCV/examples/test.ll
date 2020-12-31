; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
target triple = "tinyriscv"

; Function Attrs: noinline nounwind optnone
define dso_local i32 @main(i32 %0, i8** %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  store i8** %1, i8*** %5, align 4
  store i32 7, i32* %6, align 4
  %8 = load i32, i32* %6, align 4
  %9 = load i32, i32* %4, align 4
  %10 = mul nsw i32 %8, %9
  store i32 %10, i32* %7, align 4
  %11 = load i32, i32* %7, align 4
  %12 = add nsw i32 %11, 1
  store i32 %12, i32* %7, align 4
  %13 = load i32, i32* %7, align 4
  ret i32 %13
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"ilp32"}
!2 = !{!"clang version 10.0.0-4ubuntu1 "}
