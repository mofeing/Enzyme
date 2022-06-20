; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --include-generated-funcs
; RUN: %opt < %s %loadEnzyme -enzyme -enzyme-preopt=false -enzyme-vectorize-at-leaf-nodes -inline -mem2reg -instsimplify -gvn -dse -dse -S | FileCheck %s

define void @addOneMem(double* nocapture %x) {
entry:
  %0 = load double, double* %x, align 8
  %add = fadd double %0, 1.000000e+00
  store double %add, double* %x, align 8
  ret void
}

define void @test_derivative(double* %x, <3 x double>* %xp) {
entry:
  call void (void (double*)*, ...) @__enzyme_fwddiff(void (double*)* nonnull @addOneMem, metadata !"enzyme_width", i64 3, double* %x, <3 x double>* %xp)
  ret void
}

declare void @__enzyme_fwddiff(void (double*)*, ...)


; CHECK: define void @test_derivative(double* %x, <3 x double>* %xp)
; CHECK-NEXT: entry:
; CHECK-NEXT:   %"'ipl.i" = load <3 x double>, <3 x double>* %xp, align 8
; CHECK-NEXT:   %0 = load double, double* %x, align 8
; CHECK-NEXT:   %add.i = fadd double %0, 1.000000e+00
; CHECK-NEXT:   store double %add.i, double* %x, align 8
; CHECK-NEXT:   store <3 x double> %"'ipl.i", <3 x double>* %xp, align 8
; CHECK-NEXT:   ret void
; CHECK-NEXT: }