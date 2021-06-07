; ModuleID = 'C/demo.0.0.preopt.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = private unnamed_addr constant [7 x i8] c"{main}\00"
@1 = private unnamed_addr constant [20 x i8] c"{Getpasswd,Val1:U=}\00"
@2 = private unnamed_addr constant [15 x i8] c"{%lX:P=Val1:U}\00"
@3 = private unnamed_addr constant [16 x i8] c"{Val2:U=Val3:U}\00"
@4 = private unnamed_addr constant [16 x i8] c"{Val4:U=Val5:U}\00"
@5 = private unnamed_addr constant [23 x i8] c"{Val6:U=Val4:U,Val7:U}\00"
@6 = private unnamed_addr constant [15 x i8] c"{%lX:P=Val6:U}\00"
@7 = private unnamed_addr constant [20 x i8] c"{Trace,%lX:P=%lX:P}\00"
@8 = private unnamed_addr constant [15 x i8] c"{Val8:U=%lX:P}\00"
@9 = private unnamed_addr constant [9 x i8] c"{Val8:U}\00"
@10 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@.str = private unnamed_addr constant [6 x i8] c"CASE1\00", align 1
@11 = private unnamed_addr constant [21 x i8] c"{getenv,%lX:P=%lX:G}\00"
@12 = private unnamed_addr constant [22 x i8] c"{strtol,Val1:U=%lX:P}\00"
@13 = private unnamed_addr constant [16 x i8] c"{Val2:U=Val1:U}\00"
@14 = private unnamed_addr constant [9 x i8] c"{Val2:U}\00"
@15 = private unnamed_addr constant [8 x i8] c"{Trace}\00"
@16 = private unnamed_addr constant [15 x i8] c"{Val1:U=%lX:P}\00"
@.str.1 = private unnamed_addr constant [20 x i8] c"trace: pwd -> %u \0D\0A\00", align 1
@17 = private unnamed_addr constant [22 x i8] c"{printf,%lX:G,Val1:U}\00"

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #0 !dbg !16 {
  call void @TRC_init()
  call void (i64, i8*, ...) @TRC_trace(i64 1224979167364251648, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @0, i32 0, i32 0))
  %3 = alloca i32, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !20, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i8** %1, metadata !21, metadata !DIExpression()), !dbg !27
  %4 = bitcast i32* %3 to i8*, !dbg !28
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #5, !dbg !28
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !29
  call void @llvm.dbg.value(metadata i32* %3, metadata !23, metadata !DIExpression()), !dbg !30
  %5 = tail call i32 (...) bitcast (i32 ()* @Getpasswd to i32 (...)*)() #5, !dbg !31
  call void (i64, i8*, ...) @TRC_trace(i64 1513209543532740616, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @1, i32 0, i32 0)), !dbg !29
  call void @llvm.dbg.value(metadata i32 %5, metadata !22, metadata !DIExpression()), !dbg !29
  store i32 %5, i32* %3, align 4, !dbg !32, !tbaa !33
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761418956810, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @2, i32 0, i32 0), i32* %3), !dbg !37
  call void @llvm.dbg.value(metadata i32 0, metadata !25, metadata !DIExpression()), !dbg !37
  %6 = icmp sgt i32 %0, -3, !dbg !38
  br i1 %6, label %7, label %17, !dbg !39

; <label>:7:                                      ; preds = %2
  %8 = add nsw i32 %0, 2
  call void (i64, i8*, ...) @TRC_trace(i64 1801439919701229582, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @3, i32 0, i32 0)), !dbg !39
  br label %9, !dbg !39

; <label>:9:                                      ; preds = %9, %7
  %10 = phi i32 [ %5, %7 ], [ %13, %9 ], !dbg !37
  %11 = phi i32 [ 0, %7 ], [ %14, %9 ]
  call void @llvm.dbg.value(metadata i32 %11, metadata !25, metadata !DIExpression()), !dbg !37
  call void @llvm.dbg.value(metadata i32 %10, metadata !22, metadata !DIExpression()), !dbg !29
  %12 = shl i32 %10, 1, !dbg !40
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761452511252, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @4, i32 0, i32 0)), !dbg !42
  %13 = or i32 %12, %11, !dbg !42
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761452511253, i8* getelementptr inbounds ([23 x i8], [23 x i8]* @5, i32 0, i32 0)), !dbg !29
  call void @llvm.dbg.value(metadata i32 %13, metadata !22, metadata !DIExpression()), !dbg !29
  %14 = add nuw nsw i32 %11, 1, !dbg !43
  call void @llvm.dbg.value(metadata i32 %14, metadata !25, metadata !DIExpression()), !dbg !37
  %15 = icmp slt i32 %11, %8, !dbg !38
  br i1 %15, label %9, label %16, !dbg !39, !llvm.loop !44

