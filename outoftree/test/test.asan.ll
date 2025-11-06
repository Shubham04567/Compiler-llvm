; ModuleID = 'test/test.c'
source_filename = "test/test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

$asan.module_ctor = comdat any

$asan.module_dtor = comdat any

$.str.21576b91ab9b15712202e1b4a494877f = comdat any

$.str.1.21576b91ab9b15712202e1b4a494877f = comdat any

@.str = internal constant { [24 x i8], [40 x i8] } { [24 x i8] c"Succesfully executed:1\0A\00", [40 x i8] zeroinitializer }, comdat($.str.21576b91ab9b15712202e1b4a494877f), align 32, !dbg !0
@.str.1 = internal constant { [24 x i8], [40 x i8] } { [24 x i8] c"Succesfully executed:2\0A\00", [40 x i8] zeroinitializer }, comdat($.str.1.21576b91ab9b15712202e1b4a494877f), align 32, !dbg !7
@__asan_option_detect_stack_use_after_return = external global i32
@___asan_gen_ = private unnamed_addr constant [16 x i8] c"1 32 40 5 arr:4\00", align 1
@___asan_gen_.2 = private constant [12 x i8] c"test/test.c\00", align 1
@___asan_gen_.3 = private unnamed_addr constant [5 x i8] c".str\00", align 1
@___asan_gen_.4 = private unnamed_addr constant [7 x i8] c".str.1\00", align 1
@__asan_global_.str = private global { i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @0 to i64), i64 24, i64 64, i64 ptrtoint (ptr @___asan_gen_.3 to i64), i64 ptrtoint (ptr @___asan_gen_.2 to i64), i64 0, i64 0, i64 -1 }, section "asan_globals", comdat($.str.21576b91ab9b15712202e1b4a494877f), !associated !9
@__asan_global_.str.1 = private global { i64, i64, i64, i64, i64, i64, i64, i64 } { i64 ptrtoint (ptr @1 to i64), i64 24, i64 64, i64 ptrtoint (ptr @___asan_gen_.4 to i64), i64 ptrtoint (ptr @___asan_gen_.2 to i64), i64 0, i64 0, i64 -1 }, section "asan_globals", comdat($.str.1.21576b91ab9b15712202e1b4a494877f), !associated !10
@llvm.compiler.used = appending global [4 x ptr] [ptr @.str, ptr @.str.1, ptr @__asan_global_.str, ptr @__asan_global_.str.1], section "llvm.metadata"
@___asan_globals_registered = common hidden global i64 0
@__start_asan_globals = extern_weak hidden global i64
@__stop_asan_globals = extern_weak hidden global i64
@llvm.used = appending global [2 x ptr] [ptr @asan.module_ctor, ptr @asan.module_dtor], section "llvm.metadata"
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 1, ptr @asan.module_ctor, ptr @asan.module_ctor }]
@llvm.global_dtors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 1, ptr @asan.module_dtor, ptr @asan.module_dtor }]

@0 = private alias { [24 x i8], [40 x i8] }, ptr @.str
@1 = private alias { [24 x i8], [40 x i8] }, ptr @.str.1

; Function Attrs: noinline nounwind optnone sanitize_address uwtable
define dso_local i32 @main() #0 !dbg !21 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca ptr, align 8
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  %ptr2 = alloca ptr, align 8
  %asan_local_stack_base = alloca i64, align 8
  %0 = load i32, ptr @__asan_option_detect_stack_use_after_return, align 4
  %1 = icmp ne i32 %0, 0
  br i1 %1, label %2, label %4

2:                                                ; preds = %entry
  %3 = call i64 @__asan_stack_malloc_1(i64 128)
  br label %4

4:                                                ; preds = %entry, %2
  %5 = phi i64 [ 0, %entry ], [ %3, %2 ]
  %6 = icmp eq i64 %5, 0
  br i1 %6, label %7, label %9

