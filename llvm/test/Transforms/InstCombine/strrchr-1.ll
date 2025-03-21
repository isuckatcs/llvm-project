; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; Test that the strrchr library call simplifier works correctly.
; RUN: opt < %s -passes=instcombine -S | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128"

@hello = constant [14 x i8] c"hello world\5Cn\00"
@null = constant [1 x i8] zeroinitializer
@chp = global ptr zeroinitializer

declare ptr @strrchr(ptr, i32)

define void @test_simplify1() {
; CHECK-LABEL: @test_simplify1(
; CHECK-NEXT:    store ptr getelementptr inbounds nuw (i8, ptr @hello, i32 6), ptr @chp, align 4
; CHECK-NEXT:    ret void
;

  %dst = call ptr @strrchr(ptr @hello, i32 119)
  store ptr %dst, ptr @chp
  ret void
}

define void @test_simplify2() {
; CHECK-LABEL: @test_simplify2(
; CHECK-NEXT:    store ptr null, ptr @chp, align 4
; CHECK-NEXT:    ret void
;

  %dst = call ptr @strrchr(ptr @null, i32 119)
  store ptr %dst, ptr @chp
  ret void
}

define void @test_simplify3() {
; CHECK-LABEL: @test_simplify3(
; CHECK-NEXT:    store ptr getelementptr inbounds nuw (i8, ptr @hello, i32 13), ptr @chp, align 4
; CHECK-NEXT:    ret void
;

  %dst = call ptr @strrchr(ptr @hello, i32 0)
  store ptr %dst, ptr @chp
  ret void
}

define void @test_simplify4() {
; CHECK-LABEL: @test_simplify4(
; CHECK-NEXT:    store ptr getelementptr inbounds nuw (i8, ptr @hello, i32 13), ptr @chp, align 4
; CHECK-NEXT:    ret void
;

  %dst = call ptr @strrchr(ptr @hello, i32 65280)
  store ptr %dst, ptr @chp
  ret void
}

define void @test_xform_to_memrchr(i32 %chr) {
; CHECK-LABEL: @test_xform_to_memrchr(
; CHECK-NEXT:    [[MEMRCHR:%.*]] = call ptr @memrchr(ptr noundef nonnull dereferenceable(14) @hello, i32 [[CHR:%.*]], i32 14)
; CHECK-NEXT:    store ptr [[MEMRCHR]], ptr @chp, align 4
; CHECK-NEXT:    ret void
;

  %dst = call ptr @strrchr(ptr @hello, i32 %chr)
  store ptr %dst, ptr @chp
  ret void
}

define ptr @test1(ptr %str, i32 %c) {
; CHECK-LABEL: @test1(
; CHECK-NEXT:    [[RET:%.*]] = call ptr @strrchr(ptr noundef nonnull dereferenceable(1) [[STR:%.*]], i32 [[C:%.*]])
; CHECK-NEXT:    ret ptr [[RET]]
;

  %ret = call ptr @strrchr(ptr %str, i32 %c)
  ret ptr %ret
}

define ptr @test2(ptr %str, i32 %c) null_pointer_is_valid {
; CHECK-LABEL: @test2(
; CHECK-NEXT:    [[RET:%.*]] = call ptr @strrchr(ptr noundef [[STR:%.*]], i32 [[C:%.*]])
; CHECK-NEXT:    ret ptr [[RET]]
;

  %ret = call ptr @strrchr(ptr %str, i32 %c)
  ret ptr %ret
}
