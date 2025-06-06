// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --version 5
// RUN: %clang_cc1 -fsanitize=cfi-icall -fsanitize-trap=cfi-icall -flto -fvisibility=hidden -triple x86_64-unknown-linux -fwhole-program-vtables -emit-llvm -o - %s | FileCheck --check-prefix=TRAP %s

// RUN: %clang_cc1 -fsanitize=cfi-icall -fno-sanitize-trap=cfi-icall -flto -fvisibility=hidden -triple x86_64-unknown-linux -fwhole-program-vtables -emit-llvm -o - %s | FileCheck --check-prefix=ABORT %s

// RUN: %clang_cc1 -fsanitize=cfi-icall -fno-sanitize-trap=cfi-icall -fsanitize-recover=cfi-icall -flto -fvisibility=hidden -triple x86_64-unknown-linux -fwhole-program-vtables -emit-llvm -o - %s | FileCheck --check-prefix=RECOVER %s

// RUN: %clang_cc1 -fsanitize=cfi-icall -fno-sanitize-trap=cfi-icall -fsanitize-minimal-runtime -flto -fvisibility=hidden -triple x86_64-unknown-linux -fwhole-program-vtables -emit-llvm -o - %s | FileCheck --check-prefix=ABORT_MIN %s

// RUN: %clang_cc1 -fsanitize=cfi-icall -fno-sanitize-trap=cfi-icall -fsanitize-recover=cfi-icall -fsanitize-minimal-runtime -flto -fvisibility=hidden -triple x86_64-unknown-linux -fwhole-program-vtables -emit-llvm -o - %s | FileCheck --check-prefix=RECOVER_MIN %s

// TRAP-LABEL: define hidden void @f(
// TRAP-SAME: ) #[[ATTR0:[0-9]+]] !type [[META6:![0-9]+]] !type [[META7:![0-9]+]] {
// TRAP-NEXT:  [[ENTRY:.*:]]
// TRAP-NEXT:    ret void
//
// ABORT-LABEL: define hidden void @f(
// ABORT-SAME: ) #[[ATTR0:[0-9]+]] !type [[META6:![0-9]+]] !type [[META7:![0-9]+]] {
// ABORT-NEXT:  [[ENTRY:.*:]]
// ABORT-NEXT:    ret void
//
// RECOVER-LABEL: define hidden void @f(
// RECOVER-SAME: ) #[[ATTR0:[0-9]+]] !type [[META6:![0-9]+]] !type [[META7:![0-9]+]] {
// RECOVER-NEXT:  [[ENTRY:.*:]]
// RECOVER-NEXT:    ret void
//
// ABORT_MIN-LABEL: define hidden void @f(
// ABORT_MIN-SAME: ) #[[ATTR0:[0-9]+]] !type [[META6:![0-9]+]] !type [[META7:![0-9]+]] {
// ABORT_MIN-NEXT:  [[ENTRY:.*:]]
// ABORT_MIN-NEXT:    ret void
//
// RECOVER_MIN-LABEL: define hidden void @f(
// RECOVER_MIN-SAME: ) #[[ATTR0:[0-9]+]] !type [[META6:![0-9]+]] !type [[META7:![0-9]+]] {
// RECOVER_MIN-NEXT:  [[ENTRY:.*:]]
// RECOVER_MIN-NEXT:    ret void
//
void f() {
}

void xf();