7:                                                ; preds = %4
  %MyAlloca = alloca i8, i64 128, align 32
  %8 = ptrtoint ptr %MyAlloca to i64
  br label %9

9:                                                ; preds = %4, %7
  %10 = phi i64 [ %5, %4 ], [ %8, %7 ]
  store i64 %10, ptr %asan_local_stack_base, align 8
  %11 = add i64 %10, 32
  %12 = inttoptr i64 %11 to ptr
  %13 = inttoptr i64 %10 to ptr
  store i64 1102416563, ptr %13, align 8
  %14 = add i64 %10, 8
  %15 = inttoptr i64 %14 to ptr
  store i64 ptrtoint (ptr @___asan_gen_ to i64), ptr %15, align 8
  %16 = add i64 %10, 16
  %17 = inttoptr i64 %16 to ptr
  store i64 ptrtoint (ptr @main to i64), ptr %17, align 8
  %18 = lshr i64 %10, 3
  %19 = add i64 %18, 2147450880
  %20 = add i64 %19, 0
  %21 = inttoptr i64 %20 to ptr
  store i64 -506381209984437775, ptr %21, align 1
  %22 = add i64 %19, 8
  %23 = inttoptr i64 %22 to ptr
  store i64 -868082074056920072, ptr %23, align 1
  store i32 0, ptr %retval, align 4
  %24 = add i64 %19, 4, !dbg !26
  %25 = inttoptr i64 %24 to ptr, !dbg !26
  store i32 0, ptr %25, align 1, !dbg !26
  %26 = add i64 %19, 8, !dbg !26
  %27 = inttoptr i64 %26 to ptr, !dbg !26
  store i8 0, ptr %27, align 1, !dbg !26
  call void @llvm.lifetime.start.p0(i64 40, ptr %12) #5, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %asan_local_stack_base, metadata !27, metadata !DIExpression(DW_OP_deref, DW_OP_plus_uconst, 32)), !dbg !31
  call void @llvm.lifetime.start.p0(i64 8, ptr %ptr) #5, !dbg !32
  call void @llvm.dbg.declare(metadata ptr %ptr, metadata !33, metadata !DIExpression()), !dbg !35
  %arrayidx = getelementptr inbounds [10 x i32], ptr %12, i64 0, i64 10, !dbg !36
  store ptr %arrayidx, ptr %ptr, align 8, !dbg !35
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #5, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %a, metadata !38, metadata !DIExpression()), !dbg !39
  store i32 10, ptr %a, align 4, !dbg !39
  call void @llvm.lifetime.start.p0(i64 4, ptr %b) #5, !dbg !40
  call void @llvm.dbg.declare(metadata ptr %b, metadata !41, metadata !DIExpression()), !dbg !42
  store i32 100, ptr %b, align 4, !dbg !42
  call void @llvm.lifetime.start.p0(i64 4, ptr %c) #5, !dbg !43
  call void @llvm.dbg.declare(metadata ptr %c, metadata !44, metadata !DIExpression()), !dbg !45
  %28 = load i32, ptr %a, align 4, !dbg !46
  %29 = load i32, ptr %b, align 4, !dbg !47
  %add = add nsw i32 %28, %29, !dbg !48
  store i32 %add, ptr %c, align 4, !dbg !45
  %30 = load i32, ptr %c, align 4, !dbg !49
  %31 = load ptr, ptr %ptr, align 8, !dbg !50
  %32 = ptrtoint ptr %31 to i64, !dbg !51
  %33 = lshr i64 %32, 3, !dbg !51
  %34 = add i64 %33, 2147450880, !dbg !51
  %35 = inttoptr i64 %34 to ptr, !dbg !51
  %36 = load i8, ptr %35, align 1, !dbg !51
  %37 = icmp ne i8 %36, 0, !dbg !51
  br i1 %37, label %38, label %44, !dbg !51, !prof !52

