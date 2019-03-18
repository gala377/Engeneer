declare i32 @putchar(i32)

define i32 @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret i32 %__calltmp
}

define i32 @main() {
entry:
  %c = alloca double
  %b = alloca double
  store double 0.000000e+00, double* %b
  store double 1.000000e+00, double* %c
  %b1 = load double, double* %b
  %c2 = load double, double* %c
  %__addtmp = fsub double %b1, %c2
  %c3 = load double, double* %c
  %__cmptemp = fcmp ult double %__addtmp, %c3
  br i1 %__cmptemp, label %__iftrue, label %__iffalse

__iftrue:                                         ; preds = %entry
  %__calltmp = call i32 @put(i32 0)
  br label %__ifcontr

__iffalse:                                        ; preds = %entry
  %__calltmp4 = call i32 @put(i32 1)
  br label %__ifcontr

__ifcontr:                                        ; preds = %__iffalse, %__iftrue
  ret i32 0
}