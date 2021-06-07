; ModuleID = 'demo.0.0.preopt.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = private unnamed_addr constant [8 x i8] c"{Trace}\00"
@.str = private unnamed_addr constant [13 x i8] c"trace: %s \0D\0A\00", align 1
@1 = private unnamed_addr constant [7 x i8] c"{main}\00"
@2 = private unnamed_addr constant [3 x i8] c"{}\00"
@3 = private unnamed_addr constant [12 x i8] c"{Getpasswd}\00"
@.str.3 = private unnamed_addr constant [6 x i8] c"CASE1\00", align 1
@4 = private unnamed_addr constant [16 x i8] c"{getenv,%lX:P=}\00"
@.str.1 = private unnamed_addr constant [13 x i8] c"Value = %s\0D\0A\00", align 1
@5 = private unnamed_addr constant [8 x i8] c"{%lX:P}\00"

; Function Attrs: nounwind uwtable
define dso_local void @Trace(i8*) local_unnamed_addr #0 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979167364251648, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @0, i32 0, i32 0))
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* %0)
  ret void
}

declare void @TRC_trace(i64, i8*, ...)

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32, i8** nocapture readnone) local_unnamed_addr #0 {
  call void @TRC_init()
  call void (i64, i8*, ...) @TRC_trace(i64 1224979236083728384, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @1, i32 0, i32 0))
  %3 = tail call i8* (...) bitcast (i8* ()* @Getpasswd to i8* (...)*)() #3
  %4 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i64 0, i64 0), i8* %3) #3
  call void (i64, i8*, ...) @TRC_trace(i64 1441152018214289411, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @2, i32 0, i32 0))
  call void @TRC_exit()
  ret i32 0
}

declare void @TRC_init()

declare void @TRC_exit()

; Function Attrs: nounwind uwtable
define dso_local i8* @Getpasswd() local_unnamed_addr #0 {
  call void (i64, i8*, ...) @TRC_trace(i64 1224979304803205120, i8* getelementptr inbounds ([12 x i8], [12 x i8]* @3, i32 0, i32 0))
  %1 = tail call i8* @getenv(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i64 0, i64 0)) #3
  call void (i64, i8*, ...) @TRC_trace(i64 1531224079481176065, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @4, i32 0, i32 0), i8* %1)
  %2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.1, i64 0, i64 0), i8* %1)
  call void (i64, i8*, ...) @TRC_trace(i64 1441152086933766147, i8* getelementptr inbounds ([8 x i8], [8 x i8]* @5, i32 0, i32 0), i8* %1)
  ret i8* %1
}

; Function Attrs: nounwind readonly
declare dso_local i8* @getenv(i8* nocapture) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.ident = !{!0, !0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 7.0.0 (tags/RELEASE_700/final)"}
!1 = !{i32 1, !"wchar_size", i32 4}