38:                                               ; preds = %9
  %39 = and i64 %32, 7, !dbg !51
  %40 = add i64 %39, 3, !dbg !51
  %41 = trunc i64 %40 to i8, !dbg !51
  %42 = icmp sge i8 %41, %36, !dbg !51
  br i1 %42, label %43, label %44, !dbg !51

43:                                               ; preds = %38
  call void @__asan_report_store4(i64 %32) #6, !dbg !51
  unreachable

44:                                               ; preds = %38, %9
  store i32 %30, ptr %31, align 4, !dbg !51
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str), !dbg !53
  call void @llvm.lifetime.start.p0(i64 8, ptr %ptr2) #5, !dbg !54
  call void @llvm.dbg.declare(metadata ptr %ptr2, metadata !55, metadata !DIExpression()), !dbg !56
  %45 = load ptr, ptr %ptr, align 8, !dbg !57
  store ptr %45, ptr %ptr2, align 8, !dbg !56
  %46 = load i32, ptr %c, align 4, !dbg !58
  %47 = load ptr, ptr %ptr2, align 8, !dbg !59
  %48 = ptrtoint ptr %47 to i64, !dbg !60
  %49 = lshr i64 %48, 3, !dbg !60
  %50 = add i64 %49, 2147450880, !dbg !60
  %51 = inttoptr i64 %50 to ptr, !dbg !60
  %52 = load i8, ptr %51, align 1, !dbg !60
  %53 = icmp ne i8 %52, 0, !dbg !60
  br i1 %53, label %54, label %60, !dbg !60, !prof !52

54:                                               ; preds = %44
  %55 = and i64 %48, 7, !dbg !60
  %56 = add i64 %55, 3, !dbg !60
  %57 = trunc i64 %56 to i8, !dbg !60
  %58 = icmp sge i8 %57, %52, !dbg !60
  br i1 %58, label %59, label %60, !dbg !60

59:                                               ; preds = %54
  call void @__asan_report_store4(i64 %48) #6, !dbg !60
  unreachable

60:                                               ; preds = %54, %44
  store i32 %46, ptr %47, align 4, !dbg !60
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1), !dbg !61
  call void @llvm.lifetime.end.p0(i64 8, ptr %ptr2) #5, !dbg !62
  call void @llvm.lifetime.end.p0(i64 4, ptr %c) #5, !dbg !62
  call void @llvm.lifetime.end.p0(i64 4, ptr %b) #5, !dbg !62
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #5, !dbg !62
  call void @llvm.lifetime.end.p0(i64 8, ptr %ptr) #5, !dbg !62
  %61 = add i64 %19, 4, !dbg !62
  %62 = inttoptr i64 %61 to ptr, !dbg !62
  store i32 -117901064, ptr %62, align 1, !dbg !62
  %63 = add i64 %19, 8, !dbg !62
  %64 = inttoptr i64 %63 to ptr, !dbg !62
  store i8 -8, ptr %64, align 1, !dbg !62
  call void @llvm.lifetime.end.p0(i64 40, ptr %12) #5, !dbg !62
  store i64 1172321806, ptr %13, align 8, !dbg !63
  %65 = icmp ne i64 %5, 0, !dbg !63
  br i1 %65, label %66, label %75, !dbg !63

66:                                               ; preds = %60
  %67 = add i64 %19, 0, !dbg !63
  %68 = inttoptr i64 %67 to ptr, !dbg !63
  store i64 -723401728380766731, ptr %68, align 1, !dbg !63
  %69 = add i64 %19, 8, !dbg !63
  %70 = inttoptr i64 %69 to ptr, !dbg !63
  store i64 -723401728380766731, ptr %70, align 1, !dbg !63
  %71 = add i64 %5, 120, !dbg !63
  %72 = inttoptr i64 %71 to ptr, !dbg !63
  %73 = load i64, ptr %72, align 8, !dbg !63
  %74 = inttoptr i64 %73 to ptr, !dbg !63
  store i8 0, ptr %74, align 1, !dbg !63
  br label %80, !dbg !63

