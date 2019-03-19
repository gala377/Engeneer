declare i32 @putchar(i32)

define void @put(i32 %v) {
entry:
  %v1 = alloca i32
  store i32 %v, i32* %v1
  %v2 = load i32, i32* %v1
  %__addtmp = add i32 65, %v2
  %__calltmp = call i32 @putchar(i32 %__addtmp)
  ret void
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

define i32 @main() {
entry:
  %c = alloca float
  %b = alloca float
  %a = alloca i32
  store i32 2, i32* %a
  store float 1.500000e+00, float* %b
  %b1 = load float, float* %b
  %a2 = load i32, i32* %a
  %__cast_tmp = sitofp i32 %a2 to float
  %__addtmp = fadd float %b1, %__cast_tmp
  store float %__addtmp, float* %c
  %a3 = load i32, i32* %a
  %c4 = load float, float* %c
  %__cast_tmp5 = sitofp i32 %a3 to float
  %__cmptemp = fcmp ult float %__cast_tmp5, %c4
  br i1 %__cmptemp, label %__iftrue, label %__ifcontr

__iftrue:                                         ; preds = %entry
  call void @put(i32 0)
  br label %__ifcontr

__ifcontr:                                        ; preds = %__iftrue, %entry
  %b6 = load float, float* %b
  %__calltmp = call i32 @ftoi(float %b6)
  call void @put(i32 %__calltmp)
  ret i32 0
}