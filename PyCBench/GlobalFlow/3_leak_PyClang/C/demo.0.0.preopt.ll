; ModuleID = 'demo.0.0.preopt.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = private unnamed_addr constant [8 x i8] c"{Trace}\00"
@.str = private unnamed_addr constant [13 x i8] c"trac : %s \0D\0A\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"{main}\00"
@2 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@3 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@.str.3 = private unnamed_addr constant [6 x i8] c"CASE1\00", align 1
@4 = private unnamed_addr constant [16 x i8] c"{getenv,%lX:P=}\00"
@g = dso_local local_unnamed_addr global i8* null, align 8, !dbg !0
@5 = private unnamed_addr constant [14 x i8] c"{%lX:G=%lX:P}\00"

; Function Attrs: nounwind uwtable
define dso_local void @Trace() local_unnamed_addr #0 !dbg !14 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979167364251648, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0)), !dbg !17
  %1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([128 x i8], [128 x i8]* bitcast (i8** @g to [128 x i8]*), i64 0, i64 0)), !dbg !17
  ret void, !dbg !18
}

declare void @TRC_trace(i64, i8*, ...)

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #0 !dbg !19 {
  call void @TRC_init()
  call void (i64, i8*, ...) @TRC_trace(i64 1224979236083728384, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @1, i32 0, i32 0)), !dbg !27
  call void @llvm.dbg.value(metadata i32 %0, metadata !25, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i8** %1, metadata !26, metadata !DIExpression()), !dbg !28
  %3 = tail call i8* (...) bitcast (void ()* @Getpasswd to i8* (...)*)() #4, !dbg !29
  call void (i64, i8*, ...) @TRC_trace(i64 1513209612252217347, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @2, i32 0, i32 0)), !dbg !30
  %4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([128 x i8], [128 x i8]* bitcast (i8** @g to [128 x i8]*), i64 0, i64 0)) #4, !dbg !30
  call void @TRC_exit()
  ret i32 0, !dbg !32
}

declare void @TRC_init()

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

declare void @TRC_exit()

; Function Attrs: nounwind uwtable
define dso_local void @Getpasswd() local_unnamed_addr #0 !dbg !33 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979304803205120, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @3, i32 0, i32 0)), !dbg !34
  %1 = tail call i8* @getenv(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i64 0, i64 0)) #4, !dbg !34
  call void (i64, i8*, ...) @TRC_trace(i64 1531224079481176065, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @4, i32 0, i32 0), i8* %1), !dbg !35
  store i8* %1, i8** @g, align 8, !dbg !35, !tbaa !36
  call void (i64, i8*, ...) @TRC_trace(i64 1297036898857910274, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @5, i32 0, i32 0), i8** @g, i8* %1), !dbg !40
  ret void, !dbg !40
}

; Function Attrs: nounwind readonly
declare dso_local i8* @getenv(i8* nocapture) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!8, !2}
!llvm.ident = !{!10, !10}
!llvm.module.flags = !{!11, !12, !13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g", scope: !2, file: !3, line: 5, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "source/Passwd.c", directory: "/home/wen/LDI/Script/22_case_PyClang/C")
!4 = !{}
!5 = !{!0}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!8 = distinct !DICompileUnit(language: DW_LANG_C99, file: !9, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4)
!9 = !DIFile(filename: "source/main.c", directory: "/home/wen/LDI/Script/22_case_PyClang/C")
!10 = !{!"clang version 7.0.0 (tags/RELEASE_700/final)"}
!11 = !{i32 2, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = distinct !DISubprogram(name: "Trace", scope: !9, file: !9, line: 9, type: !15, isLocal: false, isDefinition: true, scopeLine: 10, isOptimized: true, unit: !8, retainedNodes: !4)
!15 = !DISubroutineType(types: !16)
!16 = !{null}
!17 = !DILocation(line: 11, column: 5, scope: !14)
!18 = !DILocation(line: 12, column: 5, scope: !14)
!19 = distinct !DISubprogram(name: "main", scope: !9, file: !9, line: 15, type: !20, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: true, unit: !8, retainedNodes: !24)
!20 = !DISubroutineType(types: !21)
!21 = !{!22, !22, !23}
!22 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!23 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!24 = !{!25, !26}
!25 = !DILocalVariable(name: "argc", arg: 1, scope: !19, file: !9, line: 15, type: !22)
!26 = !DILocalVariable(name: "argv", arg: 2, scope: !19, file: !9, line: 15, type: !23)
!27 = !DILocation(line: 15, column: 14, scope: !19)
!28 = !DILocation(line: 15, column: 28, scope: !19)
!29 = !DILocation(line: 17, column: 5, scope: !19)
!30 = !DILocation(line: 11, column: 5, scope: !14, inlinedAt: !31)
!31 = distinct !DILocation(line: 19, column: 5, scope: !19)
!32 = !DILocation(line: 21, column: 5, scope: !19)
!33 = distinct !DISubprogram(name: "Getpasswd", scope: !3, file: !3, line: 7, type: !15, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: true, unit: !2, retainedNodes: !4)
!34 = !DILocation(line: 9, column: 6, scope: !33)
!35 = !DILocation(line: 9, column: 4, scope: !33)
!36 = !{!37, !37, i64 0}
!37 = !{!"any pointer", !38, i64 0}
!38 = !{!"omnipotent char", !39, i64 0}
!39 = !{!"Simple C/C++ TBAA"}
!40 = !DILocation(line: 11, column: 2, scope: !33)