75:                                               ; preds = %60
  %76 = add i64 %19, 0, !dbg !63
  %77 = inttoptr i64 %76 to ptr, !dbg !63
  store i64 0, ptr %77, align 1, !dbg !63
  %78 = add i64 %19, 8, !dbg !63
  %79 = inttoptr i64 %78 to ptr, !dbg !63
  store i64 0, ptr %79, align 1, !dbg !63
  br label %80, !dbg !63

80:                                               ; preds = %75, %66
  ret i32 0, !dbg !63
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

declare i32 @printf(ptr noundef, ...) #3

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

declare void @__asan_report_load_n(i64, i64)

declare void @__asan_loadN(i64, i64)

declare void @__asan_report_load1(i64)

declare void @__asan_load1(i64)

declare void @__asan_report_load2(i64)

declare void @__asan_load2(i64)

declare void @__asan_report_load4(i64)

declare void @__asan_load4(i64)

declare void @__asan_report_load8(i64)

declare void @__asan_load8(i64)

declare void @__asan_report_load16(i64)

declare void @__asan_load16(i64)

declare void @__asan_report_store_n(i64, i64)

declare void @__asan_storeN(i64, i64)

declare void @__asan_report_store1(i64)

declare void @__asan_store1(i64)

declare void @__asan_report_store2(i64)

declare void @__asan_store2(i64)

declare void @__asan_report_store4(i64)

declare void @__asan_store4(i64)

declare void @__asan_report_store8(i64)

declare void @__asan_store8(i64)

declare void @__asan_report_store16(i64)

declare void @__asan_store16(i64)

declare void @__asan_report_exp_load_n(i64, i64, i32)

declare void @__asan_exp_loadN(i64, i64, i32)

declare void @__asan_report_exp_load1(i64, i32)

declare void @__asan_exp_load1(i64, i32)

declare void @__asan_report_exp_load2(i64, i32)

declare void @__asan_exp_load2(i64, i32)

declare void @__asan_report_exp_load4(i64, i32)

declare void @__asan_exp_load4(i64, i32)

declare void @__asan_report_exp_load8(i64, i32)

declare void @__asan_exp_load8(i64, i32)

declare void @__asan_report_exp_load16(i64, i32)

declare void @__asan_exp_load16(i64, i32)

declare void @__asan_report_exp_store_n(i64, i64, i32)

declare void @__asan_exp_storeN(i64, i64, i32)

declare void @__asan_report_exp_store1(i64, i32)

declare void @__asan_exp_store1(i64, i32)

declare void @__asan_report_exp_store2(i64, i32)

declare void @__asan_exp_store2(i64, i32)

declare void @__asan_report_exp_store4(i64, i32)

declare void @__asan_exp_store4(i64, i32)

declare void @__asan_report_exp_store8(i64, i32)

declare void @__asan_exp_store8(i64, i32)

declare void @__asan_report_exp_store16(i64, i32)

declare void @__asan_exp_store16(i64, i32)

declare ptr @__asan_memmove(ptr, ptr, i64)

declare ptr @__asan_memcpy(ptr, ptr, i64)

declare ptr @__asan_memset(ptr, i32, i64)

declare void @__asan_handle_no_return()

declare void @__sanitizer_ptr_cmp(i64, i64)

declare void @__sanitizer_ptr_sub(i64, i64)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.amdgcn.is.shared(ptr nocapture) #2

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.amdgcn.is.private(ptr nocapture) #2

declare i64 @__asan_stack_malloc_0(i64)

declare void @__asan_stack_free_0(i64, i64)

declare i64 @__asan_stack_malloc_1(i64)

declare void @__asan_stack_free_1(i64, i64)

declare i64 @__asan_stack_malloc_2(i64)

declare void @__asan_stack_free_2(i64, i64)

declare i64 @__asan_stack_malloc_3(i64)

declare void @__asan_stack_free_3(i64, i64)

declare i64 @__asan_stack_malloc_4(i64)

declare void @__asan_stack_free_4(i64, i64)

