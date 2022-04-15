; ModuleID = 'top'
source_filename = "top"

@0 = private unnamed_addr constant [4 x i8] c"%lg\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"%lg\0A\00", align 1
@2 = private unnamed_addr constant [5 x i8] c"%lg\0A\00", align 1

declare double @sin(double)

declare double @cos(double)

declare double @tan(double)

declare double @acos(double)

declare double @asin(double)

declare double @atan(double)

declare double @sinh(double)

declare double @cosh(double)

declare double @tanh(double)

declare double @exp(double)

declare double @ln(double)

declare double @pow(double, double)

declare i32 @scanf(i8*, ...)

declare i32 @printf(i8*, ...)

define double @factorial(double %0) {
entry:
  %x0 = alloca double, align 8
  store double %0, double* %x0, align 8
  %x01 = load double, double* %x0, align 8
  %tmpres_ule = fcmp ule double %x01, 1.000000e+00
  br i1 %tmpres_ule, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  ret double 1.000000e+00
  br label %if_else

if_else:                                          ; preds = %if_then, %entry
  %x02 = load double, double* %x0, align 8
  %x03 = load double, double* %x0, align 8
  %subtmp = fsub double %x03, 1.000000e+00
  %tmp = call double @factorial(double %subtmp)
  %multmp = fmul double %x02, %tmp
  ret double %multmp
}

define i32 @main() {
entry:
  %x3 = alloca double, align 8
  %x2 = alloca double, align 8
  %x1 = alloca double, align 8
  %x0 = alloca double, align 8
  store double 0.000000e+00, double* %x0, align 8
  store double 0.000000e+00, double* %x1, align 8
  store double 0.000000e+00, double* %x2, align 8
  store double 0.000000e+00, double* %x3, align 8
  %0 = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), double* %x0)
  %x01 = load double, double* %x0, align 8
  %tmp = call double @factorial(double %x01)
  store double %tmp, double* %x0, align 8
  store double 0.000000e+00, double* %x2, align 8
  store double 0.000000e+00, double* %x3, align 8
  br label %loop_cond

loop_cond:                                        ; preds = %loop_end, %entry
  %x22 = load double, double* %x2, align 8
  %tmpres_ult = fcmp ult double %x22, 1.000000e+01
  br i1 %tmpres_ult, label %loop, label %loop_end13

loop:                                             ; preds = %loop_cond
  store double 0.000000e+00, double* %x3, align 8
  br label %loop_cond3

loop_cond3:                                       ; preds = %loop6, %loop
  %x34 = load double, double* %x3, align 8
  %tmpres_ult5 = fcmp ult double %x34, 1.000000e+01
  br i1 %tmpres_ult5, label %loop6, label %loop_end

loop6:                                            ; preds = %loop_cond3
  %x27 = load double, double* %x2, align 8
  %multmp = fmul double 1.000000e+01, %x27
  %x38 = load double, double* %x3, align 8
  %addtmp = fadd double %multmp, %x38
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @1, i32 0, i32 0), double %addtmp)
  %x39 = load double, double* %x3, align 8
  %addtmp10 = fadd double %x39, 1.000000e+00
  store double %addtmp10, double* %x3, align 8
  br label %loop_cond3

loop_end:                                         ; preds = %loop_cond3
  %x211 = load double, double* %x2, align 8
  %addtmp12 = fadd double %x211, 1.000000e+00
  store double %addtmp12, double* %x2, align 8
  br label %loop_cond

loop_end13:                                       ; preds = %loop_cond
  %x014 = load double, double* %x0, align 8
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @2, i32 0, i32 0), double %x014)
  ret i32 0
}