// TRAP-LABEL: define hidden void @g(
// TRAP-SAME: i32 noundef [[B:%.*]]) #[[ATTR0]] !type [[META8:![0-9]+]] !type [[META9:![0-9]+]] {
// TRAP-NEXT:  [[ENTRY:.*:]]
// TRAP-NEXT:    [[B_ADDR:%.*]] = alloca i32, align 4
// TRAP-NEXT:    [[FP:%.*]] = alloca ptr, align 8
// TRAP-NEXT:    store i32 [[B]], ptr [[B_ADDR]], align 4
// TRAP-NEXT:    [[TMP0:%.*]] = load i32, ptr [[B_ADDR]], align 4
// TRAP-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[TMP0]], 0
// TRAP-NEXT:    [[TMP1:%.*]] = zext i1 [[TOBOOL]] to i64
// TRAP-NEXT:    [[COND:%.*]] = select i1 [[TOBOOL]], ptr @f, ptr @xf
// TRAP-NEXT:    store ptr [[COND]], ptr [[FP]], align 8
// TRAP-NEXT:    [[TMP2:%.*]] = load ptr, ptr [[FP]], align 8
// TRAP-NEXT:    [[TMP3:%.*]] = call i1 @llvm.type.test(ptr [[TMP2]], metadata !"_ZTSFvE"), !nosanitize [[META10:![0-9]+]]
// TRAP-NEXT:    br i1 [[TMP3]], label %[[CONT:.*]], label %[[TRAP:.*]], !prof [[PROF11:![0-9]+]], !nosanitize [[META10]]
// TRAP:       [[TRAP]]:
// TRAP-NEXT:    call void @llvm.ubsantrap(i8 2) #[[ATTR4:[0-9]+]], !nosanitize [[META10]]
// TRAP-NEXT:    unreachable, !nosanitize [[META10]]
// TRAP:       [[CONT]]:
// TRAP-NEXT:    call void (...) [[TMP2]]()
// TRAP-NEXT:    ret void
//
// ABORT-LABEL: define hidden void @g(
// ABORT-SAME: i32 noundef [[B:%.*]]) #[[ATTR0]] !type [[META8:![0-9]+]] !type [[META9:![0-9]+]] {
// ABORT-NEXT:  [[ENTRY:.*:]]
// ABORT-NEXT:    [[B_ADDR:%.*]] = alloca i32, align 4
// ABORT-NEXT:    [[FP:%.*]] = alloca ptr, align 8
// ABORT-NEXT:    store i32 [[B]], ptr [[B_ADDR]], align 4
// ABORT-NEXT:    [[TMP0:%.*]] = load i32, ptr [[B_ADDR]], align 4
// ABORT-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[TMP0]], 0
// ABORT-NEXT:    [[TMP1:%.*]] = zext i1 [[TOBOOL]] to i64
// ABORT-NEXT:    [[COND:%.*]] = select i1 [[TOBOOL]], ptr @f, ptr @xf
// ABORT-NEXT:    store ptr [[COND]], ptr [[FP]], align 8
// ABORT-NEXT:    [[TMP2:%.*]] = load ptr, ptr [[FP]], align 8
// ABORT-NEXT:    [[TMP3:%.*]] = call i1 @llvm.type.test(ptr [[TMP2]], metadata !"_ZTSFvE"), !nosanitize [[META10:![0-9]+]]
// ABORT-NEXT:    br i1 [[TMP3]], label %[[CONT:.*]], label %[[HANDLER_CFI_CHECK_FAIL:.*]], !prof [[PROF11:![0-9]+]], !nosanitize [[META10]]
// ABORT:       [[HANDLER_CFI_CHECK_FAIL]]:
// ABORT-NEXT:    [[TMP4:%.*]] = ptrtoint ptr [[TMP2]] to i64, !nosanitize [[META10]]
// ABORT-NEXT:    call void @__ubsan_handle_cfi_check_fail_abort(ptr @anon.3d4044d65abdda407a92991f1300ec97.1, i64 [[TMP4]], i64 undef) #[[ATTR4:[0-9]+]], !nosanitize [[META10]]
// ABORT-NEXT:    unreachable, !nosanitize [[META10]]
// ABORT:       [[CONT]]:
// ABORT-NEXT:    call void (...) [[TMP2]]()
// ABORT-NEXT:    ret void
//
// RECOVER-LABEL: define hidden void @g(
// RECOVER-SAME: i32 noundef [[B:%.*]]) #[[ATTR0]] !type [[META8:![0-9]+]] !type [[META9:![0-9]+]] {
// RECOVER-NEXT:  [[ENTRY:.*:]]
// RECOVER-NEXT:    [[B_ADDR:%.*]] = alloca i32, align 4
// RECOVER-NEXT:    [[FP:%.*]] = alloca ptr, align 8
// RECOVER-NEXT:    store i32 [[B]], ptr [[B_ADDR]], align 4
// RECOVER-NEXT:    [[TMP0:%.*]] = load i32, ptr [[B_ADDR]], align 4
// RECOVER-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[TMP0]], 0
// RECOVER-NEXT:    [[TMP1:%.*]] = zext i1 [[TOBOOL]] to i64
// RECOVER-NEXT:    [[COND:%.*]] = select i1 [[TOBOOL]], ptr @f, ptr @xf
// RECOVER-NEXT:    store ptr [[COND]], ptr [[FP]], align 8
// RECOVER-NEXT:    [[TMP2:%.*]] = load ptr, ptr [[FP]], align 8
// RECOVER-NEXT:    [[TMP3:%.*]] = call i1 @llvm.type.test(ptr [[TMP2]], metadata !"_ZTSFvE"), !nosanitize [[META10:![0-9]+]]
// RECOVER-NEXT:    br i1 [[TMP3]], label %[[CONT:.*]], label %[[HANDLER_CFI_CHECK_FAIL:.*]], !prof [[PROF11:![0-9]+]], !nosanitize [[META10]]
// RECOVER:       [[HANDLER_CFI_CHECK_FAIL]]:
// RECOVER-NEXT:    [[TMP4:%.*]] = ptrtoint ptr [[TMP2]] to i64, !nosanitize [[META10]]
// RECOVER-NEXT:    call void @__ubsan_handle_cfi_check_fail(ptr @anon.3d4044d65abdda407a92991f1300ec97.1, i64 [[TMP4]], i64 undef) #[[ATTR4:[0-9]+]], !nosanitize [[META10]]
// RECOVER-NEXT:    br label %[[CONT]], !nosanitize [[META10]]
// RECOVER:       [[CONT]]:
// RECOVER-NEXT:    call void (...) [[TMP2]]()
// RECOVER-NEXT:    ret void
//
// ABORT_MIN-LABEL: define hidden void @g(
// ABORT_MIN-SAME: i32 noundef [[B:%.*]]) #[[ATTR0]] !type [[META8:![0-9]+]] !type [[META9:![0-9]+]] {
// ABORT_MIN-NEXT:  [[ENTRY:.*:]]
// ABORT_MIN-NEXT:    [[B_ADDR:%.*]] = alloca i32, align 4
// ABORT_MIN-NEXT:    [[FP:%.*]] = alloca ptr, align 8
// ABORT_MIN-NEXT:    store i32 [[B]], ptr [[B_ADDR]], align 4
// ABORT_MIN-NEXT:    [[TMP0:%.*]] = load i32, ptr [[B_ADDR]], align 4
// ABORT_MIN-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[TMP0]], 0
// ABORT_MIN-NEXT:    [[TMP1:%.*]] = zext i1 [[TOBOOL]] to i64
// ABORT_MIN-NEXT:    [[COND:%.*]] = select i1 [[TOBOOL]], ptr @f, ptr @xf
// ABORT_MIN-NEXT:    store ptr [[COND]], ptr [[FP]], align 8
// ABORT_MIN-NEXT:    [[TMP2:%.*]] = load ptr, ptr [[FP]], align 8
// ABORT_MIN-NEXT:    [[TMP3:%.*]] = call i1 @llvm.type.test(ptr [[TMP2]], metadata !"_ZTSFvE"), !nosanitize [[META10:![0-9]+]]
// ABORT_MIN-NEXT:    br i1 [[TMP3]], label %[[CONT:.*]], label %[[HANDLER_CFI_CHECK_FAIL:.*]], !prof [[PROF11:![0-9]+]], !nosanitize [[META10]]
// ABORT_MIN:       [[HANDLER_CFI_CHECK_FAIL]]:
// ABORT_MIN-NEXT:    call void @__ubsan_handle_cfi_check_fail_minimal_abort() #[[ATTR4:[0-9]+]], !nosanitize [[META10]]
// ABORT_MIN-NEXT:    unreachable, !nosanitize [[META10]]
// ABORT_MIN:       [[CONT]]:
// ABORT_MIN-NEXT:    call void (...) [[TMP2]]()
// ABORT_MIN-NEXT:    ret void
//
// RECOVER_MIN-LABEL: define hidden void @g(
// RECOVER_MIN-SAME: i32 noundef [[B:%.*]]) #[[ATTR0]] !type [[META8:![0-9]+]] !type [[META9:![0-9]+]] {
// RECOVER_MIN-NEXT:  [[ENTRY:.*:]]
// RECOVER_MIN-NEXT:    [[B_ADDR:%.*]] = alloca i32, align 4
// RECOVER_MIN-NEXT:    [[FP:%.*]] = alloca ptr, align 8
// RECOVER_MIN-NEXT:    store i32 [[B]], ptr [[B_ADDR]], align 4
// RECOVER_MIN-NEXT:    [[TMP0:%.*]] = load i32, ptr [[B_ADDR]], align 4
// RECOVER_MIN-NEXT:    [[TOBOOL:%.*]] = icmp ne i32 [[TMP0]], 0
// RECOVER_MIN-NEXT:    [[TMP1:%.*]] = zext i1 [[TOBOOL]] to i64
// RECOVER_MIN-NEXT:    [[COND:%.*]] = select i1 [[TOBOOL]], ptr @f, ptr @xf
// RECOVER_MIN-NEXT:    store ptr [[COND]], ptr [[FP]], align 8
// RECOVER_MIN-NEXT:    [[TMP2:%.*]] = load ptr, ptr [[FP]], align 8
// RECOVER_MIN-NEXT:    [[TMP3:%.*]] = call i1 @llvm.type.test(ptr [[TMP2]], metadata !"_ZTSFvE"), !nosanitize [[META10:![0-9]+]]
// RECOVER_MIN-NEXT:    br i1 [[TMP3]], label %[[CONT:.*]], label %[[HANDLER_CFI_CHECK_FAIL:.*]], !prof [[PROF11:![0-9]+]], !nosanitize [[META10]]
// RECOVER_MIN:       [[HANDLER_CFI_CHECK_FAIL]]:
// RECOVER_MIN-NEXT:    call void @__ubsan_handle_cfi_check_fail_minimal() #[[ATTR4:[0-9]+]], !nosanitize [[META10]]
// RECOVER_MIN-NEXT:    br label %[[CONT]], !nosanitize [[META10]]
// RECOVER_MIN:       [[CONT]]:
// RECOVER_MIN-NEXT:    call void (...) [[TMP2]]()
// RECOVER_MIN-NEXT:    ret void
//
void g(int b) {
  void (*fp)() = b ? f : xf;
  fp();
}
//.
// TRAP: [[META6]] = !{i64 0, !"_ZTSFvE"}
// TRAP: [[META7]] = !{i64 0, !"_ZTSFvE.generalized"}
// TRAP: [[META8]] = !{i64 0, !"_ZTSFviE"}
// TRAP: [[META9]] = !{i64 0, !"_ZTSFviE.generalized"}
// TRAP: [[META10]] = !{}
// TRAP: [[PROF11]] = !{!"branch_weights", i32 1048575, i32 1}
//.
// ABORT: [[META6]] = !{i64 0, !"_ZTSFvE"}
// ABORT: [[META7]] = !{i64 0, !"_ZTSFvE.generalized"}
// ABORT: [[META8]] = !{i64 0, !"_ZTSFviE"}
// ABORT: [[META9]] = !{i64 0, !"_ZTSFviE.generalized"}
// ABORT: [[META10]] = !{}
// ABORT: [[PROF11]] = !{!"branch_weights", i32 1048575, i32 1}
//.
// RECOVER: [[META6]] = !{i64 0, !"_ZTSFvE"}
// RECOVER: [[META7]] = !{i64 0, !"_ZTSFvE.generalized"}
// RECOVER: [[META8]] = !{i64 0, !"_ZTSFviE"}
// RECOVER: [[META9]] = !{i64 0, !"_ZTSFviE.generalized"}
// RECOVER: [[META10]] = !{}
// RECOVER: [[PROF11]] = !{!"branch_weights", i32 1048575, i32 1}
//.
// ABORT_MIN: [[META6]] = !{i64 0, !"_ZTSFvE"}
// ABORT_MIN: [[META7]] = !{i64 0, !"_ZTSFvE.generalized"}
// ABORT_MIN: [[META8]] = !{i64 0, !"_ZTSFviE"}
// ABORT_MIN: [[META9]] = !{i64 0, !"_ZTSFviE.generalized"}
// ABORT_MIN: [[META10]] = !{}
// ABORT_MIN: [[PROF11]] = !{!"branch_weights", i32 1048575, i32 1}
//.
// RECOVER_MIN: [[META6]] = !{i64 0, !"_ZTSFvE"}
// RECOVER_MIN: [[META7]] = !{i64 0, !"_ZTSFvE.generalized"}
// RECOVER_MIN: [[META8]] = !{i64 0, !"_ZTSFviE"}
// RECOVER_MIN: [[META9]] = !{i64 0, !"_ZTSFviE.generalized"}
// RECOVER_MIN: [[META10]] = !{}
// RECOVER_MIN: [[PROF11]] = !{!"branch_weights", i32 1048575, i32 1}
//.