declare i64 @__asan_stack_malloc_5(i64)

declare void @__asan_stack_free_5(i64, i64)

declare i64 @__asan_stack_malloc_6(i64)

declare void @__asan_stack_free_6(i64, i64)

declare i64 @__asan_stack_malloc_7(i64)

declare void @__asan_stack_free_7(i64, i64)

declare i64 @__asan_stack_malloc_8(i64)

declare void @__asan_stack_free_8(i64, i64)

declare i64 @__asan_stack_malloc_9(i64)

declare void @__asan_stack_free_9(i64, i64)

declare i64 @__asan_stack_malloc_10(i64)

declare void @__asan_stack_free_10(i64, i64)

declare void @__asan_poison_stack_memory(i64, i64)

declare void @__asan_unpoison_stack_memory(i64, i64)

declare void @__asan_set_shadow_00(i64, i64)

declare void @__asan_set_shadow_01(i64, i64)

declare void @__asan_set_shadow_02(i64, i64)

declare void @__asan_set_shadow_03(i64, i64)

declare void @__asan_set_shadow_04(i64, i64)

declare void @__asan_set_shadow_05(i64, i64)

declare void @__asan_set_shadow_06(i64, i64)

declare void @__asan_set_shadow_07(i64, i64)

declare void @__asan_set_shadow_f1(i64, i64)

declare void @__asan_set_shadow_f2(i64, i64)

declare void @__asan_set_shadow_f3(i64, i64)

declare void @__asan_set_shadow_f5(i64, i64)

declare void @__asan_set_shadow_f8(i64, i64)

declare void @__asan_alloca_poison(i64, i64)

declare void @__asan_allocas_unpoison(i64, i64)

declare void @__asan_before_dynamic_init(i64)

declare void @__asan_after_dynamic_init()

declare void @__asan_register_globals(i64, i64)

declare void @__asan_unregister_globals(i64, i64)

declare void @__asan_register_image_globals(i64)

declare void @__asan_unregister_image_globals(i64)

declare void @__asan_register_elf_globals(i64, i64, i64)

declare void @__asan_unregister_elf_globals(i64, i64, i64)

declare void @__asan_init()

; Function Attrs: nounwind uwtable
define internal void @asan.module_ctor() #4 comdat {
  call void @__asan_init()
  call void @__asan_version_mismatch_check_v8()
  call void @__asan_register_elf_globals(i64 ptrtoint (ptr @___asan_globals_registered to i64), i64 ptrtoint (ptr @__start_asan_globals to i64), i64 ptrtoint (ptr @__stop_asan_globals to i64))
  ret void
}

declare void @__asan_version_mismatch_check_v8()

; Function Attrs: nounwind uwtable
define internal void @asan.module_dtor() #4 comdat {
  call void @__asan_unregister_elf_globals(i64 ptrtoint (ptr @___asan_globals_registered to i64), i64 ptrtoint (ptr @__start_asan_globals to i64), i64 ptrtoint (ptr @__stop_asan_globals to i64))
  ret void
}

attributes #0 = { noinline nounwind optnone sanitize_address uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind uwtable "frame-pointer"="all" }
attributes #5 = { nounwind }
attributes #6 = { nomerge }

