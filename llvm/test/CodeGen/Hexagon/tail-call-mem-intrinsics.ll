; RUN: llc -mtriple=hexagon < %s | FileCheck %s

; CHECK-LABEL: tail_memcpy:
; CHECK: jump memcpy
define void @tail_memcpy(ptr nocapture %p, ptr nocapture readonly %q, i32 %n) #0 {
entry:
  tail call void @llvm.memcpy.p0.p0.i32(ptr %p, ptr %q, i32 %n, i1 false)
  ret void
}

; CHECK-LABEL: tail_memmove:
; CHECK: jump memmove
define void @tail_memmove(ptr nocapture %p, ptr nocapture readonly %q, i32 %n) #0 {
entry:
  tail call void @llvm.memmove.p0.p0.i32(ptr %p, ptr %q, i32 %n, i1 false)
  ret void
}

; CHECK-LABEL: tail_memset:
; CHECK: jump memset
define void @tail_memset(ptr nocapture %p, i8 %c, i32 %n) #0 {
entry:
  tail call void @llvm.memset.p0.i32(ptr %p, i8 %c, i32 %n, i1 false)
  ret void
}

declare void @llvm.memcpy.p0.p0.i32(ptr nocapture, ptr nocapture readonly, i32, i1) #0
declare void @llvm.memmove.p0.p0.i32(ptr nocapture, ptr nocapture readonly, i32, i1) #0
declare void @llvm.memset.p0.i32(ptr nocapture, i8, i32, i1) #0

attributes #0 = { nounwind }
