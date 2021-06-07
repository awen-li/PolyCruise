; ModuleID = 'demo.0.0.preopt.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { [128 x i8], i32 }

@0 = private unnamed_addr constant [8 x i8] c"{Trace}\00"
@1 = private unnamed_addr constant [14 x i8] c"{%lX:P=%lX:P}\00"
@.str = private unnamed_addr constant [13 x i8] c"trac : %s \0D\0A\00", align 1
@2 = private unnamed_addr constant [21 x i8] c"{printf,%lX:G,%lX:P}\00"
@3 = private unnamed_addr constant [7 x i8] c"{main}\00"
@4 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@5 = private unnamed_addr constant [14 x i8] c"{%lX:P=%lX:G}\00"
@6 = private unnamed_addr constant [27 x i8] c"{strcpy,%lX:P=%lX:P,%lX:P}\00"
@7 = private unnamed_addr constant [21 x i8] c"{printf,%lX:G,%lX:P}\00"
@8 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@.str.3 = private unnamed_addr constant [6 x i8] c"CASE1\00", align 1
@9 = private unnamed_addr constant [21 x i8] c"{getenv,%lX:P=%lX:G}\00"
@g = dso_local local_unnamed_addr global i8* null, align 8, !dbg !0
@10 = private unnamed_addr constant [14 x i8] c"{%lX:G=%lX:P}\00"

; Function Attrs: nounwind uwtable
define dso_local void @Trace(%struct.S*) local_unnamed_addr #0 !dbg !14 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979304803205120, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0)), !dbg !29
  call void @llvm.dbg.value(metadata %struct.S* %0, metadata !28, metadata !DIExpression()), !dbg !29
  %2 = getelementptr inbounds %struct.S, %struct.S* %0, i64 0, i32 0, i64 0, !dbg !30
  call void (i64, i8*, ...) @TRC_trace(i64 1657324869047549954, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @1, i32 0, i32 0), i8* %2, %struct.S* %0), !dbg !31
  %3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* %2), !dbg !31
  call void (i64, i8*, ...) @TRC_trace(i64 1513209680971694083, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @2, i32 0, i32 0), [13 x i8]* @.str, %struct.S* %0), !dbg !32
  ret void, !dbg !32
}

declare void @TRC_trace(i64, i8*, ...)

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #0 !dbg !33 {
  call void @TRC_init()
  call void (i64, i8*, ...) @TRC_trace(i64 1224979167364251648, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @3, i32 0, i32 0))
  %3 = alloca %struct.S, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !38, metadata !DIExpression()), !dbg !41
  call void @llvm.dbg.value(metadata i8** %1, metadata !39, metadata !DIExpression()), !dbg !42
  %4 = tail call i8* (...) bitcast (void ()* @Getpasswd to i8* (...)*)() #5, !dbg !43
  call void (i64, i8*, ...) @TRC_trace(i64 1513209543532740612, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @4, i32 0, i32 0)), !dbg !44
  %5 = getelementptr inbounds %struct.S, %struct.S* %3, i64 0, i32 0, i64 0, !dbg !44
  call void @llvm.lifetime.start.p0i8(i64 132, i8* nonnull %5) #5, !dbg !44
  %6 = load i8*, i8** @g, align 8, !dbg !45, !tbaa !46
  call void (i64, i8*, ...) @TRC_trace(i64 1297036761418956807, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @5, i32 0, i32 0), i8* %6, i8** @g), !dbg !50
  %7 = call i8* @strcpy(i8* nonnull %5, i8* %6) #5, !dbg !50
  call void (i64, i8*, ...) @TRC_trace(i64 1513209543532740616, i8* getelementptr inbounds ([27 x i8], [27 x i8]* @6, i32 0, i32 0), %struct.S* %3, %struct.S* %3, i8* %6), !dbg !51
  call void @llvm.dbg.value(metadata %struct.S* %3, metadata !40, metadata !DIExpression(DW_OP_deref)), !dbg !51
  call void @llvm.dbg.value(metadata %struct.S* %3, metadata !28, metadata !DIExpression()) #5, !dbg !52
  %8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* nonnull %5) #5, !dbg !54
  call void (i64, i8*, ...) @TRC_trace(i64 1513209543532740619, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @7, i32 0, i32 0), [13 x i8]* @.str, %struct.S* %3), !dbg !55
  call void @llvm.lifetime.end.p0i8(i64 132, i8* nonnull %5) #5, !dbg !55
  call void @TRC_exit()
  ret i32 0, !dbg !56
}

declare void @TRC_init()

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #3

; Function Attrs: nounwind
declare dso_local i8* @strcpy(i8*, i8* nocapture readonly) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #3

declare void @TRC_exit()