; <label>:16:                                     ; preds = %9
  store i32 %13, i32* %3, align 4, !dbg !46, !tbaa !33
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761469288475, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @6, i32 0, i32 0), i32* %3), !dbg !39
  br label %17, !dbg !39

; <label>:17:                                     ; preds = %16, %2
  call void @llvm.dbg.value(metadata i32* %3, metadata !22, metadata !DIExpression(DW_OP_deref)), !dbg !29
  call void @Trace(i32* nonnull %3) #5, !dbg !47
  call void (i64, i8*, ...) @TRC_trace(i64 1513209543599849502, i8* getelementptr inbounds ([20 x i8], [20 x i8]* @7, i32 0, i32 0), i32* %3, i32* %3), !dbg !48
  %18 = load i32, i32* %3, align 4, !dbg !48, !tbaa !33
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761486065695, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @8, i32 0, i32 0), i32* %3), !dbg !29
  call void @llvm.dbg.value(metadata i32 %18, metadata !22, metadata !DIExpression()), !dbg !29
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #5, !dbg !49
  call void (i64, i8*, ...) @TRC_trace(i64 1441151949561921570, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @9, i32 0, i32 0)), !dbg !50
  call void @TRC_exit()
  ret i32 %18, !dbg !50
}

declare void @TRC_init()

declare void @TRC_trace(i64, i8*, ...)

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

declare void @TRC_exit()

; Function Attrs: nounwind uwtable
define dso_local i32 @Getpasswd() local_unnamed_addr #0 !dbg !51 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979236083728384, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @10, i32 0, i32 0)), !dbg !56
  %1 = tail call i8* @getenv(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0)) #5, !dbg !56
  call void (i64, i8*, ...) @TRC_trace(i64 1531224010761699329, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @11, i32 0, i32 0), i8* %1, [6 x i8]* @.str), !dbg !57
  call void @llvm.dbg.value(metadata i8* %1, metadata !55, metadata !DIExpression()), !dbg !57
  call void @llvm.dbg.value(metadata i8* %1, metadata !58, metadata !DIExpression()) #5, !dbg !66
  %2 = tail call i64 @strtol(i8* nocapture nonnull %1, i8** null, i32 10) #5, !dbg !68
  call void (i64, i8*, ...) @TRC_trace(i64 1513209612252217348, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @12, i32 0, i32 0), i8* %1), !dbg !69
  %3 = trunc i64 %2 to i32, !dbg !69
  call void (i64, i8*, ...) @TRC_trace(i64 1297036830138433541, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @13, i32 0, i32 0)), !dbg !70
  call void (i64, i8*, ...) @TRC_trace(i64 1441152018214289414, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @14, i32 0, i32 0)), !dbg !70
  ret i32 %3, !dbg !70
}

; Function Attrs: nounwind readonly
declare dso_local i8* @getenv(i8* nocapture) local_unnamed_addr #3

; Function Attrs: nounwind
declare dso_local i64 @strtol(i8* readonly, i8** nocapture, i32) local_unnamed_addr #4

