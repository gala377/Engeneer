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
  %b = alloca float
  %a = alloca i32
  store i32 0, i32* %a
  %a1 = load i32, i32* %a
  %__calltmp = call float @itof(i32 %a1)
  store float %__calltmp, float* %b
  %b2 = load float, float* %b
  %__calltmp3 = call i32 @ftoi(float %b2)
  call void @put(i32 %__calltmp3)
  ret i32 0
}