!llvm.dbg.cu = !{!11}
!llvm.module.flags = !{!13, !14, !15, !16, !17, !18, !19}
!llvm.ident = !{!20}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(scope: null, file: !2, line: 12, type: !3, isLocal: true, isDefinition: true)
!2 = !DIFile(filename: "test/test.c", directory: "/home/induman/Academic/semester7/OELP/Compiler-llvm/outoftree", checksumkind: CSK_MD5, checksum: "107a2270796423dc91aaf972350a70e0")
!3 = !DICompositeType(tag: DW_TAG_array_type, baseType: !4, size: 192, elements: !5)
!4 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!5 = !{!6}
!6 = !DISubrange(count: 24)
!7 = !DIGlobalVariableExpression(var: !8, expr: !DIExpression())
!8 = distinct !DIGlobalVariable(scope: null, file: !2, line: 16, type: !3, isLocal: true, isDefinition: true)
!9 = !{ptr @.str}
!10 = !{ptr @.str.1}
!11 = distinct !DICompileUnit(language: DW_LANG_C11, file: !2, producer: "Ubuntu clang version 18.1.3 (1ubuntu1)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !12, splitDebugInlining: false, nameTableKind: None)
!12 = !{!0, !7}
!13 = !{i32 7, !"Dwarf Version", i32 5}
!14 = !{i32 2, !"Debug Info Version", i32 3}
!15 = !{i32 1, !"wchar_size", i32 4}
!16 = !{i32 8, !"PIC Level", i32 2}
!17 = !{i32 7, !"PIE Level", i32 2}
!18 = !{i32 7, !"uwtable", i32 2}
!19 = !{i32 7, !"frame-pointer", i32 2}
!20 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!21 = distinct !DISubprogram(name: "main", scope: !2, file: !2, line: 3, type: !22, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !11, retainedNodes: !25)
!22 = !DISubroutineType(types: !23)
!23 = !{!24}
!24 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!25 = !{}
!26 = !DILocation(line: 4, column: 5, scope: !21)
!27 = !DILocalVariable(name: "arr", scope: !21, file: !2, line: 4, type: !28)
!28 = !DICompositeType(tag: DW_TAG_array_type, baseType: !24, size: 320, elements: !29)
!29 = !{!30}
!30 = !DISubrange(count: 10)
!31 = !DILocation(line: 4, column: 9, scope: !21)
!32 = !DILocation(line: 5, column: 5, scope: !21)
!33 = !DILocalVariable(name: "ptr", scope: !21, file: !2, line: 5, type: !34)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !24, size: 64)
!35 = !DILocation(line: 5, column: 11, scope: !21)
!36 = !DILocation(line: 5, column: 18, scope: !21)
!37 = !DILocation(line: 7, column: 5, scope: !21)
!38 = !DILocalVariable(name: "a", scope: !21, file: !2, line: 7, type: !24)
!39 = !DILocation(line: 7, column: 9, scope: !21)
!40 = !DILocation(line: 8, column: 5, scope: !21)
!41 = !DILocalVariable(name: "b", scope: !21, file: !2, line: 8, type: !24)
!42 = !DILocation(line: 8, column: 9, scope: !21)
!43 = !DILocation(line: 9, column: 5, scope: !21)
!44 = !DILocalVariable(name: "c", scope: !21, file: !2, line: 9, type: !24)
!45 = !DILocation(line: 9, column: 9, scope: !21)
!46 = !DILocation(line: 9, column: 13, scope: !21)
!47 = !DILocation(line: 9, column: 15, scope: !21)
!48 = !DILocation(line: 9, column: 14, scope: !21)
!49 = !DILocation(line: 11, column: 12, scope: !21)
!50 = !DILocation(line: 11, column: 6, scope: !21)
!51 = !DILocation(line: 11, column: 10, scope: !21)
!52 = !{!"branch_weights", i32 1, i32 100000}
!53 = !DILocation(line: 12, column: 5, scope: !21)
!54 = !DILocation(line: 14, column: 5, scope: !21)
!55 = !DILocalVariable(name: "ptr2", scope: !21, file: !2, line: 14, type: !34)
!56 = !DILocation(line: 14, column: 10, scope: !21)
!57 = !DILocation(line: 14, column: 17, scope: !21)
!58 = !DILocation(line: 15, column: 13, scope: !21)
!59 = !DILocation(line: 15, column: 6, scope: !21)
!60 = !DILocation(line: 15, column: 11, scope: !21)
!61 = !DILocation(line: 16, column: 5, scope: !21)
!62 = !DILocation(line: 18, column: 1, scope: !21)
!63 = !DILocation(line: 17, column: 5, scope: !21)