; Function Attrs: nounwind uwtable
define dso_local void @Getpasswd() local_unnamed_addr #0 !dbg !57 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979236083728384, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @8, i32 0, i32 0)), !dbg !60
  %1 = tail call i8* @getenv(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i64 0, i64 0)) #5, !dbg !60
  call void (i64, i8*, ...) @TRC_trace(i64 1531224010761699329, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @9, i32 0, i32 0), i8* %1, [6 x i8]* @.str.3), !dbg !61
  store i8* %1, i8** @g, align 8, !dbg !61, !tbaa !46
  call void (i64, i8*, ...) @TRC_trace(i64 1297036830138433538, i8* getelementptr inbounds ([14 x i8], [14 x i8]* @10, i32 0, i32 0), i8** @g, i8* %1), !dbg !62
  ret void, !dbg !62
}

; Function Attrs: nounwind readonly
declare dso_local i8* @getenv(i8* nocapture) local_unnamed_addr #4

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }
attributes #4 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!8, !2}
!llvm.ident = !{!10, !10}
!llvm.module.flags = !{!11, !12, !13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g", scope: !2, file: !3, line: 5, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "source/Passwd.c", directory: "/home/wen/LDI/Script/23_case_PyClang/C")
!4 = !{}
!5 = !{!0}
!6 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!7 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!8 = distinct !DICompileUnit(language: DW_LANG_C99, file: !9, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4)
!9 = !DIFile(filename: "source/main.c", directory: "/home/wen/LDI/Script/23_case_PyClang/C")
!10 = !{!"clang version 7.0.0 (tags/RELEASE_700/final)"}
!11 = !{i32 2, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = distinct !DISubprogram(name: "Trace", scope: !9, file: !9, line: 15, type: !15, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: true, unit: !8, retainedNodes: !27)
!15 = !DISubroutineType(types: !16)
!16 = !{null, !17}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIDerivedType(tag: DW_TAG_typedef, name: "S", file: !9, line: 13, baseType: !19)
!19 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S", file: !9, line: 9, size: 1056, elements: !20)
!20 = !{!21, !25}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "ctx", scope: !19, file: !9, line: 11, baseType: !22, size: 1024)
!22 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 1024, elements: !23)
!23 = !{!24}
!24 = !DISubrange(count: 128)
!25 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !19, file: !9, line: 12, baseType: !26, size: 32, offset: 1024)
!26 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!27 = !{!28}
!28 = !DILocalVariable(name: "st", arg: 1, scope: !14, file: !9, line: 15, type: !17)
!29 = !DILocation(line: 15, column: 16, scope: !14)
!30 = !DILocation(line: 17, column: 31, scope: !14)
!31 = !DILocation(line: 17, column: 5, scope: !14)
!32 = !DILocation(line: 18, column: 5, scope: !14)
!33 = distinct !DISubprogram(name: "main", scope: !9, file: !9, line: 21, type: !34, isLocal: false, isDefinition: true, scopeLine: 22, flags: DIFlagPrototyped, isOptimized: true, unit: !8, retainedNodes: !37)
!34 = !DISubroutineType(types: !35)
!35 = !{!26, !26, !36}
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!37 = !{!38, !39, !40}
!38 = !DILocalVariable(name: "argc", arg: 1, scope: !33, file: !9, line: 21, type: !26)
!39 = !DILocalVariable(name: "argv", arg: 2, scope: !33, file: !9, line: 21, type: !36)
!40 = !DILocalVariable(name: "st", scope: !33, file: !9, line: 25, type: !18)
!41 = !DILocation(line: 21, column: 14, scope: !33)
!42 = !DILocation(line: 21, column: 28, scope: !33)
!43 = !DILocation(line: 23, column: 5, scope: !33)
!44 = !DILocation(line: 25, column: 5, scope: !33)
!45 = !DILocation(line: 27, column: 21, scope: !33)
!46 = !{!47, !47, i64 0}
!47 = !{!"any pointer", !48, i64 0}
!48 = !{!"omnipotent char", !49, i64 0}
!49 = !{!"Simple C/C++ TBAA"}
!50 = !DILocation(line: 27, column: 5, scope: !33)
!51 = !DILocation(line: 25, column: 7, scope: !33)
!52 = !DILocation(line: 15, column: 16, scope: !14, inlinedAt: !53)
!53 = distinct !DILocation(line: 29, column: 5, scope: !33)
!54 = !DILocation(line: 17, column: 5, scope: !14, inlinedAt: !53)
!55 = !DILocation(line: 32, column: 1, scope: !33)
!56 = !DILocation(line: 31, column: 5, scope: !33)
!57 = distinct !DISubprogram(name: "Getpasswd", scope: !3, file: !3, line: 7, type: !58, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: true, unit: !2, retainedNodes: !4)
!58 = !DISubroutineType(types: !59)
!59 = !{null}
!60 = !DILocation(line: 9, column: 6, scope: !57)
!61 = !DILocation(line: 9, column: 4, scope: !57)
!62 = !DILocation(line: 11, column: 2, scope: !57)
