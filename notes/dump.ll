define i32 @factorial(i32 %n) {
entry:
  %n1 = alloca i32
  store i32 %n, i32* %n1
  %n2 = load i32, i32* %n1
  %__cmptemp = icmp sle i32 %n2, 1
  br i1 %__cmptemp, label %__iftrue, label %__ifcontr

__iftrue:                                         ; preds = %entry
  ret i32 1
  br label %__ifcontr

__ifcontr:                                        ; preds = %__iftrue, %entry
  %n3 = load i32, i32* %n1
  %n4 = load i32, i32* %n1
  %__addtmp = sub i32 %n4, 1
  %__calltmp = call i32 @factorial(i32 %__addtmp)
  %__multmp = mul i32 %n3, %__calltmp
  ret i32 %__multmp
}

define i32 @ftoi(float %f) {
entry:
  %i = alloca i32
  %f1 = alloca float
  store float %f, float* %f1
  %f2 = load float, float* %f1
  %__cast_tmp = fptosi float %f2 to i32
  store i32 %__cast_tmp, i32* %i
  %i3 = load i32, i32* %i
  ret i32 %i3
}

define float @itof(i32 %i) {
entry:
  %f = alloca float
  %i1 = alloca i32
  store i32 %i, i32* %i1
  %i2 = load i32, i32* %i1
  %__cast_tmp = sitofp i32 %i2 to float
  store float %__cast_tmp, float* %f
  %f3 = load float, float* %f
  ret float %f3
}

define i32 @main() {
entry:
  %__calltmp = call i32 @factorial(i32 4)
  call void @put(i32 %__calltmp)
  ret i32 0
}

define void @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret void
}

declare i32 @putchar(i32)