; Function Attrs: nounwind uwtable
define dso_local void @Trace(i32* nocapture readonly) local_unnamed_addr #0 !dbg !71 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979304803205120, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @15, i32 0, i32 0)), !dbg !76
  call void @llvm.dbg.value(metadata i32* %0, metadata !75, metadata !DIExpression()), !dbg !76
  %2 = load i32, i32* %0, align 4, !dbg !77, !tbaa !33
  call void (i64, i8*, ...) @TRC_trace(i64 1297036898857910274, i8* getelementptr inbounds ([15 x i8], [15 x i8]* @16, i32 0, i32 0), i32* %0), !dbg !78
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.1, i64 0, i64 0), i32 %2), !dbg !78
  call void (i64, i8*, ...) @TRC_trace(i64 1513209680971694083, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @17, i32 0, i32 0), [20 x i8]* @.str.1), !dbg !79
  ret void, !dbg !79
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!0, !3}
!llvm.ident = !{!12, !12}
!llvm.module.flags = !{!13, !14, !15}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "source/main.c", directory: "/home/wen/LDI/Script/35_case_Clang/C")
!2 = !{}
!3 = distinct !DICompileUnit(language: DW_LANG_C99, file: !4, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !5)
!4 = !DIFile(filename: "source/Passwd.c", directory: "/home/wen/LDI/Script/35_case_Clang/C")
!5 = !{!6, !7, !8, !11}
!6 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!12 = !{!"clang version 7.0.0 (tags/RELEASE_700/final)"}
!13 = !{i32 2, !"Dwarf Version", i32 4}
!14 = !{i32 2, !"Debug Info Version", i32 3}
!15 = !{i32 1, !"wchar_size", i32 4}
!16 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 7, type: !17, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{!7, !7, !8}
!19 = !{!20, !21, !22, !23, !25}
!20 = !DILocalVariable(name: "argc", arg: 1, scope: !16, file: !1, line: 7, type: !7)
!21 = !DILocalVariable(name: "argv", arg: 2, scope: !16, file: !1, line: 7, type: !8)
!22 = !DILocalVariable(name: "pwd", scope: !16, file: !1, line: 9, type: !6)
!23 = !DILocalVariable(name: "ptr", scope: !16, file: !1, line: 10, type: !24)
!24 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!25 = !DILocalVariable(name: "num", scope: !16, file: !1, line: 13, type: !7)
!26 = !DILocation(line: 7, column: 14, scope: !16)
!27 = !DILocation(line: 7, column: 28, scope: !16)
!28 = !DILocation(line: 9, column: 5, scope: !16)
!29 = !DILocation(line: 9, column: 14, scope: !16)
!30 = !DILocation(line: 10, column: 15, scope: !16)
!31 = !DILocation(line: 12, column: 11, scope: !16)
!32 = !DILocation(line: 12, column: 9, scope: !16)
!33 = !{!34, !34, i64 0}
!34 = !{!"int", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !DILocation(line: 13, column: 9, scope: !16)
!38 = !DILocation(line: 14, column: 16, scope: !16)
!39 = !DILocation(line: 14, column: 5, scope: !16)
!40 = !DILocation(line: 16, column: 19, scope: !41)
!41 = distinct !DILexicalBlock(scope: !16, file: !1, line: 15, column: 5)
!42 = !DILocation(line: 16, column: 23, scope: !41)
!43 = !DILocation(line: 18, column: 12, scope: !41)
!44 = distinct !{!44, !39, !45}
!45 = !DILocation(line: 19, column: 5, scope: !16)
!46 = !DILocation(line: 16, column: 13, scope: !41)
!47 = !DILocation(line: 21, column: 5, scope: !16)
!48 = !DILocation(line: 23, column: 12, scope: !16)
!49 = !DILocation(line: 24, column: 1, scope: !16)
!50 = !DILocation(line: 23, column: 5, scope: !16)
!51 = distinct !DISubprogram(name: "Getpasswd", scope: !4, file: !4, line: 4, type: !52, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: true, unit: !3, retainedNodes: !54)
!52 = !DISubroutineType(types: !53)
!53 = !{!6}
!54 = !{!55}
!55 = !DILocalVariable(name: "Value", scope: !51, file: !4, line: 6, type: !9)
!56 = !DILocation(line: 6, column: 19, scope: !51)
!57 = !DILocation(line: 6, column: 11, scope: !51)
!58 = !DILocalVariable(name: "__nptr", arg: 1, scope: !59, file: !60, line: 361, type: !63)
!59 = distinct !DISubprogram(name: "atoi", scope: !60, file: !60, line: 361, type: !61, isLocal: false, isDefinition: true, scopeLine: 362, flags: DIFlagPrototyped, isOptimized: true, unit: !3, retainedNodes: !65)
!60 = !DIFile(filename: "/usr/include/stdlib.h", directory: "/home/wen/LDI/Script/35_case_Clang/C")
!61 = !DISubroutineType(types: !62)
!62 = !{!7, !63}
!63 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64)
!64 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !10)
!65 = !{!58}
!66 = !DILocation(line: 361, column: 1, scope: !59, inlinedAt: !67)
!67 = distinct !DILocation(line: 8, column: 22, scope: !51)
!68 = !DILocation(line: 363, column: 16, scope: !59, inlinedAt: !67)
!69 = !DILocation(line: 363, column: 10, scope: !59, inlinedAt: !67)
!70 = !DILocation(line: 8, column: 5, scope: !51)
!71 = distinct !DISubprogram(name: "Trace", scope: !4, file: !4, line: 11, type: !72, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: true, unit: !3, retainedNodes: !74)
!72 = !DISubroutineType(types: !73)
!73 = !{null, !24}
!74 = !{!75}
!75 = !DILocalVariable(name: "ptr", arg: 1, scope: !71, file: !4, line: 11, type: !24)
!76 = !DILocation(line: 11, column: 23, scope: !71)
!77 = !DILocation(line: 13, column: 38, scope: !71)
!78 = !DILocation(line: 13, column: 5, scope: !71)
!79 = !DILocation(line: 14, column: 5, scope: !71)
